#ifndef epoch_table_h
#define epoch_table_h

#include <array>
#include <vector>
#include <atomic>
#include "common.h"
#include "interval_list.h"

namespace mvcc {
    struct epoch_table_node{
        uint64_t epoch_num;
        epoch_node* first_node;
        std::atomic<epoch_node*> last_node;

        explicit epoch_table_node(uint64_t epoch_num)
                : epoch_num(epoch_num), first_node(nullptr), last_node(nullptr) {
        }
    };

    class epoch_table{

    private:
        std::array<epoch_table_node*,EPOCH_TABLE_SIZE> epoch_table;
        std::vector<epoch_table_node*> long_live_epochs;
    };

} // namespace mvcc

#endif /* epoch_table_h */