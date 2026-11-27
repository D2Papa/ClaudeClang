/**
 * @file    clamp_tdd_test.cpp
 * @brief   Clamp_Int 구현 중 TDD로 작성한 테스트. 단위 테스트(test/unit)와 분리하며 결과·커버리지도 따로 측정한다.
 */
#include <gtest/gtest.h>

#include "clamp/clamp.h"

TEST(ClampTdd, ReturnsMinWhenBelowRange)
{
    int out = 0;
    ASSERT_EQ(STATUS_OK, Clamp_Int(-5, 0, 3, &out));
    EXPECT_EQ(0, out);
}

TEST(ClampTdd, ReturnsMaxWhenAboveRange)
{
    int out = 0;
    ASSERT_EQ(STATUS_OK, Clamp_Int(50, 0, 3, &out));
    EXPECT_EQ(3, out);
}

TEST(ClampTdd, KeepsValueInsideRange)
{
    int out = 0;
    ASSERT_EQ(STATUS_OK, Clamp_Int(2, 0, 3, &out));
    EXPECT_EQ(2, out);
}
