#include <gtest/gtest.h>
#include <kuku/kuku.h>
#include "include/accelerateMVCC.h"

using namespace std;
using namespace kuku;

TEST(CommonTests, SetItem) {
    item_type bl;

    set_item(0, 0, bl);
    ASSERT_EQ(0, get_low_word(bl));
    ASSERT_EQ(0, get_high_word(bl));

    set_item(1, 0, bl);
    ASSERT_EQ(1, get_low_word(bl));
    ASSERT_EQ(0, get_high_word(bl));

    set_item(0, 1, bl);
    ASSERT_EQ(0, get_low_word(bl));
    ASSERT_EQ(1, get_high_word(bl));

    set_item(0xF00F, 0xBABA, bl);
    ASSERT_EQ(0xF00F, get_low_word(bl));
    ASSERT_EQ(0xBABA, get_high_word(bl));

    set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
    ASSERT_EQ(0xF00FF00FF00FF00F, get_low_word(bl));
    ASSERT_EQ(0xBABABABABABABABA, get_high_word(bl));

    unsigned char data[bytes_per_item]{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                       0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    set_item(data, bl);
    ASSERT_EQ(0x0706050403020100, get_low_word(bl));
    ASSERT_EQ(0x0706050403020100, get_high_word(bl));
}

TEST(CommonTests, SetZeroItem) {
    item_type bl;

    set_item(0, 0, bl);
    set_zero_item(bl);
    ASSERT_EQ(0, get_low_word(bl));
    ASSERT_EQ(0, get_high_word(bl));

    set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
    set_zero_item(bl);
    ASSERT_EQ(0, get_low_word(bl));
    ASSERT_EQ(0, get_high_word(bl));
}

TEST(CommonTests, SetAllOnesItem) {
    item_type bl;

    set_zero_item(bl);
    set_all_ones_item(bl);
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, get_low_word(bl));
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, get_high_word(bl));

    set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
    set_all_ones_item(bl);
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, get_low_word(bl));
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, get_high_word(bl));
}

TEST(CommonTests, IsZeroItem) {
    item_type bl;

    set_item(0, 0, bl);
    ASSERT_TRUE(is_zero_item(bl));

    set_item(1, 0, bl);
    ASSERT_FALSE(is_zero_item(bl));

    set_item(0, 1, bl);
    ASSERT_FALSE(is_zero_item(bl));

    set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
    ASSERT_FALSE(is_zero_item(bl));
}

TEST(CommonTests, IsAllOnesItem) {
    item_type bl;

    set_all_ones_item(bl);
    ASSERT_TRUE(is_all_ones_item(bl));

    set_item(0xFFFFFFFFFFFFFFFFULL, 0, bl);
    ASSERT_FALSE(is_all_ones_item(bl));

    set_item(0, 0xFFFFFFFFFFFFFFFFULL, bl);
    ASSERT_FALSE(is_all_ones_item(bl));

    set_item(0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFFULL, bl);
    ASSERT_FALSE(is_all_ones_item(bl));
}

TEST(CommonTests, SetRandomItem) {
    item_type bl;

    set_random_item(bl);
    ASSERT_FALSE(is_zero_item(bl));
    item_type bl2 = bl;
    ASSERT_TRUE(are_equal_item(bl, bl2));
    set_random_item(bl);
    ASSERT_FALSE(is_zero_item(bl));
    ASSERT_FALSE(are_equal_item(bl, bl2));
}

TEST(CommonTests, ZeroItem) {
    item_type bl = make_random_item();
    ASSERT_FALSE(is_zero_item(bl));
    bl = make_zero_item();
    ASSERT_TRUE(is_zero_item(bl));
}

TEST(CommonTests, IncrementItem) {
    item_type bl = make_item(0, 0);

    increment_item(bl);
    ASSERT_EQ(1, get_low_word(bl));
    ASSERT_EQ(0, get_high_word(bl));

    bl = make_item(0xF00F, 0xBAAB);
    increment_item(bl);
    ASSERT_EQ(0xF010, get_low_word(bl));
    ASSERT_EQ(0xBAAB, get_high_word(bl));

    bl = make_item(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE);
    increment_item(bl);
    ASSERT_EQ(0x0, get_low_word(bl));
    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, get_high_word(bl));

    bl = make_item(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    increment_item(bl);
    ASSERT_EQ(0x0, get_low_word(bl));
    ASSERT_EQ(0x0, get_high_word(bl));
}

TEST(KukuTableTests, Create) {
    ASSERT_THROW(KukuTable(0, 0, 2, make_zero_item(), 1, make_zero_item()), invalid_argument);
    ASSERT_THROW(KukuTable(1, 0, 0, make_zero_item(), 1, make_zero_item()), invalid_argument);
    ASSERT_THROW(KukuTable(1, 0, 2, make_zero_item(), 0, make_zero_item()), invalid_argument);
    ASSERT_NO_THROW(KukuTable(min_table_size, 0, 2, make_zero_item(), 1, make_zero_item()));
    ASSERT_NO_THROW(KukuTable(1, 0, min_loc_func_count, make_zero_item(), 1, make_zero_item()));
}

TEST(KukuTableTests, Populate1) {
    {
        KukuTable ct(1, 0, 1, make_zero_item(), 10, make_zero_item());
        ASSERT_TRUE(ct.is_empty(0));
        ASSERT_TRUE(ct.insert(make_item(1, 0)));
        ASSERT_FALSE(ct.insert(make_item(0, 1)));
        ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
        ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);
        ASSERT_FALSE(ct.is_empty(0));
    }
    {
        KukuTable ct(1, 0, 2, make_zero_item(), 10, make_zero_item());
        ASSERT_TRUE(ct.is_empty(0));
        ASSERT_TRUE(ct.insert(make_item(1, 0)));
        ASSERT_FALSE(ct.insert(make_item(0, 1)));
        ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
        ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);
        ASSERT_FALSE(ct.is_empty(0));
    }
    {
        KukuTable ct(2, 0, 1, make_zero_item(), 10, make_zero_item());
        ASSERT_TRUE(ct.is_empty(0));
        ASSERT_TRUE(ct.insert(make_item(1, 0)));

        // No collision
        ASSERT_TRUE(ct.insert(make_item(0, 1)));

        // Collision
        ASSERT_FALSE(ct.insert(make_item(0, 2)));

        ASSERT_FALSE(ct.is_empty(0));
        ASSERT_FALSE(ct.is_empty(1));
    }
}

TEST(KukuTableTests, Populate2) {
    KukuTable ct(1 << 10, 0, 2, make_zero_item(), 10, make_zero_item());
    for (location_type i = 0; i < ct.table_size(); i++) {
        ASSERT_TRUE(ct.is_empty(i));
    }

    ASSERT_TRUE(ct.insert(make_item(1, 0)));
    ASSERT_TRUE(ct.insert(make_item(0, 1)));
    ASSERT_TRUE(ct.insert(make_item(1, 1)));
    ASSERT_TRUE(ct.insert(make_item(2, 2)));
    ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
    ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);

    int non_empties = 0;
    for (location_type i = 0; i < ct.table_size(); i++) {
        non_empties += ct.is_empty(i) ? 0 : 1;
    }
    ASSERT_EQ(non_empties, 4);

    ASSERT_TRUE(ct.query(make_item(1, 0)));
    ASSERT_TRUE(ct.query(make_item(0, 1)));
    ASSERT_TRUE(ct.query(make_item(1, 1)));
    ASSERT_TRUE(ct.query(make_item(2, 2)));
    ASSERT_FALSE(ct.query(make_item(3, 3)));
}

TEST(KukuTableTests, Populate3) {
    KukuTable ct(1 << 10, 0, 2, make_zero_item(), 10, make_random_item());
    for (location_type i = 0; i < ct.table_size(); i++) {
        ASSERT_TRUE(ct.is_empty(i));
    }

    ASSERT_TRUE(ct.insert(make_item(0, 0)));
    ASSERT_TRUE(ct.insert(make_item(1, 0)));
    ASSERT_TRUE(ct.insert(make_item(0, 1)));
    ASSERT_TRUE(ct.insert(make_item(1, 1)));
    ASSERT_TRUE(ct.insert(make_item(2, 2)));

    // Fails
    ASSERT_FALSE(ct.insert(make_item(2, 2)));

    ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);

    int non_empties = 0;
    for (location_type i = 0; i < ct.table_size(); i++) {
        non_empties += ct.is_empty(i) ? 0 : 1;
    }
    ASSERT_EQ(non_empties, 5);

    ASSERT_TRUE(ct.query(make_item(0, 0)));
    ASSERT_TRUE(ct.query(make_item(1, 0)));
    ASSERT_TRUE(ct.query(make_item(0, 1)));
    ASSERT_TRUE(ct.query(make_item(1, 1)));
    ASSERT_TRUE(ct.query(make_item(2, 2)));
    ASSERT_FALSE(ct.query(make_item(3, 3)));
}

TEST(KukuTableTests, Fill1) {
    KukuTable ct(1 << 10, 0, 2, make_zero_item(), 100, make_random_item());
    vector<item_type> inserted_items;
    for (int i = 0; i < 100; i++) {
        inserted_items.emplace_back(make_random_item());
        ASSERT_TRUE(ct.insert(inserted_items.back()));
    }
    for (auto b: inserted_items) {
        ASSERT_TRUE(ct.query(b));
    }
    ASSERT_FALSE(ct.query(make_random_item()));
}

TEST(KukuTableTests, Fill2) {
    KukuTable ct((1 << 10) - 1, 0, 4, make_zero_item(), 100, make_random_item());
    vector<item_type> inserted_items;
    for (int i = 0; i < 600; i++) {
        inserted_items.emplace_back(make_random_item());
        ASSERT_TRUE(ct.insert(inserted_items.back()));
    }
    for (auto b: inserted_items) {
        ASSERT_TRUE(ct.query(b));
    }
    ASSERT_FALSE(ct.query(make_random_item()));
}

TEST(KukuTableTests, Fill3) {
    KukuTable ct((1 << 10) + 1, 4, 2, make_zero_item(), 100, make_random_item());
    vector<item_type> inserted_items;
    for (int i = 0; i < 950; i++) {
        inserted_items.emplace_back(make_random_item());
        if (!ct.insert(inserted_items.back())) {
            auto it = find_if(inserted_items.cbegin(), inserted_items.cend(), [&](const item_type &item) {
                return are_equal_item(ct.leftover_item(), item);
            });
            ASSERT_TRUE(it != inserted_items.cend());
            ASSERT_FALSE(ct.query(ct.leftover_item()));
            inserted_items.erase(it);
        }
    }
    for (auto b: inserted_items) {
        ASSERT_TRUE(ct.query(b));
    }
}

TEST(KukuTableTests, Locations) {
    uint8_t lfc = 2;
    KukuTable ct(1 << 10, 4, lfc, make_random_item(), 100, make_all_ones_item());
    for (int k = 0; k < 20; k++) {
        auto it = make_random_item();
        auto all_locs = ct.all_locations(it);

        bool collision_found = false;
        for (uint8_t i = 0; i < lfc; i++) {
            for (uint8_t j = 0; j < i; j++) {
                collision_found = collision_found || (ct.location(it, i) == ct.location(it, j));
            }
        }

        ASSERT_EQ(all_locs.size() < lfc, collision_found);
    }
}

TEST(KukuTableTests, RepeatedInsert) {
    KukuTable ct(1 << 10, 0, 4, make_zero_item(), 10, make_zero_item());
    ASSERT_TRUE(ct.insert(make_item(1, 0)));
    ASSERT_TRUE(ct.insert(make_item(0, 1)));
    ASSERT_TRUE(ct.insert(make_item(1, 1)));
    ASSERT_TRUE(ct.insert(make_item(2, 2)));

    ASSERT_FALSE(ct.insert(make_item(1, 0)));
    ASSERT_FALSE(ct.insert(make_item(0, 1)));
    ASSERT_FALSE(ct.insert(make_item(1, 1)));
    ASSERT_FALSE(ct.insert(make_item(2, 2)));
}

TEST(LocFuncTests, Create) {
    ASSERT_THROW(LocFunc(0, make_item(0, 0)), invalid_argument);
    ASSERT_THROW(LocFunc(max_table_size + 1, make_item(0, 0)), invalid_argument);
    ASSERT_THROW(LocFunc(0, make_item(1, 1)), invalid_argument);
}

TEST(LocFuncTests, Randomness) {
    for (table_size_type ts = min_table_size; ts < 5 * min_table_size; ts++) {
        for (int attempts = 0; attempts < 10; attempts++) {
            item_type seed = make_random_item();
            LocFunc lf(ts, seed);
            LocFunc lf2(ts, seed);

            uint64_t zeros = 0;
            uint64_t total = 1000;
            for (uint64_t i = 0; i < total; i++) {
                item_type bl;
                set_random_item(bl);
                ASSERT_TRUE(lf(bl) == lf2(bl));
                zeros += (lf(bl) == size_t(0));
            }
            ASSERT_TRUE(
                    abs(static_cast<double>(zeros) / static_cast<double>(total) - 1 / static_cast<double>(ts)) < 0.05);
        }
    }
}

TEST(KukuValueTest, ValueManipulation) {
    item_type item;
    std::uint64_t value = 123456789;

    // Set the value
    set_value(value, item);

    // Verify the value is correctly set
    ASSERT_EQ(value, get_value(item));

    // Modify the value
    std::uint64_t newValue = 987654321;
    set_value(newValue, item);

    // Verify the value is correctly modified
    ASSERT_EQ(newValue, get_value(item));
}

TEST(KukuValueTest, location1) {
    KukuTable ct(1, 0, 1, make_zero_item(), 10, make_zero_item());

    ASSERT_TRUE(ct.is_empty(0));

    item_type item = make_item(1, 0);
    std::uint64_t value = 123456789;
    set_value(value, item);


    ASSERT_TRUE(ct.insert(item));
    ASSERT_FALSE(ct.insert(make_item(0, 1)));
    ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
    ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);
    ASSERT_FALSE(ct.is_empty(0));


    QueryResult result = ct.query(make_item(1, 0));
    ASSERT_EQ(get_value(ct.table(result.location())), value);

}

TEST(KukuValueTest, location2) {
    KukuTable ct(1, 1, 1, make_zero_item(), 10, make_zero_item());

    ASSERT_TRUE(ct.is_empty(0));

    item_type item1 = make_item(1, 0);
    std::uint64_t value1 = 123456789;
    set_value(value1, item1);

    item_type item2 = make_item(0, 1);
    std::uint64_t value2 = 987654321;
    set_value(value2, item2);

    // inserted in hashTable
    ASSERT_TRUE(ct.insert(item1));
    // inserted in stash space
    ASSERT_TRUE(ct.insert(item2));
    ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
    ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);
    ASSERT_FALSE(ct.is_empty(0));


    QueryResult result1 = ct.query(make_item(1, 0));
    ASSERT_EQ(result1.found(), true);
    ASSERT_EQ(result1.in_stash(), false);
    ASSERT_EQ(get_value(ct.table(result1.location())), value1);

    QueryResult result2 = ct.query(make_item(0, 1));


    ASSERT_EQ(result2.found(), true);
    ASSERT_EQ(result2.in_stash(), true);
    ASSERT_EQ(get_value(ct.stash(result2.location())), value2);

}

TEST(AccelerateTest, initalize_accelerate_mvcc) {
    mvcc::Accelerate_mvcc mvcc(10);
    ASSERT_EQ(true, true);
}

TEST(AccelerateTest, create_1M_dummy_read_transaction) {
    mvcc::Accelerate_mvcc mvcc(10);
    for (uint64_t i = 0; i < 1000000; i++) {
        mvcc.dummy_read_trx();
    }
    ASSERT_EQ(true, true);
}

TEST(AccelerateTest, inserting_1M_to_single_node_interval_list) {
    mvcc::Accelerate_mvcc mvcc(10);
    for(uint64_t i = 0 ; i < 1000000 ; i ++){
        mvcc.insert(1,1,i,i,i,i);
    }
    ASSERT_EQ(true, true);
}

TEST(AccelerateTest, inserting_1M_to_single_node_trx_manager) {
    mvcc::Accelerate_mvcc mvcc(10);
    for(uint64_t i = 0 ; i < 1000000 ; i ++){
        uint64_t index = i % 10;
        mvcc.insert_trx(index);
    }
    ASSERT_EQ(true, true);
}
