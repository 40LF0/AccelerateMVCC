#include "epochList.h"
#include "common.h"
#include <iostream>

#define EPSILON 0.2
#define setMark(address) ((EpochNode *)((uintptr_t)address | 2))
#define setFlag(address) ((EpochNode *)((uintptr_t)address | 1))
#define getMark(address) ((int)((uintptr_t)address & 2) == 2 ? 1 : 0)
#define getFlag(address) ((int)((uintptr_t)address & 1))
#define getNodeAddress(address) ((EpochNode *)((uintptr_t)address & -4))

namespace acmvcc
{
	/**When insert epochList pointer to hashMap, we have to do CAS operation.
	* If CAS succeed(return : null value), new epochList is successfuly inserted to hashmap.
	* If CAS fail(return : not null value), epochList is already inserted.
	*  - Free useless epochList and try to insert undoLogEntry to existing epochList.
	*/
	EpochList::EpochList(uint64_t firstTrxId)
	{
		this->epochSize = epochSize;
		// EpochNode* epochNode = new EpochNode(firstTrxId / EPOCH_SIZE);
		// next = epochNode;
		insertNewEpoch(firstTrxId / EPOCH_SIZE, next);
	}

	EpochNode* constructArgs(EpochNode *node, int mark, int flag)
	{
		EpochNode *temp = getNodeAddress(node);
		if (mark == 1)
			(temp) = setMark(temp);
		if (flag == 1)
			(temp) = setFlag(temp);
		return temp;
	}

	void helpMarked(EpochNode *prev, EpochNode *del)
	{
		EpochNode *next = getNodeAddress(del)->next;

		EpochNode *delNode = constructArgs(del, 0, 1);
		EpochNode *nextNode = constructArgs(next, 0, 0);
		if (sizeof(int *) == 4)
			__sync_val_compare_and_swap((unsigned int *)&(getNodeAddress(prev)->next), (unsigned int)delNode, (unsigned int)nextNode);
		else
			__sync_val_compare_and_swap((unsigned long long *)&(getNodeAddress(prev)->next),
										(unsigned long long)delNode, (unsigned long long)nextNode);

		free(getNodeAddress(del)); // to be modified -> pass to BG purger
	}

	return_sf searchFrom(int epoch, EpochNode *curr)
	{
		return_sf s;
		EpochNode *next = curr->next;
		EpochNode *prev;
		while (getNodeAddress(next)->epochNumber >= epoch)
		{
			while (getMark(next) == 1 && (getMark(curr) == 0 || getNodeAddress(curr)->next != getNodeAddress(next)))
			{
				if (getNodeAddress(curr)->next == getNodeAddress(next))
				{
					helpMarked(curr, next);
				}
				next = getNodeAddress(curr)->next;
			}
			if (getNodeAddress(next)->epochNumber >= epoch)
			{
				prev = curr;
				curr = next;
				next = getNodeAddress(curr)->next;
			}
		}
		if (getNodeAddress(curr)->epochNumber == epoch)
		{
			s.current = curr;
			s.next = next;
			return s;
		}
		else
		{
			insertNewEpoch(epoch, prev);
			return searchFrom(epoch, prev);
		}
	}

	void TryMark(EpochNode *del)
	{
		EpochNode *next;
		EpochNode *result;
		do
		{
			next = getNodeAddress(del)->next;

			EpochNode *nextNode = constructArgs(next, 0, 0);
			EpochNode *next_node1 = constructArgs(next, 1, 0);
			if (sizeof(int *) == 4)
				result = (EpochNode *)__sync_val_compare_and_swap((unsigned int *)&(getNodeAddress(del)->next),
																(unsigned int)(nextNode), (unsigned int)(next_node1));
			else
			{
				result = (EpochNode *)__sync_val_compare_and_swap((unsigned long long *)&(getNodeAddress(del)->next),
																(unsigned long long)(nextNode), (unsigned long long)(next_node1));
			}

			if (getMark(result) == 0 && getFlag(result) == 1)
				helpFlagged(del, result);
		} while (getMark(getNodeAddress(del)->next) != 1);
	}

	void helpFlagged(EpochNode *prev, EpochNode *del)
	{
		getNodeAddress(del)->backlink = prev;
		if (getMark(getNodeAddress(del)->next) == 0)
			TryMark(del);
		helpMarked(prev, del);
	}

	int insertNewEpoch(int epoch, EpochNode *head)
	{
		return_sf s = searchFrom(epoch, head);
		EpochNode *prev;
		EpochNode *next;
		EpochNode *prev_succ;
		prev = s.current;
		next = s.next;
		if (getNodeAddress(prev)->epochNumber == epoch)
			return -1;
		EpochNode *newNode = (EpochNode *)malloc(sizeof(EpochNode));
		newNode->epochNumber = epoch;
		newNode->undoLogEntry = nullptr;
		newNode->next = nullptr;

		while (1)
		{
			prev_succ = getNodeAddress(prev)->next;
			if (getFlag(prev_succ) == 1)
				helpFlagged(prev, getNodeAddress(prev_succ)->next);
			else
			{
				newNode->next = next;
				EpochNode *nextNode = constructArgs(next, 0, 0);
				EpochNode *new_Node = constructArgs(newNode, 0, 0);
				EpochNode *result;
				if (sizeof(int *) == 4)
					result = (EpochNode *)__sync_val_compare_and_swap((unsigned int *)&(getNodeAddress(prev)->next),
																	(unsigned int)(nextNode), (unsigned int)(new_Node));
				else
					result = (EpochNode *)__sync_val_compare_and_swap((unsigned long long *)&(getNodeAddress(prev)->next),
																	(unsigned long long)(nextNode), (unsigned long long)(new_Node));

				if (result == next)
				{
					return 0;
				}
				else
				{
					if (getFlag(result) == 1)
						helpFlagged(prev, (result)->next);
					while (getMark(prev) == 1)
						prev = getNodeAddress(prev)->backlink;
				}
			}
			s = searchFrom(epoch, prev);
			prev = s.current;
			next = s.next;
			if (getNodeAddress(s.current)->epochNumber == epoch)
			{
				free(newNode);
				return -1;
			}
		}
		return 0;
	}

	return_tf TryFlag(EpochNode *prev, EpochNode *target)
	{
		return_tf r;
		while (1)
		{
			if (getNodeAddress(prev)->next == target && getMark(getNodeAddress(prev)->next) == 0 && getFlag(getNodeAddress(prev)->next) == 1) // Already Flagged. Some other process would delete_node it.
			{
				r.node = prev;
				r.result = 0;
				r.del_node = NULL;
				return r;
			}
			EpochNode *target_node = constructArgs(target, 0, 0);
			EpochNode *target_node_new = constructArgs(target, 0, 1);
			EpochNode *result;
			if (sizeof(int *) == 4)
				result = (EpochNode *)__sync_val_compare_and_swap((unsigned int *)&(getNodeAddress(prev)->next),
																(unsigned int)(target_node), (unsigned int)(target_node_new));
			else
				result = (EpochNode *)__sync_val_compare_and_swap((unsigned int *)&(getNodeAddress(prev)->next),
																(unsigned int)(target_node), (unsigned int)(target_node_new));

			if (result == target && getMark(result) == 0 && getFlag(result) == 0)
			{
				r.node = prev;
				r.result = 1;
				r.del_node = NULL;
				return r;
			}
			if (result == target && getMark(result) == 0 && getFlag(result) == 1)
			{
				r.node = prev;
				r.result = 0;
				r.del_node = NULL;
				return r;
			}
			while (getMark(getNodeAddress(prev)->next) == 1)
				prev = getNodeAddress(prev)->backlink;

			return_sf s = searchFrom((getNodeAddress(target)->epochNumber) - EPSILON, prev);
			// return_sf s = searchFrom((getNodeAddress(target)->epochNumber), prev);
			r.node = s.current;
			r.prev_node = s.current;
			r.del_node = s.next;
			prev = s.current;
			if (s.next != target)
			{
				r.node = NULL;
				r.result = 0;
				return r;
			}
		}
	}

	EpochNode *searchEpoch(int epoch, EpochNode *head)
	{
		return_sf s = searchFrom(epoch, head);
		if (s.current->epochNumber == epoch)
		{
			return s.current;
		}
		else
			0x0; // NULL
	}

	int delete_node(int epoch, EpochNode *head) // returns 0 if delete is successful, -1 on failure.
	{
		return_sf s = searchFrom(epoch - EPSILON, head);
		// return_sf s = searchFrom(epoch, head);
		EpochNode *prev = s.current;
		EpochNode *del = s.next;
		if (getNodeAddress(del)->epochNumber != epoch)
			return -1;
		return_tf tf = TryFlag(prev, del);
		prev = tf.node;
		if (tf.del_node != NULL)
			del = tf.del_node;
		if (prev != NULL)
			helpFlagged(prev, del);
		if (tf.result == 0)
		{
			return -1;
		}
		return 0;
	}

	EpochNode *init_LF_list()
	{
		EpochNode *head = (EpochNode *)malloc(sizeof(EpochNode));
		EpochNode *tail = (EpochNode *)malloc(sizeof(EpochNode));
		head->next = tail;
		head->epochNumber = std::numeric_limits<int>::max();
		tail->next = NULL;
		tail->epochNumber = std::numeric_limits<int>::min();
		return head;
	}

	void printlist(EpochNode *head)
	{
		head = getNodeAddress(head)->next;
		while (getNodeAddress(head)->epochNumber != std::numeric_limits<int>::min())
		{
			printf("%d\t", getNodeAddress(head)->epochNumber);
			head = head->next;
		}
		printf("\n");
	}
} // namespace acmvcc
