// Licensed under the MIT license.
#pragma once

#include <cstdint>
#include "trxManager.h"

namespace acmvcc
{
    /**
    The AccelerateMvcc class represents a entire in-memory structure for accelerating MVCC. 
    It includes HashTable, HashResult, EpochList, and TrxManager.
    */
    class AccelerateMvcc;

    /**
    The KukuTable class represents a cockoo hash table. It includes information about the location functions (hash
    functions) and holds the items inserted into the table.
    */
    class KukuTable;

    /**
    The HashResult class represents the result of a hash table query. It includes information about whether a queried
    item was found in the hash table, its location in the hash table or stash (if found), and the index of the location
    function (hash function) that was used to insert it. HashResult objects are returned by the KukuTable::query
    function.
    */
    class HashResult;

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



} // namespace acmvcc