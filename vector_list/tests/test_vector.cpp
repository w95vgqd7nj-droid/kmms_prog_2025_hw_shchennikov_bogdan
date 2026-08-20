#include <gtest/gtest.h>
#include "vector.hpp"

using biv::Vector;

class VectorTest : public testing::Test {
protected:
    Vector<int> vec;
};

TEST_F(VectorTest, EmptyVector) {
    EXPECT_EQ(vec.get_size(), 0);
    EXPECT_FALSE(vec.has_item(42));
}

TEST_F(VectorTest, InsertAndPushBack) {
    vec.push_back(10);
    vec.push_back(20);
    EXPECT_EQ(vec.get_size(), 2);
    
    EXPECT_TRUE(vec.insert(1, 15));
    EXPECT_EQ(vec.get_size(), 3);
    EXPECT_TRUE(vec.has_item(15));
    
    EXPECT_FALSE(vec.insert(100, 5));
}

TEST_F(VectorTest, RemoveFirst) {
    vec.push_back(100);
    vec.push_back(200);
    vec.push_back(100);
    
    EXPECT_TRUE(vec.remove_first(100));
    EXPECT_EQ(vec.get_size(), 2);
    
    EXPECT_FALSE(vec.remove_first(999));
}

TEST_F(VectorTest, CapacityReallocation) {
    for (int i = 0; i < 20; ++i) {
        vec.push_back(i);
    }
    EXPECT_EQ(vec.get_size(), 20);
    EXPECT_TRUE(vec.has_item(19));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}