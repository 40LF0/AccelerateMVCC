// Licensed under the MIT license.
#pragma once

#ifndef accelerateMVCC_h
#define accelerateMVCC_h

#include <cstdint>
#include <iostream>
#include "trxManager.h"
#include "kuku/kuku.h"
#include "interval_list.h"
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

        void insert_trx(uint64_t index){
            // start transaction
            auto *trx = start_write_trx();
            uint64_t trx_id = trx->trx_id;

            // get write lock of the record
            get_mutex(index);

            // insert undo log entry to interval list
            insert(1,index,trx_id,trx_id,trx_id,trx_id);

            // commit transaction
            commit_trx(trx);

            // release write lock
            release_mutex(index);
        }

        void dummy_read_trx() {
            auto* trx = start_trx();
            uint64_t trx_id = trx->trx_id;
            commit_trx(trx);
        }

        // insert undo log entry to interval list
        bool insert(uint64_t table_id, uint64_t index, uint64_t trx_id, uint64_t space_id, uint64_t page_id, uint64_t offset);

        bool search(uint64_t table_id, uint64_t index,
            uint64_t trx_id, uint64_t& space_id, uint64_t& page_id, uint64_t& offset,
            std::vector<uint64_t> active_trx_list
            );

        static uint64_t get_epoch_num(uint64_t trx_id) {
            return trx_id / EPOCH_SIZE;
        }

        trx_t* start_trx(){
            return trxManger->startTrx();
        }

        trx_t* start_write_trx(){
            return trxManger->startWriteTrx();
        }

        void commit_trx(trx_t* trx){
            trxManger->commitTrx(trx);
        }

        void get_mutex(uint64_t index){
            trxManger->get_mutex(index);
        }

        void release_mutex(uint64_t index){
            trxManger->release_mutex(index);
        }

        void operate_gc(){
            uint64_t trx_id = trxManger->get_next_trx_id() - 1;
            epoch_table->garbage_collect(get_epoch_num(trx_id));
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

        Epoch_table* epoch_table;

    };


} // namespace mvcc
#endif /* accelerateMVCC_h */