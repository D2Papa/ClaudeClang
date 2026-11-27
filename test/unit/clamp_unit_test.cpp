/**
 * @file    clamp_unit_test.cpp
 * @brief   Clamp_Int 단위 테스트 (SWE.4). 파이프라인 검증용 샘플이다.
 * @note    MC/DC: (out != NULL) && (min <= max) 의 두 조건이 각각 결과를 단독으로 바꾸는 쌍을 포함한다.
 */
#include <gtest/gtest.h>

#include "clamp/clamp.h"

/**
 * @brief   [목적] 범위 안의 값은 그대로 출력되는지 확인한다. (동치 클래스: 범위 내)
 * @par 입력값  value=5, min=0, max=10, out=유효한 버퍼
 * @par 절차    -# out을 -1로 초기화한다. -# Clamp_Int(5, 0, 10, &out)를 호출한다.
 * @par 예상 출력값  반환값 == STATUS_OK, out == 5
 */
TEST(ClampUnit, ValueWithinRange_ReturnsValue)
{
    int out = -1;
    EXPECT_EQ(STATUS_OK, Clamp_Int(5, 0, 10, &out));
    EXPECT_EQ(5, out);
}

/**
 * @brief   [목적] 하한 미만의 값은 하한으로 제한되는지 확인한다. (경계값: min-1)
 * @par 입력값  value=-1, min=0, max=10, out=유효한 버퍼
 * @par 절차    -# Clamp_Int(-1, 0, 10, &out)를 호출한다.
 * @par 예상 출력값  반환값 == STATUS_OK, out == 0
 */
TEST(ClampUnit, ValueBelowMin_ReturnsMin)
{
    int out = -99;
    EXPECT_EQ(STATUS_OK, Clamp_Int(-1, 0, 10, &out));
    EXPECT_EQ(0, out);
}

/**
 * @brief   [목적] 상한 초과의 값은 상한으로 제한되는지 확인한다. (경계값: max+1)
 * @par 입력값  value=11, min=0, max=10, out=유효한 버퍼
 * @par 절차    -# Clamp_Int(11, 0, 10, &out)를 호출한다.
 * @par 예상 출력값  반환값 == STATUS_OK, out == 10
 */
TEST(ClampUnit, ValueAboveMax_ReturnsMax)
{
    int out = -99;
    EXPECT_EQ(STATUS_OK, Clamp_Int(11, 0, 10, &out));
    EXPECT_EQ(10, out);
}

/**
 * @brief   [목적] out이 NULL이면 오류를 반환하는지 확인한다. (MC/DC: out 조건의 독립성)
 * @par 입력값  value=5, min=0, max=10, out=NULL
 * @par 절차    -# Clamp_Int(5, 0, 10, NULL)를 호출한다.
 * @par 예상 출력값  반환값 == STATUS_ERR_PARAM
 */
TEST(ClampUnit, NullOut_ReturnsParamError)
{
    EXPECT_EQ(STATUS_ERR_PARAM, Clamp_Int(5, 0, 10, nullptr));
}

/**
 * @brief   [목적] min이 max보다 크면 오류를 반환하고 out을 변경하지 않는지 확인한다. (MC/DC: 범위 조건의 독립성)
 * @par 입력값  value=5, min=10, max=0, out=유효한 버퍼(-1)
 * @par 절차    -# Clamp_Int(5, 10, 0, &out)를 호출한다.
 * @par 예상 출력값  반환값 == STATUS_ERR_PARAM, out == -1 (변경 없음)
 */
TEST(ClampUnit, MinGreaterThanMax_ReturnsParamError)
{
    int out = -1;
    EXPECT_EQ(STATUS_ERR_PARAM, Clamp_Int(5, 10, 0, &out));
    EXPECT_EQ(-1, out);
}
