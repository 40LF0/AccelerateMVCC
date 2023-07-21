#ifndef epoch_table_h
#define epoch_table_h

#include <array>
#include <vector>
#include <atomic>
#include "common.h"
#include "interval_list.h"

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
        std::atomic<int> count;

        explicit epoch_table_node(uint64_t epoch_num)
                : epoch_num(epoch_num), count(0) {
            // dummy node!
            auto *epoch_wrapper = new epoch_node_wrapper(nullptr);
            first_node.store(epoch_wrapper);
            last_node.store(epoch_wrapper);
        }
    };

    class epoch_table {
    public:
        epoch_table() {
            for (int i = 0; i < EPOCH_TABLE_SIZE; i++) {
                table.at(i) = new epoch_table_node(i);
                auto* wrapper_dummy = new epoch_node_wrapper(nullptr);
                first_dummy_node.store(wrapper_dummy);
                last_dummy_node.store(wrapper_dummy);
            }
        }

        bool insert(epoch_node *epoch) {
            uint64_t epoch_num = epoch->epoch_num;
            uint64_t index = epoch_num / EPOCH_TABLE_SIZE;
            epoch_table_node *table_node = table.at(index);
            table_node->count.fetch_add(1);
            if (epoch_num < table_node->epoch_num) { // NOLINT(bugprone-branch-clone)
                // something goes wrong..
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

        bool garbage_collect(uint64_t epoch_num) {

        }


    private:
        std::array<epoch_table_node *, EPOCH_TABLE_SIZE> table{};
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