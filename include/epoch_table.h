#ifndef epoch_table_h
#define epoch_table_h

#include <array>
#include <utility>
#include <vector>
#include <atomic>
#include <cstring>
#include <algorithm>
#include "common.h"
#include "interval_list.h"
#include "trxManager.h"

#define NUM_DEADZONE (50)

namespace mvcc {

    struct epoch_node_wrapper {
        epoch_node *epoch;
        std::atomic<epoch_node_wrapper *> next;

        explicit epoch_node_wrapper(epoch_node *epoch)
                : epoch(epoch) {}
    };

    struct epoch_table_node {
        uint64_t epoch_num;
        std::atomic<epoch_node_wrapper *> first_node;
        std::atomic<epoch_node_wrapper *> last_node;

        // watcher number for this node
        std::atomic<int> count;

        explicit epoch_table_node(uint64_t epoch_num)
                : epoch_num(epoch_num), count(0) {
            // dummy node!
            auto *epoch_wrapper = new epoch_node_wrapper(nullptr);
            first_node.store(epoch_wrapper);
            last_node.store(epoch_wrapper);
        }
    };

    class Epoch_table {
    public:
        explicit Epoch_table() {
            for (int i = 0; i < EPOCH_TABLE_SIZE; i++) {
                table.at(i).store(new epoch_table_node(i));
                auto* wrapper_dummy = new epoch_node_wrapper(nullptr);
                first_dummy_node.store(wrapper_dummy);
                last_dummy_node.store(wrapper_dummy);
            }
        }

        bool insert(epoch_node *epoch) {
            uint64_t epoch_num = epoch->epoch_num;
            uint64_t index = epoch_num % EPOCH_TABLE_SIZE;
            epoch_table_node *table_node = table.at(index).load();
            table_node->count.fetch_add(1);
            if (epoch_num < table_node->epoch_num) { // NOLINT(bugprone-branch-clone)
                // GC is already performed
                table_node->count.fetch_sub(1);
                insert_to_dummy(epoch);
                return false;
            } else if (epoch_num > table_node->epoch_num) {
                // GC is too late …
                table_node->count.fetch_sub(1);
                insert_to_dummy(epoch);
                return false;
            } else {
                auto *epoch_wrapper = new epoch_node_wrapper(epoch);

                while (true) {
                    epoch_node_wrapper *last = table_node->last_node.load();
                    epoch_node_wrapper *expected_last = last;
                    epoch_wrapper->next.store(expected_last);
                    if (table_node->last_node.compare_exchange_weak(last, epoch_wrapper)) {
                        table_node->count.fetch_sub(1);
                        return true;
                    }
                    // The compare_exchange_weak failed, retry the operation.
                }
            };
        }

        struct deadzone {
            deadzone(std::vector<trx_t> oldest_active_trx_list, uint64_t oldest_low_limit_id) : len(0) {
                memset(range, 0x00, sizeof(uint64_t) * NUM_DEADZONE * 2);
                this->oldest_active_trx_list = oldest_active_trx_list;
            }

            uint64_t range[2 * NUM_DEADZONE]{};
            uint64_t  oldest_low_limit_id;
            uint64_t len;
            std::vector<trx_t> oldest_active_trx_list;

        };


        uint64_t get_dead_up_limit_id(uint64_t limit_id, const std::vector<trx_t> &vector, const uint64_t trx_id) const {

            for (uint64_t i = 0; i < vector.size(); ++i) {
                if (vector.at(i).trx_id > limit_id) {
                    return (vector.at(i).trx_id);
                }
            }

            return trx_id;
        }

        /**
        update dead zone */
        deadzone* generate_dead_zone(const std::vector<trx_t>& vector) {

            uint64_t oldest_low_limit_id;

            if(vector.at(0).active_trx_list.empty()){
                oldest_low_limit_id = vector.at(0).trx_id;
            }else{
                oldest_low_limit_id = vector.at(0).active_trx_list.at(0).trx_id;
            }
            /* 1. Get free deadzone structure */
            auto* zone = new deadzone(vector.at(0).active_trx_list,oldest_low_limit_id);

            /* 2. Update "zone 1" */
            zone->range[0] = 0;
            zone->range[1] = oldest_low_limit_id;
            zone->len = 1;

            uint64_t low_limit_id;
            uint64_t dead_up_limit_id;
            /* 3. Update other deadzone */
            for (uint64_t i = 1; i < vector.size(); ++i) {
                low_limit_id = vector.at(i - 1).trx_id;
                dead_up_limit_id = get_dead_up_limit_id(low_limit_id, vector.at(i).active_trx_list,
                                                        vector.at(i).trx_id);

                zone->range[2 * zone->len] = low_limit_id;
                zone->range[2 * zone->len + 1] = dead_up_limit_id;
                zone->len++;
            }
            return zone;
        }

        bool can_pruning(uint64_t v_start, uint64_t v_end , deadzone* zone) {
            bool ret = false;
            bool flag = true;


            /* compare its v_start & v_end to deadzone */
            for (uint64_t i = 0; i < zone->len; ++i) {
                if (i == 0 && zone->oldest_active_trx_list.size() != 0) {
                    if (v_end < zone->range[1]) {
                        ret = true;
                        break;
                    }
                    else if (v_end >= zone->oldest_low_limit_id) {
                        continue;
                    }

                    for (int j = 0; j < zone->oldest_active_trx_list.size(); ++j) {
                        if (zone->oldest_active_trx_list.at(j).trx_id == v_end) {
                            flag = false;
                            break;
                        }
                    }
                    if (flag) {
                        ret = true;
                        break;
                    }
                }
                else {
                    if (zone->range[2 * i] < v_start &&
                        v_end < zone->range[2 * i + 1]) {

                        ret = true;
                        break;
                    }
                }
            }

            return (ret);
        }

        bool can_operate_gc(epoch_node_wrapper *epoch_wrapper, deadzone *deadzone) {
            uint64_t epoch_num = epoch_wrapper->epoch->epoch_num;
            uint64_t v_start  = epoch_num * EPOCH_SIZE;
            uint64_t v_end = ((epoch_num + 1) * EPOCH_SIZE) - 1;
            if(can_pruning(v_start,v_end,deadzone)){
                return true;
            }
            return false;
        }

// TODO : when this function is called ?? - every 25(EPOCH_TABLE_SIZE/4) times! 50, 75, 100 …
        //  -> do not start at 25!
        // we need to separate 2 phase of gc processing
        // Phase1 : send epoch_tables nodes to LLT vector
        // <- (epoch_num - epoch_table_size/4)  ~ (epoch_num)
        // Phase2 : processing gc in LLT vector
        bool garbage_collect(uint64_t epoch_num, std::vector<trx_t> vector) {
            if (epoch_num == EPOCH_TABLE_SIZE / 4) {
                return false;
            }
            {
                // get index range to move elements from table to long_live_epochs
                uint64_t start_index = (epoch_num - EPOCH_TABLE_SIZE / 2) % EPOCH_TABLE_SIZE;
                uint64_t end_index = ((epoch_num - EPOCH_TABLE_SIZE / 4) - 1) % EPOCH_TABLE_SIZE;

                int i_start = static_cast<int>(start_index);
                int i_end = static_cast<int>(end_index);
                for (int i = i_start; i <= i_end; i++) {
                    epoch_table_node *prev_table_node = table.at(i).load();
                    long_live_epochs.push_back(prev_table_node);
                    auto *new_table_node = new epoch_table_node((prev_table_node->epoch_num) + EPOCH_TABLE_SIZE);
                    table.at(i).store(new_table_node);
                }
            }
            if (epoch_num == EPOCH_TABLE_SIZE / 2) {
                return false;
            }
            deadzone* deadzone = generate_dead_zone(vector);
            {
                //get size of long_live_epochs and operate gc from (size - EPOCH_TABLE_SIZE / 2) to (size - EPOCH_TABLE_SIZE / 4 - 1)
                uint64_t llt_size = long_live_epochs.size();
                uint64_t start_index = llt_size - (EPOCH_TABLE_SIZE / 2);
                uint64_t end_index = llt_size - (EPOCH_TABLE_SIZE / 4) - 1;
                std::vector<epoch_table_node *> deleteIndexes;

                int i_start = static_cast<int>(start_index);
                int i_end = static_cast<int>(end_index);
                for(int i = i_start;  i <= i_end; i++){
                    epoch_table_node * table_node = long_live_epochs.at(i);

                    epoch_node_wrapper *last_node = table_node->last_node.load();
                    epoch_node_wrapper *prev_node = table_node->first_node.load();
                    if(table_node->count.load() != 0){
                        // some thread trying to insert
                        // you can gc other nodes BUT you should keep last node!
                        // By leave one node for this table node, we can preserve consistency of inserting

                        for(epoch_node_wrapper* node = table_node->first_node.load()->next.load(); node != last_node || node != nullptr;){
                            if(can_operate_gc(node,deadzone)){
                                    prev_node->next = node->next.load();

                                    epoch_node *epochNode = node->epoch;
                                    epochNode->prev.load()->next.store(epochNode->next.load());
                                    delete epochNode;

                                    epoch_node_wrapper* node1 = node;
                                    node = node->next.load();
                                    delete node1;
                            }
                            else{
                                node = node->next.load();
                            }
                        }
                    }
                    else{
                        // you can erase whole node and event table node itself!!

                        for(epoch_node_wrapper* node = table_node->first_node.load()->next.load(); node != nullptr ; node = node->next.load()){
                            if(can_operate_gc(node,deadzone)){
                                    prev_node->next = node->next.load();

                                    epoch_node *epochNode = node->epoch;
                                    epochNode->prev.load()->next.store(epochNode->next.load());
                                    delete epochNode;

                                    epoch_node_wrapper* node1 = node;
                                    node = node->next.load();
                                    delete node1;
                            }
                            else{
                                node = node->next.load();
                            }
                        }

                        if(table_node->first_node.load() == last_node){
                            delete table_node->first_node.load();
                            deleteIndexes.push_back(table_node);
                        }
                    }
                }

                for(epoch_table_node* node : deleteIndexes){
                    auto it = std::find(long_live_epochs.begin(), long_live_epochs.end(), node);
                    if (it != long_live_epochs.end()) {
                        long_live_epochs.erase(it);
                    }
                    delete node;
                }
            }

            delete deadzone;
            //if logic is completed, then change false to true
            return false;
        }


    private:
        std::array<std::atomic<epoch_table_node *>, EPOCH_TABLE_SIZE> table{};
        std::vector<epoch_table_node *> long_live_epochs;
        std::atomic<epoch_node_wrapper *> first_dummy_node;
        std::atomic<epoch_node_wrapper *> last_dummy_node;

        void insert_to_dummy(epoch_node* epoch){
            auto *epoch_wrapper = new epoch_node_wrapper(epoch);
            while (true) {
                epoch_node_wrapper *last = last_dummy_node.load();
                epoch_node_wrapper *expected_last = last;
                epoch_wrapper->next.store(expected_last);
                if (last_dummy_node.compare_exchange_weak(last, epoch_wrapper)) {
                    return;
                }
                // The compare_exchange_weak failed, retry the operation.
            }
        }
    };

} // namespace mvcc

#endif /* epoch_table_h */