// Licensed under the MIT license.
#pragma once

#include <cstdint>
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
    The EpochList class represents the item of a hash table.
    It includes information about undo log entries information for corresponding transaction id.
    */
    class EpochList;

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
        AccelerateMvcc();

    private:
        /**
        The kukuTable represents a cockoo hash table. It includes information about the location functions (hash
        functions) and holds the items inserted into the table.
        */
        kuku::KukuTable kukuTable;

        /**
        The queryresult represents the result of a hash table query. It includes information about whether a queried
        item was found in the hash table, its location in the hash table or stash (if found), and the index of the location
        function (hash function) that was used to insert it. HashResult objects are returned by the KukuTable::query
        function.
        */
        kuku::QueryResult queryresult;

        /**
        The trxManager represents mimc version of transaction manager in DBMS
        It should mange transaction id and classify which epoch is active or not.
        */
        TrxManager trxManger;

    };


} // namespace acmvcc