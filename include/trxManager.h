// Licensed under the MIT license.
#pragma once

#include <cstdint>
#include <atomic>

namespace acmvcc
{
    /**
    The TrxManager class represents mimc version of transaction manager in DBMS
    It should mange transaction id and classify which epoch is active or not.
    */

    class TrxManager
    {
    public:
        TrxManager(uint64_t record_count);

        uint64_t startTrx();

        bool commitTrx(uint64_t trxId);

        bool rollBackTrx(uint64_t trxId);

    private:
        std::atomic<uint64_t> AtomicNextTrxId = 0;
        // The record_count represent the number of records saved in the hash table
        uint64_t record_count;
    };


} // namespace acmvcc