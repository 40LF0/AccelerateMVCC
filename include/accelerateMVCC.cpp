#include "accelerateMVCC.h"

acmvcc::AccelerateMvcc::AccelerateMvcc(uint64_t record_count)
{
	// if you are willing to test large number of elements, you have to change table size : (1 << 10) + 1 to (1 << 16)
	kukuTable = new kuku::KukuTable((1 << 10) + 1, 0, 2, kuku::make_zero_item(), 100, kuku::make_random_item());
	trxManger = new TrxManager(record_count); 
	for (int i = 0; i < record_count; i++) {
		kuku::item_type item = kuku::make_item(1,i);
		// value is header node for epoch-based interval linked list
		std::uint64_t value = 987654321;
		kuku::set_value(value, item);
		if (!kukuTable->insert(item)) {
			std::cout << "record number : " << i << "is not inserted" << std::endl;
		}
	}
}
