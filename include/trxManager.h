// Licensed under the MIT license.
#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map> // Add this header for the std::unordered_map

namespace acmvcc
{

    /**
    The TrxManager class represents mimc version of transaction manager in DBMS
    It should mange transaction id and classify which epoch is active or not.
    */

    class Trx_manager
    {
    public:
        Trx_manager(uint64_t record_count)
            : next_trx_id(1), record_count(record_count), mutexes(record_count)
        {}

        uint64_t startTrx();

        bool commitTrx(uint64_t trxId);

        bool rollBackTrx(uint64_t trxId);

    private:
        std::atomic<uint64_t> next_trx_id;
        // The record_count represent the number of records saved in the hash table
        uint64_t record_count;

        // Map to store the mutexes with unique identifiers for each table_id and index combination
        std::vector<std::mutex> mutexes;
    };


} // namespace acmvcc