#include "trxManager.h"

// Later, we will implement transaction manager from VDriver.

namespace acmvcc
{
    TrxManager::TrxManager(uint64_t epochSize)
    {
        this->epochSize = epochSize;
    }

    uint64_t TrxManager::startTrx()
    {
        return AtomicNextTrxId++;
    }


    bool TrxManager::commitTrx(uint64_t trxId)
    {
        return true;
    }

    bool TrxManager::rollBackTrx(uint64_t trxId)
    {
        return true;
    }


} // namespace acmvcc







