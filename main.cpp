// main.cpp : Defines the entry point for the application.

#include "include/accelerateMvcc.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <kuku/kuku.h>

using namespace std;
using namespace kuku;


void print_table(const KukuTable &table) {
    table_size_type col_count = 8;
    for (table_size_type i = 0; i < table.table_size(); i++) {
        const auto &item = table.table(i);
        cout << setw(5)
             << i << ": " << setw(5) << get_high_word(item) << "," << get_low_word(item)
             << ((i % col_count == col_count - 1) ? "\n" : "\t");
    }

    cout << endl << endl << "Stash: " << endl;
    for (table_size_type i = 0; i < table.stash().size(); i++) {
        const auto &item = table.stash(i);
        cout << i << ": " << get_high_word(item) << "," << get_low_word(item) << endl;
    }
    cout << endl;
}

int main() {
    auto table_size = static_cast<table_size_type>(5);
    auto stash_size = static_cast<table_size_type>(5);
    uint8_t loc_func_count = static_cast<uint8_t>(1);
    item_type loc_func_seed = make_random_item();
    uint64_t max_probe = static_cast<uint64_t>(1);
    item_type empty_item = make_item(0, 0);

    KukuTable table(table_size, stash_size, loc_func_count, loc_func_seed, max_probe, empty_item);

    item_type item1 = make_item(100, 101);
    set_value(50, item1);


    if (table.insert(item1)) {
        std::cout << "Item inserted successfully." << std::endl;
    } else {
        std::cout << "Failed to insert item." << std::endl;
    }


    QueryResult result = table.query(make_item(100, 101));

    QueryResult res = result;
    cout << "Found: " << boolalpha << res << endl;
    if (res) {
        cout << "Location: " << res.location() << endl;
        cout << "In stash: " << boolalpha << res.in_stash() << endl;
        cout << "Hash function index: " << res.loc_func_index() << endl << endl;
    }
    if (result) {
        std::cout << "Item found at location: " << result.location() << std::endl;
    } else {
        std::cout << "Item not found." << std::endl;
    }
    std::cout << get_value(table.table(result.location())) << std::endl;


    cout << "Hello CMake." << endl;


    mvcc::Accelerate_mvcc mvcc(10);

    clock_t start, finish;
    double duration;
    
    start = clock();
    for(uint64_t i = 0 ; i < 1000000 ; i ++){
        mvcc.insert(1,1,i,i,i,i);
    }
    finish = clock();
    duration = (double)(finish - start);
    cout << duration << "ms" << endl;


    return 0;
}