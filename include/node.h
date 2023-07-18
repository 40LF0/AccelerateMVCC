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

	struct undo_entry_node {
		uint64_t trx_id;
		uint64_t space_id;
		uint64_t page_id;
		uint64_t offset;
		std::atomic<undo_entry_node*> next_entry;

		undo_entry_node(uint64_t trx_id, uint64_t space_id, uint64_t page_id, uint64_t offset) 
			:trx_id(trx_id), space_id(space_id), page_id(page_id), offset(offset), next_entry(nullptr) {}
	};

	struct epoch_node{
		uint64_t epoch_num;
		uint64_t min_trx_id;
		uint64_t max_trx_id;
		uint64_t count;
		undo_entry_node* next_entry;
		std::atomic<epoch_node*> next;

		epoch_node(uint64_t epoch_num, uint64_t trx_id, undo_entry_node* next_entry, epoch_node* next)
			: epoch_num(epoch_num), min_trx_id(trx_id), max_trx_id(trx_id), count(1), next_entry(next_entry), next(next)  {}
	};

	struct header_node {
		uint64_t next_epoch_num;
		std::atomic<epoch_node*> next;
		header_node() 
			: next_epoch_num(0), next(nullptr) {}
	};



} // namespace acmvcc