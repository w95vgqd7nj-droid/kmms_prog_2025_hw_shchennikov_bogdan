#include <gtest/gtest.h>
#include "doubly_linked_list.hpp"

using biv::DoublyLinkedList;

class ListTest : public testing::Test {
protected:
    DoublyLinkedList<int> list;
};

TEST_F(ListTest, EmptyListSize) {
    EXPECT_EQ(list.get_size(), 0);
    EXPECT_FALSE(list.has_item(5));
}

TEST_F(ListTest, PushBackAndSize) {
    list.push_back(10);
    list.push_back(20);
    EXPECT_EQ(list.get_size(), 2);
    EXPECT_TRUE(list.has_item(10));
    EXPECT_TRUE(list.has_item(20));
}

TEST_F(ListTest, RemoveFirst) {
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(2);
    
    EXPECT_TRUE(list.remove_first(2));
    EXPECT_EQ(list.get_size(), 3);
    EXPECT_TRUE(list.has_item(2));
    
    EXPECT_FALSE(list.remove_first(99));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}