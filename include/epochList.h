// Licensed under the MIT license.
#pragma once

#include <cstdint>
#include <atomic>
#include "node.h"

namespace acmvcc
{
    /**
    The EpochList class represents the item of a hash table.
    It includes information about undo log entries information for corresponding transaction id.
    */
    class EpochList {
    public:

        EpochList(uint64_t firstTrxId);

        bool search(uint64_t trxId);

        bool insert(uint64_t trxId, bool information);

    private:
        uint64_t epochSize;
        std::atomic<EpochNode*> nextEpoch;

    };

} // namespace acmvcc