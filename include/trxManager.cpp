#include "trxManager.h"

// Later, we will implement transaction manager from VDriver.

namespace acmvcc
{
    TrxManager::TrxManager(uint64_t record_count)
    {
        this->record_count = record_count;
    }

    uint64_t TrxManager::startTrx()
    {
        return AtomicNextTrxId++;
    }


    bool TrxManager::commitTrx(uint64_t trxId)
    {
        return false;
    }

    bool TrxManager::rollBackTrx(uint64_t trxId)
    {
        return false;
    }


} // namespace acmvcc







