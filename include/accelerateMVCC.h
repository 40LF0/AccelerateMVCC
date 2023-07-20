// Licensed under the MIT license.
#pragma once

#ifndef accelerateMVCC_h
#define accelerateMVCC_h

#include <cstdint>
#include <iostream>
#include "trxManager.h"
#include "kuku/kuku.h"
#include "node.h"
#include "common.h"
#include "epoch_table.h"

namespace mvcc
{
    /**
    The AccelerateMvcc class represents a entire in-memory structure for accelerating MVCC. 
    It includes HashTable, HashResult, EpochList, and TrxManager.
    */
    class Accelerate_mvcc;

    /**
    The TrxManager class represents mimic version of transaction manager in DBMS
    It should mange transaction id and classify which epoch is active or not.
    */
    class Trx_manager;

    /**
    The AccelerateMvcc class represents a entire in-memory structure for accelerating MVCC.
    It includes HashTable, HashResult, EpochList, and TrxManager.
    */
    class Accelerate_mvcc {

    public:
        explicit Accelerate_mvcc(uint64_t record_count);

        bool insert(uint64_t table_id, uint64_t index, uint64_t trx_id, uint64_t space_id, uint64_t page_id, uint64_t offset);

        bool search(uint64_t table_id, uint64_t index,
            uint64_t trx_id, uint64_t& space_id, uint64_t& page_id, uint64_t& offset,
            std::vector<uint64_t> active_trx_list
            );

        static uint64_t get_epoch_num(uint64_t trx_id) {
            return trx_id / EPOCH_SIZE;
        }


    private:
        /**
        The kukuTable represents a cuckoo hash table. It includes information about the location functions (hash
        functions) and holds the items inserted into the table.
        */
        kuku::KukuTable* kuku_table;

        /**
        The trxManager represents mimic version of transaction manager in DBMS
        It should mange transaction id and classify which epoch is active or not.
        */
        Trx_manager* trxManger;

    };


} // namespace mvcc
#endif /* accelerateMVCC_h */