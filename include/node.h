// Licensed under the MIT license.
#pragma once

#include <cstdint>
#include <atomic>


namespace acmvcc{

	struct UndoLogEntryNode {

		uint64_t trxId;

		uint64_t spaceId;
		uint64_t pageId;
		uint64_t offset;

		// std::atomic<UndoLogEntryNode*> nextUndoLogEntry = nullptr;
		UndoLogEntryNode* next = nullptr;

		UndoLogEntryNode(uint64_t trxId, uint64_t spaceId ,uint64_t pageId,uint64_t offset);
	};


	struct EpochNode {

		uint64_t epochNumber;

		// std::atomic<UndoLogEntryNode*> startUndoLogEntry = nullptr;
		// std::atomic<UndoLogEntryNode*> endUndoLogEntry = nullptr;
		// std::atomic<EpochNode*> nextEpoch = nullptr;
		UndoLogEntryNode* undoLogEntry = nullptr;
		EpochNode* next = nullptr;
		EpochNode* backlink = nullptr;

		EpochNode(uint64_t epochNumber);
	};

	UndoLogEntryNode* search(uint64_t trxId, EpochNode *head);
	int insert(uint64_t trxId, uint64_t spaceId, uint64_t pageId, uint64_t offset, EpochNode *head);
} // namespace acmvcc