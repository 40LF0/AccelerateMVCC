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

        // bool search(uint64_t trxId);
        // bool insert(uint64_t trxId, bool information);

    private:
        uint64_t epochSize;
        // std::atomic<EpochNode*> nextEpoch;
        EpochNode* next;
    };

    typedef struct searchfrom
    {
        EpochNode *current;
        EpochNode *next;
    } return_sf;

    typedef struct csArg
    {
        int *node;
    } cs_arg;

    typedef struct return_tryFlag
    {
        EpochNode *node;
        int result;
        EpochNode *prev_node;
        EpochNode *del_node;
    } return_tf;

    void helpFlagged(EpochNode *prev, EpochNode *del);
    EpochNode *constructArgs(EpochNode *node, int mark, int flag);
    void helpMarked(EpochNode *prev, EpochNode *del);
    return_sf searchFrom(int epoch, EpochNode *curr);
    void tryMark(EpochNode *del);
    void helpFlagged(EpochNode *prev, EpochNode *del);
    int insertNewEpoch(int epoch, EpochNode *head);
    EpochNode *searchEpoch(int epoch, EpochNode *head);
    return_tf tryFlag(EpochNode *prev, EpochNode *target);
    int deleteNode(int epoch, EpochNode *head);
    void printList(EpochNode *head);
    void destroy(EpochNode *head);

} // namespace acmvcc