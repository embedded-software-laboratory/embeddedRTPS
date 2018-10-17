#include <gtest/gtest.h>
#include <TestA.h>

TEST(AccountTest, AccountStartsEmpty){
    TestA a;
    EXPECT_EQ(a.balance, 0);
}