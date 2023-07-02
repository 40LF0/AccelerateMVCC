#include "node.h"
#include "epochList.h"
#include "common.h"
#include <mutex>

acmvcc::UndoLogEntryNode::UndoLogEntryNode(uint64_t trxId, uint64_t spaceId, uint64_t pageId, uint64_t offset)
{
	this->trxId = trxId;
	this->spaceId = spaceId;
	this->pageId = pageId;
	this->offset = offset;
}

acmvcc::EpochNode::EpochNode(uint64_t epochNumber)
{
	this->epochNumber = epochNumber;
}

namespace acmvcc
{
	UndoLogEntryNode *search(uint64_t trxId, EpochNode *head)
	{
		EpochNode *epochNode = searchEpoch(trxId / EPOCH_SIZE, head);
		UndoLogEntryNode *curr = epochNode->undoLogEntry;
		UndoLogEntryNode *next = curr->next;
		while (next->trxId >= trxId)
		{
			curr = next;
			next = curr->next;
		}
		return curr;
	}

	int insert(uint64_t trxId, uint64_t spaceId, uint64_t pageId, uint64_t offset, EpochNode *head)
	{
		EpochNode *epochNode = searchEpoch(trxId / EPOCH_SIZE, head);
		UndoLogEntryNode *curr = epochNode->undoLogEntry;
		UndoLogEntryNode *next = curr->next;
		UndoLogEntryNode *newNode = (UndoLogEntryNode *)malloc(sizeof(UndoLogEntryNode));
		newNode->trxId = trxId;
		newNode->spaceId = spaceId;
		newNode->pageId = pageId;
		newNode->offset = offset;
		bool result;
		while (1)
		{
			while (next->trxId >= trxId)
			{
				curr = next;
				next = curr->next;
			}

			if (curr->trxId == trxId) return -1;		

			newNode->next = curr->next;
			if (sizeof(int *) == 4)
				result = __sync_val_compare_and_swap((unsigned int *)&(curr->next),
														  (unsigned int)(next), (unsigned int)(newNode));
			else
				result = __sync_val_compare_and_swap((unsigned long long *)&(curr->next),
														  (unsigned long long)(next), (unsigned long long)(newNode));
			if (result) return 0;

			next = curr->next;
		}
	}
}
