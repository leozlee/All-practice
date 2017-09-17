//
// Created by leo on 8/13/17.
//

// A sample program demonstrating using Google C++ testing framework.
//
// Author: wan@google.com (Zhanyong Wan)
// Modify: leozlee@163.com

// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.
//
// Writing a unit test using Google C++ testing framework is easy as 1-2-3:


// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "project1.h"
#include "gtest/gtest.h"


// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.
//
// <TechnicalDetails>
//
// In Google Test, tests are grouped into test cases.  This is how we
// keep test code organized.  You should put logically related tests
// into the same test case.
//
// The test case name and the test name should both be valid C++
// identifiers.  And you should not use underscore (_) in the names.
//
// Google Test guarantees that each test you define is run exactly
// once, but it makes no guarantee on the order the tests are
// executed.  Therefore, you should write your tests in such a way
// that their results don't depend on their order.
//
// </TechnicalDetails>


// Tests Project_file.
// This test is named "Negative", and belongs to the "FactorialTest"
// test case.
TEST(IndependentMethod,T)
{
    int i = 3;
    independentMethod(i);
    EXPECT_EQ(0, i);
}





// Tests factorial of negative numbers.
//TEST(FactorialTest, Negative) {
    // This test is named "Negative", and belongs to the "FactorialTest"
    // test case.

    // <TechnicalDetails>
    //
    // EXPECT_EQ(expected, actual) is the same as
    //
    //   EXPECT_TRUE((expected) == (actual))
    //
    // except that it will print both the expected value and the actual
    // value when the assertion fails.  This is very helpful for
    // debugging.  Therefore in this case EXPECT_EQ is preferred.
    //
    // On the other hand, EXPECT_TRUE accepts any Boolean expression,
    // and is thus more general.
    //
    // </TechnicalDetails>
//}

// Tests factorial of 0.
//TEST(FactorialTest, Zero) {
//    EXPECT_EQ(1, Factorial(0));
//}

// Tests factorial of positive numbers.
//TEST(FactorialTest, Positive) {
//    EXPECT_EQ(1, Factorial(1));
//    EXPECT_EQ(2, Factorial(2));
//    EXPECT_EQ(6, Factorial(3));
//    EXPECT_EQ(40320, Factorial(8));
//}


// Tests IsPrime()
//
// Tests negative input.
//TEST(IsPrimeTest, Negative) {
//    // This test belongs to the IsPrimeTest test case.
//
//    EXPECT_FALSE(IsPrime(-1));
//    EXPECT_FALSE(IsPrime(-2));
//    EXPECT_FALSE(IsPrime(INT_MIN));
//}
//
// Tests some trivial cases.
//TEST(IsPrimeTest, Trivial) {
//    EXPECT_FALSE(IsPrime(0));
//    EXPECT_FALSE(IsPrime(1));
//    EXPECT_TRUE(IsPrime(2));
//    EXPECT_TRUE(IsPrime(3));
//}
//
// Tests positive input.
//TEST(IsPrimeTest, Positive) {
//    EXPECT_FALSE(IsPrime(4));
//    EXPECT_TRUE(IsPrime(5));
//    EXPECT_FALSE(IsPrime(6));
//    EXPECT_TRUE(IsPrime(23));
//}

// Step 3. Call RUN_ALL_TESTS() in main().


