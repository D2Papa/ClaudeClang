/**
 * @file    control_integration_test.cpp
 * @brief   제어 코어 컴포넌트 통합 테스트 (SWE.5): 실제 컴포넌트를 ARCH-I 인터페이스로 연결해 검증한다.
 * @trace   ARCH-I-001~007, ARCH-C-001~013
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
#include "common/params.h"
#include "control/control_cycle.h"
#include "control/freshness_monitor.h"
#include "control/input_validator.h"
#include "control/state_manager.h"
}

namespace
{

RawValue_t Num(double v)
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

RawInput_t Raw(int cycle)
{
    RawInput_t in{};
    in.timestamp_s = cycle * 0.01;
    in.speed = Num(0.0);
    in.gear = Enum(GEAR_P);
    in.source_timestamp = Num(in.timestamp_s);
    in.crash = Enum(CRASH_NONE);
    in.approach[0] = Bool(0);
    in.approach[1] = Bool(0);
    in.fire = Bool(0);
    in.overtemp = Bool(0);
    in.adult = Bool(0);
    in.isofix[0] = Bool(0);
    in.isofix[1] = Bool(0);
    in.ignition = Bool(1);
    in.sensor_fault = Bool(0);
    return in;
}

void Cmd(RawInput_t *in, int side, int action)
{
    in->cmd_side = Enum(side);
    in->cmd_action = Enum(action);
    in->cmd_source = Enum(SOURCE_PHYSICAL_BUTTON);
}

class ControlIntegration : public ::testing::Test
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

/** IT-001 목적: ARCH-I-001→002→003 연결. Validator의 INVALID가 Freshness와 StateManager를 거쳐 DEGRADED가 된다. 입력: 원천 시각이 0.3초 지난 차속. 기대: DEGRADED, 유효성 비트 0 비트 해제. 추적: ARCH-I-001, ARCH-I-002, ARCH-I-003, SSR-008, SSR-009 */
TEST_F(ControlIntegration, ValidatorFreshnessState_StaleSpeed_Degraded)
{
    RawInput_t in = Raw(50);
    in.source_timestamp = Num(in.timestamp_s - 0.3);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_DEGRADED, r.state);
    EXPECT_EQ(0U, r.input_validity & 5U);
}

/** IT-002 목적: 규칙→OverrideManager→Arbiter 연결 (ARCH-I-004, 005, 006). 입력: 접근위험(우) + unlock(우) 두 번 → 명령 없이 지속. 기대: SUPPRESSED → OVERRIDE → OVERRIDE, 좌 출력은 영향 없음. 추적: SRS-F-007, SRS-F-029, SRS-F-030, SSR-005 */
TEST_F(ControlIntegration, RulesOverrideArbiter_RightDoorOverride_LeftUntouched)
{
    RawInput_t in = Raw(0);
    in.approach[DOOR_RIGHT] = Bool(1);
    Cmd(&in, SIDE_RIGHT, ACTION_UNLOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, r.reason_code[DOOR_RIGHT]);

    in = Raw(1);
    in.approach[DOOR_RIGHT] = Bool(1);
    Cmd(&in, SIDE_RIGHT, ACTION_UNLOCK);
    r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_OVERRIDE, r.reason_code[DOOR_RIGHT]);

    in = Raw(2);
    in.approach[DOOR_RIGHT] = Bool(1);
    r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
}

/** IT-003 목적: Validator의 명령 무효 결과가 DriverCommandRule과 Arbiter를 거쳐 INPUT_INVALID로 기록된다. 입력: side 범위 밖 명령. 기대: 출력 변화 없음, reason INPUT_INVALID. 추적: SSR-007, ARCH-I-001, ARCH-I-004, ARCH-I-006 */
TEST_F(ControlIntegration, ValidatorCommandArbiter_InvalidCommand_ReasonRecorded)
{
    RawInput_t in = Raw(0);
    Cmd(&in, SIDE_MAX + 1, ACTION_LOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_INPUT_INVALID, r.reason_code[DOOR_LEFT]);
}

/** IT-004 목적: 속도 조건이 규칙과 Arbiter를 거쳐 unlock 거절 이유로 표시된다. 입력: 속도 3.0에서 LOCK된 상태에서 unlock. 기대: LOCK 유지(자동 잠금), unlock은 순위에서 밀림. 추적: SRS-F-003, SRS-F-004, SRS-F-017 */
TEST_F(ControlIntegration, SpeedRules_UnlockAtThreshold_AutoLockWins)
{
    RawInput_t in = Raw(0);
    in.speed = Num(3.0);
    Cmd(&in, SIDE_ALL, ACTION_UNLOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_AUTO_LOCK, r.priority_reason[DOOR_LEFT]);
}

/** IT-005 목적: StateManager의 OFF가 Arbiter의 순위 제외로 이어진다. 입력: ignition off + ISOFIX + 접근위험 + lock 명령 + 속도 3.0. 기대: RELEASE, OFF, IGNITION_OFF. 추적: SRS-F-016, SRS-F-031, ARCH-I-003, ARCH-I-006 */
TEST_F(ControlIntegration, StateArbiter_IgnitionOff_AllLockRulesIgnored)
{
    for (int c = 0; c < 3; c++)
    {
        RawInput_t in = Raw(c);
        in.ignition = Bool(0);
        in.isofix[0] = Bool(1);
        in.approach[1] = Bool(1);
        in.speed = Num(3.0);
        Cmd(&in, SIDE_ALL, ACTION_LOCK);
        CycleResult_t r = Step(in);
        EXPECT_EQ(STATE_OFF, r.state);
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
        EXPECT_EQ(REASON_IGNITION_OFF, r.reason_code[DOOR_LEFT]);
    }
}

/** IT-006 목적: 컨텍스트(직전 출력) 전달 인터페이스(ARCH-I-007). 입력: lock 명령 후 입력 없는 주기. 기대: 명령 결과가 이후 주기에 직전 출력으로 유지된다. 추적: ARCH-I-007, SRS-F-002 */
TEST_F(ControlIntegration, ControlContext_PreviousOutputCarriedAcrossCycles)
{
    RawInput_t in = Raw(0);
    Cmd(&in, SIDE_LEFT, ACTION_LOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    r = Step(Raw(1));
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, r.reason_code[DOOR_LEFT]);
}

/** IT-007 목적: 긴급 입력이 충돌 외 규칙들과의 중재를 통과한다. 입력: 화재 + ISOFIX + 접근위험, 이후 충돌 CONFIRMED 추가. 기대: FIRE로 RELEASE, 충돌이 추가되면 CRASH_CONFIRMED. 추적: SRS-F-014, SSR-001, SRS-F-017 */
TEST_F(ControlIntegration, EmergencyAndCrash_PriorityThroughStack)
{
    RawInput_t in = Raw(0);
    in.fire = Bool(1);
    in.isofix[0] = Bool(1);
    in.approach[1] = Bool(1);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_FIRE, r.priority_reason[DOOR_LEFT]);

    in = Raw(1);
    in.fire = Bool(1);
    in.crash = Enum(CRASH_CONFIRMED);
    r = Step(in);
    EXPECT_EQ(REASON_CRASH_CONFIRMED, r.priority_reason[DOOR_LEFT]);
}

/** IT-008 목적: Freshness→State 컨텍스트 오류 전파 (ARCH-I-002). 입력: 시각이 되돌아간 입력. 기대: ERR_STATE. 추적: ARCH-I-002, ARCH-I-007 */
TEST_F(ControlIntegration, FreshnessError_PropagatesThroughControlStep)
{
    Step(Raw(10));
    RawInput_t back = Raw(5);
    CycleResult_t out{};
    EXPECT_EQ(STATUS_ERR_STATE, Control_Step(&ctx_, &back, &out));
}

} // namespace
