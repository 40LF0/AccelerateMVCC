// Licensed under the MIT license.

#include "accelerateMVCC.h"

acmvcc::Accelerate_mvcc::Accelerate_mvcc(uint64_t record_count)
{
	// if you are willing to test large number of elements, you have to change table size : (1 << 10) + 1 to (1 << 16)
	kuku_table = new kuku::KukuTable((1 << 10), 0, 2, kuku::make_zero_item(), 100, kuku::make_random_item());
	// kukuTable = new kuku::KukuTable((1 << 16), 0, 2, kuku::make_zero_item(), 100, kuku::make_random_item());
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
	undo_entry_node* undo_entry = new undo_entry_node(trx_id, space_id, page_id, offset);
	uint64_t epoch_num = get_epoch_num(trx_id);
	epoch_node* epoch = new epoch_node(epoch_num, trx_id, undo_entry, nullptr);
	header_node* header = new header_node();
	header->next_epoch_num = epoch_num;
	header->next.store(epoch);
	std::uint64_t value = reinterpret_cast<std::uint64_t>(&header);
	kuku::item_type item = kuku::make_item(table_id, index);
	kuku::set_value(value, item);

	return kuku_table->insert(item);
}
