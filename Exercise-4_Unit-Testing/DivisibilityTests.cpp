#include "gtest/gtest.h"
#include "Divisibility.h"

//Unit tests go here

// 1. A is positive, B is positive, and A does divide B. The function shuld return true
TEST(DivisibilityTest, PositiveADividesPositiveB){
	bool test_result = isDivisible(3,6);
	ASSERT_TRUE(test_result);
}

// 2. A is positivem B is positive, and A does not divide B. The function should return false.
TEST(DivisibilityTest, PositiveANotDividesPositiveB){
	bool test_result = isDivisible(4,6);
	ASSERT_FALSE(test_result);
}

// 3. A is positive, B is zero. The function should return true.
TEST(DivisibilityTest, PositiveADividesZeroB){
	bool test_result = isDivisible(9,0);
	ASSERT_TRUE(test_result);
}

// 4. A is negative, B is positive, and A does divide B. The function should return true.
TEST(DivisibilityTest, NegativeADividesPositiveB){
	bool test_result = isDivisible(-2,6);
	ASSERT_TRUE(test_result);
}
