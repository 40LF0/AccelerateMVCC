// Licensed under the MIT license.

#include "accelerateMVCC.h"

acmvcc::Accelerate_mvcc::Accelerate_mvcc(uint64_t record_count)
{
	// if you are willing to test large number of elements, you have to change table size : (1 << 10) + 1 to (1 << 16)
	kuku_table = new kuku::KukuTable((1 << 10), (1 << 10), 2, kuku::make_zero_item(), 100, kuku::make_random_item());
	// kukuTable = new kuku::KukuTable((1 << 16), (1 << 10), 2, kuku::make_zero_item(), 100, kuku::make_random_item());
	trxManger = new Trx_manager(record_count); 

	for (int i = 0; i < record_count; i++) {
		kuku::item_type item = kuku::make_item(1,i);
		
		// value is header node pointer address for epoch-based interval linked list
		header_node* header = new header_node();

		
		std::uint64_t value = reinterpret_cast<std::uint64_t>(&header);
		
		// we can get header address from value
		// header_node* header = reinterpret_cast<header_node*>(value);
		
		kuku::set_value(value, item);
		if (!kuku_table->insert(item)) {
			std::cout << "record number : " << i << "is not inserted" << std::endl;
		}
	}


}


// this will be used when implementing to mysql source code.
bool acmvcc::Accelerate_mvcc::insert(uint64_t table_id, uint64_t index, 
	uint64_t trx_id, uint64_t space_id, uint64_t page_id, uint64_t offset)
{
	kuku::item_type item = kuku::make_item(table_id, index);

	undo_entry_node* undo_entry = new undo_entry_node(trx_id, space_id, page_id, offset);
	uint64_t epoch_num = get_epoch_num(trx_id);

	kuku::QueryResult query =  kuku_table->query(item);
	if (query.found()) {
		uint64_t value;
		int location = query.location();
		if (query.in_stash()) {
			item = kuku_table->stash(query.location());
			value = kuku::get_value(item);
		}
		else {
			item = kuku_table->table(query.location());
			value = kuku::get_value(item);
		}
		header_node* header = reinterpret_cast<header_node*>(&item);
		if (header->next_epoch_num < epoch_num) {
			// create new epoch and insert it to header
			epoch_node* epoch = new epoch_node(epoch_num, trx_id, undo_entry, header->next.load());
			header->next_epoch_num = epoch_num;
			header->next.store(epoch);
		}
		else {
			// insert undo log entry to existing epoch
			epoch_node* epoch = header->next.load();
			epoch->count++;
			undo_entry_node* next_entry = epoch->next_entry;
			if (trx_id < epoch->min_trx_id) {
				epoch->min_trx_id = trx_id;
			}
			if (trx_id > epoch->max_trx_id) {
				epoch->max_trx_id = trx_id;
			}
			undo_entry->next_entry.store(next_entry);
			epoch->next_entry.store(undo_entry);
		}
	}
	else {
		epoch_node* epoch = new epoch_node(epoch_num, trx_id, undo_entry, nullptr);
		header_node* header = new header_node();
		header->next_epoch_num = epoch_num;
		header->next.store(epoch);
		std::uint64_t value = reinterpret_cast<std::uint64_t>(&header);

		kuku::set_value(value, item);

		return kuku_table->insert(item);
	}

}

bool acmvcc::Accelerate_mvcc::search(uint64_t table_id, uint64_t index,
	uint64_t trx_id, uint64_t& space_id, uint64_t& page_id, uint64_t& offset, 
	std::vector<uint64_t> active_trx_list)
{
	kuku::item_type item = kuku::make_item(table_id, index);
	uint64_t epoch_num = get_epoch_num(trx_id);
	kuku::QueryResult query = kuku_table->query(item);
	if (!query.found()) {
		return false;
	}

	uint64_t value;
	int location = query.location();

	if (query.in_stash()) {
		item = kuku_table->stash(query.location());
		value = kuku::get_value(item);
	}
	else {
		item = kuku_table->table(query.location());
		value = kuku::get_value(item);
	}
	header_node* header = reinterpret_cast<header_node*>(&item);

	epoch_node* epoch = header->next.load();
	while (epoch != NULL && epoch != nullptr) {
		if (epoch->epoch_num > epoch_num) {
			epoch = epoch->next.load();
			continue;
		}
		else {
			undo_entry_node* undo_entry = epoch->next_entry.load();
			while (undo_entry != NULL && undo_entry != nullptr) {
				if (undo_entry->trx_id < trx_id) {
					// undo log's trx_id is not in active transaction list
					if(std::find(active_trx_list.begin(), active_trx_list.end(), undo_entry->trx_id) == active_trx_list.end()) {
						space_id = undo_entry->space_id;
						page_id = undo_entry->page_id;
						offset = undo_entry->offset;
						return true;
					}
				}
				undo_entry = undo_entry->next_entry.load();
			}
		}
	}

	return false;
}
