// main.cpp : Defines the entry point for the application.
//
#include <iostream>
#include "include/accelerateMVCC.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <kuku/kuku.h>

using namespace std;
using namespace kuku;

void trxManagerTest();





int main()
{

	auto table_size = static_cast<table_size_type>(5);
	auto stash_size = static_cast<table_size_type>(5);
	uint8_t loc_func_count = static_cast<uint8_t>(1);
	item_type loc_func_seed = make_random_item();
	uint64_t max_probe = static_cast<uint64_t>(1);
	item_type empty_item = make_item(0, 0);

	KukuTable table(table_size, stash_size, loc_func_count, loc_func_seed, max_probe, empty_item);


	cout << "Hello CMake." << endl;

	trxManagerTest();


	return 0;
}

void trxManagerTest() {
	acmvcc::TrxManager trxManager = acmvcc::TrxManager(1);

	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
}
