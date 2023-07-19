#include "trxManager.h"

// Later, we will implement transaction manager from VDriver.

namespace acmvcc
{

    uint64_t Trx_manager::startTrx()
    {
        return next_trx_id.fetch_add(1);
    }


    bool Trx_manager::commitTrx(uint64_t trxId)
    {
        return false;
    }

    bool Trx_manager::rollBackTrx(uint64_t trxId)
    {
        return false;
    }



} // namespace acmvcc







