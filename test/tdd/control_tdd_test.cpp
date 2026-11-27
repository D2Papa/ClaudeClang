/**
 * @file    control_tdd_test.cpp
 * @brief   제어 코어 TDD 테스트 (구현 단계 산출물, 단위 테스트와 분리).
 * @trace   SRS-F-004, SRS-F-007, SRS-F-014, SRS-F-016, SSR-001, SSR-004, SSR-008, SSR-011
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
#include <gtest/gtest.h>

extern "C" {
#include "control/control_cycle.h"
}

namespace
{

RawValue_t Number(double v)
{
    RawValue_t r{};
    r.kind = RAW_NUMBER;
    r.number = v;
    return r;
}

RawValue_t Bool(int v)
{
    RawValue_t r{};
    r.kind = RAW_BOOLEAN;
    r.boolean = v;
    return r;
}

RawValue_t Enum(int v)
{
    RawValue_t r{};
    r.kind = RAW_ENUM;
    r.enum_id = v;
    return r;
}

RawInput_t NormalInput(double t)
{
    RawInput_t in{};
    in.timestamp_s = t;
    in.speed = Number(0.0);
    in.gear = Enum(GEAR_P);
    in.source_timestamp = Number(t);
    in.crash = Enum(CRASH_NONE);
    in.approach[DOOR_LEFT] = Bool(0);
    in.approach[DOOR_RIGHT] = Bool(0);
    in.fire = Bool(0);
    in.overtemp = Bool(0);
    in.adult = Bool(0);
    in.isofix[DOOR_LEFT] = Bool(0);
    in.isofix[DOOR_RIGHT] = Bool(0);
    in.ignition = Bool(1);
    in.sensor_fault = Bool(0);
    return in;
}

class ControlTdd : public ::testing::Test
{
protected:
    void SetUp() override { ASSERT_EQ(STATUS_OK, Control_Init(&ctx_)); }

    CycleResult_t Step(const RawInput_t &in)
    {
        CycleResult_t out{};
        EXPECT_EQ(STATUS_OK, Control_Step(&ctx_, &in, &out));
        return out;
    }

    Control_t ctx_{};
};

TEST_F(ControlTdd, NormalInputKeepsReleaseAndNormalState)
{
    CycleResult_t r = Step(NormalInput(0.0));
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(STATE_NORMAL, r.state);
    EXPECT_EQ(REASON_NONE, r.reason_code[DOOR_LEFT]);
}

TEST_F(ControlTdd, CrashConfirmedReleasesBothDoorsOverIsofix)
{
    RawInput_t in = NormalInput(0.0);
    in.crash = Enum(CRASH_CONFIRMED);
    in.isofix[DOOR_LEFT] = Bool(1);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_CRASH_CONFIRMED, r.priority_reason[DOOR_LEFT]);
}

TEST_F(ControlTdd, ApproachRiskLocksOnlyThatDoor)
{
    RawInput_t in = NormalInput(0.0);
    in.approach[DOOR_LEFT] = Bool(1);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
}

TEST_F(ControlTdd, SpeedThresholdLocksBothDoors)
{
    RawInput_t in = NormalInput(0.0);
    in.speed = Number(3.0);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_AUTO_LOCK, r.priority_reason[DOOR_LEFT]);
}

TEST_F(ControlTdd, IgnitionOffReleasesAndStateIsOff)
{
    RawInput_t in = NormalInput(0.0);
    in.ignition = Bool(0);
    in.isofix[DOOR_LEFT] = Bool(1);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_OFF, r.state);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_IGNITION_OFF, r.reason_code[DOOR_LEFT]);
}

TEST_F(ControlTdd, StaleSpeedMakesStateDegraded)
{
    RawInput_t in = NormalInput(1.0);
    in.source_timestamp = Number(0.79);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_DEGRADED, r.state);
}

TEST_F(ControlTdd, SensorFaultHoldsDriverCommandAndReportsFault)
{
    RawInput_t in = NormalInput(0.0);
    in.sensor_fault = Bool(1);
    in.cmd_side = Enum(SIDE_ALL);
    in.cmd_action = Enum(ACTION_LOCK);
    in.cmd_source = Enum(SOURCE_AVN);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_FAULT, r.state);
    EXPECT_EQ(1, r.warning_code);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_SENSOR_FAULT, r.reason_code[DOOR_LEFT]);
}

TEST_F(ControlTdd, SecondUnlockWithinWindowOverridesApproachRisk)
{
    RawInput_t in = NormalInput(0.0);
    in.approach[DOOR_LEFT] = Bool(1);
    in.cmd_side = Enum(SIDE_LEFT);
    in.cmd_action = Enum(ACTION_UNLOCK);
    in.cmd_source = Enum(SOURCE_PHYSICAL_BUTTON);

    CycleResult_t first = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, first.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, first.reason_code[DOOR_LEFT]);

    in = NormalInput(0.01);
    in.approach[DOOR_LEFT] = Bool(1);
    in.cmd_side = Enum(SIDE_LEFT);
    in.cmd_action = Enum(ACTION_UNLOCK);
    in.cmd_source = Enum(SOURCE_PHYSICAL_BUTTON);
    CycleResult_t second = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, second.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, second.reason_code[DOOR_LEFT]);
}

TEST_F(ControlTdd, NullPointersAreRejected)
{
    CycleResult_t out{};
    RawInput_t in = NormalInput(0.0);
    EXPECT_EQ(STATUS_ERR_PARAM, Control_Step(nullptr, &in, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Control_Step(&ctx_, nullptr, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Control_Step(&ctx_, &in, nullptr));
}

} // namespace
