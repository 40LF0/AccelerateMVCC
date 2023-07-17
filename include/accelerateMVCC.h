// Licensed under the MIT license.
#pragma once

#include <cstdint>
#include <iostream>
#include "epochList.h"
#include "trxManager.h"
#include "kuku/kuku.h"

namespace acmvcc
{
    /**
    The AccelerateMvcc class represents a entire in-memory structure for accelerating MVCC. 
    It includes HashTable, HashResult, EpochList, and TrxManager.
    */
    class AccelerateMvcc;

    /**
    The TrxManager class represents mimc version of transaction manager in DBMS
    It should mange transaction id and classify which epoch is active or not.
    */
    class TrxManager;

    /**
    The AccelerateMvcc class represents a entire in-memory structure for accelerating MVCC.
    It includes HashTable, HashResult, EpochList, and TrxManager.
    */
    class AccelerateMvcc {

    public:
        AccelerateMvcc(uint64_t record_count);

    private:
        /**
        The kukuTable represents a cockoo hash table. It includes information about the location functions (hash
        functions) and holds the items inserted into the table.
        */
        kuku::KukuTable* kukuTable;

        /**
        The trxManager represents mimc version of transaction manager in DBMS
        It should mange transaction id and classify which epoch is active or not.
        */
        TrxManager* trxManger;

    };


} // namespace acmvcc