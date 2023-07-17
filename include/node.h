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

		std::atomic<UndoLogEntryNode*> nextUndoLogEntry = nullptr;

		UndoLogEntryNode(uint64_t trxId, uint64_t spaceId ,uint64_t pageId, uint64_t offset);
	};


	struct EpochNode {
		uint64_t epochNumber;

		std::atomic<UndoLogEntryNode*> startUndoLogEntry = nullptr;
		std::atomic<UndoLogEntryNode*> endUndoLogEntry = nullptr;
		std::atomic<EpochNode*> nextEpoch = nullptr;

		EpochNode(uint64_t epochNumber);
	};



} // namespace acmvcc