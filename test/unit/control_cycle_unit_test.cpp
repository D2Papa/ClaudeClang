/**
 * @file    control_cycle_unit_test.cpp
 * @brief   ControlCycle(DD-M-013) 단위 테스트 (실제 하위 모듈을 그대로 사용).
 * @trace   DD-F-025~028, SRS-Q-001, SRS-C-002, SRS-C-005, SSR-002
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

#include "unit_helpers.h"

using namespace uh;

namespace
{

class ControlUnit : public ::testing::Test
{
protected:
    void SetUp() override { ASSERT_EQ(STATUS_OK, Control_Init(&ctx_)); }

    CycleResult_t Step(const RawInput_t &in)
    {
        CycleResult_t out{};
        EXPECT_EQ(STATUS_OK, Control_Step(&ctx_, &in, &out));
        return out;
    }

    RawInput_t At(int cycle) { return NormalRaw(cycle * CYCLE_PERIOD_S); }

    Control_t ctx_{};
};

/** 목적: 초기화 널 거절과 초기 상태. 입력: NULL / 유효 ctx. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: ERR_PARAM / OK, 직전 출력 RELEASE. 추적: UT-080 -> DD-F-025. 기법: 오류 추정 */
TEST_F(ControlUnit, Init_NullAndValid_ReturnsExpected)
{
    Control_t c{};
    EXPECT_EQ(STATUS_ERR_PARAM, Control_Init(nullptr));
    EXPECT_EQ(STATUS_OK, Control_Init(&c));
    EXPECT_EQ(1, c.initialized);
    EXPECT_EQ(OUTPUT_RELEASE, c.previous[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, c.previous[DOOR_RIGHT]);
}

/** 목적: Step의 인자 오류와 미초기화. 입력: ctx/raw/out NULL, 초기화 안 한 ctx. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: ERR_PARAM, ERR_STATE. 추적: UT-081 -> DD-F-026. 기법: 오류 추정(MC/DC) */
TEST_F(ControlUnit, Step_InvalidArguments_ReturnErrors)
{
    RawInput_t in = At(0);
    CycleResult_t out{};
    EXPECT_EQ(STATUS_ERR_PARAM, Control_Step(nullptr, &in, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Control_Step(&ctx_, nullptr, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Control_Step(&ctx_, &in, nullptr));
    Control_t raw{};
    EXPECT_EQ(STATUS_ERR_STATE, Control_Step(&raw, &in, &out));
}

/** 목적: 정상 입력의 최종 결과와 유효성 비트 마스크. 입력: NormalRaw. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 양쪽 RELEASE, NORMAL, 경고 0, input_validity 0x1FFF. 추적: UT-082 -> DD-F-026, DD-F-028. 기법: 동치 클래스 */
TEST_F(ControlUnit, Step_NormalInput_ReleaseNormalAllValid)
{
    CycleResult_t r = Step(At(0));
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(STATE_NORMAL, r.state);
    EXPECT_EQ(0, r.warning_code);
    EXPECT_EQ(REASON_NONE, r.reason_code[DOOR_LEFT]);
    EXPECT_EQ(0x1FFFU, r.input_validity);
}

/** 목적: 입력 유효성 비트가 항목별로 대응한다. 입력: 필드 하나씩 형식 오류로 만든 입력 13가지. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 해당 비트만 0. 추적: UT-083 -> DD-F-028, SRS-F-019. 기법: 동치 클래스(전수 13) */
TEST_F(ControlUnit, Step_EachInvalidField_ClearsOnlyItsBit)
{
    for (int field = 0; field < 13; field++)
    {
        Control_t c{};
        Control_Init(&c);
        RawInput_t in = At(0);
        RawValue_t bad = Kind(RAW_OTHER);
        RawValue_t *targets[13] = {&in.speed,        &in.gear,        &in.source_timestamp, &in.crash,
                                   &in.approach[0],  &in.approach[1], &in.fire,             &in.overtemp,
                                   &in.adult,        &in.isofix[0],   &in.isofix[1],        &in.ignition,
                                   &in.sensor_fault};
        *targets[field] = bad;
        CycleResult_t out{};
        EXPECT_EQ(STATUS_OK, Control_Step(&c, &in, &out));
        EXPECT_EQ(0x1FFFU & ~(1U << field), out.input_validity) << field;
    }
}

/** 목적: 시각 역행 시 오류를 반환하고 직전 출력은 바뀌지 않는다. 입력: 속도 3.0으로 LOCK 후 시각이 과거인 입력, 이어서 속도 0 입력. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: ERR_STATE, 다음 주기는 직전 LOCK 유지. 추적: UT-084 -> DD-F-026, SRS-Q-001. 기법: 오류 추정, 시나리오 */
TEST_F(ControlUnit, Step_TimeGoesBackwards_ErrorAndPreviousOutputKept)
{
    RawInput_t fast = At(10);
    fast.speed = Num(3.0);
    CycleResult_t r = Step(fast);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);

    RawInput_t past = At(5);
    CycleResult_t out{};
    EXPECT_EQ(STATUS_ERR_STATE, Control_Step(&ctx_, &past, &out));

    r = Step(At(11)); // 속도 0, 규칙 없음 -> 직전 출력 유지
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_NONE, r.reason_code[DOOR_LEFT]);
}

/** 목적: 충돌 CONFIRMED 시 같은 주기에 양쪽 RELEASE (SSR-002: 300 ms 이내=30주기 이내). 입력: LOCK 상태에서 CONFIRMED 주입. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 즉시 RELEASE, 이후 30주기 유지. 추적: UT-085 -> DD-F-026, SSR-001, SSR-002. 기법: 시나리오 */
TEST_F(ControlUnit, Step_CrashConfirmed_ReleasesWithinSameCycleAndHolds)
{
    RawInput_t fast = At(0);
    fast.speed = Num(50.0);
    Step(fast);
    for (int c = 1; c <= 30; c++)
    {
        RawInput_t in = At(c);
        in.speed = Num(50.0);
        in.crash = Enum(CRASH_CONFIRMED);
        CycleResult_t r = Step(in);
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]) << c;
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]) << c;
        EXPECT_EQ(REASON_CRASH_CONFIRMED, r.priority_reason[DOOR_LEFT]) << c;
    }
}

/** 목적: 차속이 오래되면 DEGRADED이고 운전자 명령은 적용되지 않는다 (SSR-008~011). 입력: 원천 시각이 0.25초 지난 차속 + lock 명령. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: DEGRADED, 출력 RELEASE 유지, DEGRADED_STALE. 추적: UT-086 -> DD-F-026, SSR-008, SSR-009, SSR-010. 기법: 시나리오 */
TEST_F(ControlUnit, Step_StaleSpeedWithCommand_DegradedHoldsOutput)
{
    RawInput_t in = At(30);
    in.source_timestamp = Num(0.05);
    SetCommand(&in, SIDE_ALL, ACTION_LOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_DEGRADED, r.state);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_DEGRADED_STALE, r.reason_code[DOOR_LEFT]);
    EXPECT_EQ(0U, r.input_validity & 1U);
}

/** 목적: 센서 고장은 FAULT와 경고코드를 내고 명령을 적용하지 않지만 접근위험 LOCK과 충돌 해제는 적용한다 (SSR-011, 012, 013, 016). 입력: sensor_fault TRUE + lock 명령 / + 접근위험 / + 충돌 CONFIRMED. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: FAULT, W-001, 각각 RELEASE 유지 / LOCK / RELEASE. 추적: UT-087 -> DD-F-026, SSR-011, SSR-012, SSR-016. 기법: 결정표 */
TEST_F(ControlUnit, Step_SensorFault_ReportsFaultAndAppliesSafetyRules)
{
    RawInput_t in = At(0);
    in.sensor_fault = Bool(1);
    SetCommand(&in, SIDE_ALL, ACTION_LOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_FAULT, r.state);
    EXPECT_EQ(1, r.warning_code);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_SENSOR_FAULT, r.reason_code[DOOR_LEFT]);

    in = At(1);
    in.sensor_fault = Bool(1);
    in.approach[DOOR_RIGHT] = Bool(1);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);

    in = At(2);
    in.sensor_fault = Bool(1);
    in.crash = Enum(CRASH_CONFIRMED);
    r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(STATE_FAULT, r.state);
}

/** 목적: override 전체 흐름. 입력: 접근위험(좌) + unlock → 재입력 → 명령 없이 지속 → 속도 3.0으로 종료. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: LOCK+SUPPRESSED → RELEASE+OVERRIDE → RELEASE+OVERRIDE → 좌 LOCK+APPROACH_RISK_LOCK(우는 AUTO_LOCK). 추적: UT-088 -> DD-F-026, SRS-F-007, SRS-F-029, SRS-F-030. 기법: 시나리오 */
TEST_F(ControlUnit, Step_OverrideLifecycle_EstablishPersistEndBySpeed)
{
    RawInput_t in = At(0);
    in.approach[DOOR_LEFT] = Bool(1);
    SetCommand(&in, SIDE_LEFT, ACTION_UNLOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, r.reason_code[DOOR_LEFT]);

    in = At(1);
    in.approach[DOOR_LEFT] = Bool(1);
    SetCommand(&in, SIDE_LEFT, ACTION_UNLOCK);
    r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, r.priority_reason[DOOR_LEFT]);

    in = At(2);
    in.approach[DOOR_LEFT] = Bool(1);
    r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, r.reason_code[DOOR_LEFT]);

    in = At(3);
    in.approach[DOOR_LEFT] = Bool(1);
    in.speed = Num(3.0);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_LOCK, r.priority_reason[DOOR_LEFT]); // 접근위험(순위 4)이 자동 잠금(순위 7)보다 높다
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_AUTO_LOCK, r.priority_reason[DOOR_RIGHT]);
}

/** 목적: override 지속 중 접근위험 입력이 무효가 되거나 lock 명령이 들어오면 종료한다. 입력: override 성립 후 (a) 접근위험 형식 오류 (b) lock 명령. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: (a) 지속 종료로 직전 출력 유지(규칙 없음) 후 접근위험 복귀 시 LOCK (b) LOCK+CMD_APPLIED. 추적: UT-089 -> DD-F-027, SRS-F-030, A-18. 기법: 시나리오 */
TEST_F(ControlUnit, Step_OverrideEnds_OnInvalidRiskAndOnLockCommand)
{
    auto establish = [&](int base) {
        RawInput_t in = At(base);
        in.approach[DOOR_LEFT] = Bool(1);
        SetCommand(&in, SIDE_LEFT, ACTION_UNLOCK);
        Step(in);
        in = At(base + 1);
        in.approach[DOOR_LEFT] = Bool(1);
        SetCommand(&in, SIDE_LEFT, ACTION_UNLOCK);
        CycleResult_t r = Step(in);
        ASSERT_EQ(REASON_OVERRIDE, r.reason_code[DOOR_LEFT]);
    };

    establish(0);
    RawInput_t in = At(2);
    in.approach[DOOR_LEFT] = Kind(RAW_OTHER);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_DEGRADED, r.state);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_NE(REASON_OVERRIDE, r.reason_code[DOOR_LEFT]);

    Control_Init(&ctx_);
    establish(10);
    in = At(12);
    in.approach[DOOR_LEFT] = Bool(1);
    SetCommand(&in, SIDE_LEFT, ACTION_LOCK);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_LOCK, r.priority_reason[DOOR_LEFT]);
}

/** 목적: 속도가 무효이면 override 입력의 속도 조건이 거짓이다. 입력: 접근위험 + unlock 후 속도 형식 오류 + 재입력. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: DEGRADED이므로 재입력이 적용되지 않아 LOCK 유지. 추적: UT-090 -> DD-F-027, SSR-010. 기법: MC/DC */
TEST_F(ControlUnit, Step_InvalidSpeed_DoesNotEstablishOverride)
{
    RawInput_t in = At(0);
    in.approach[DOOR_LEFT] = Bool(1);
    SetCommand(&in, SIDE_LEFT, ACTION_UNLOCK);
    Step(in);
    in = At(1);
    in.approach[DOOR_LEFT] = Bool(1);
    in.speed = Kind(RAW_OTHER);
    SetCommand(&in, SIDE_LEFT, ACTION_UNLOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_DEGRADED, r.state);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
}

/** 목적: 결정론 (SRS-Q-001). 입력: 같은 입력 열을 두 컨텍스트에 재생. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 모든 주기에서 출력, 이유, state, 유효성이 같다. 추적: UT-091 -> DD-F-026, SRS-Q-001. 기법: 동일 입력 재생 */
TEST_F(ControlUnit, Step_SameInputSequence_SameResults)
{
    Control_t a{}, b{};
    Control_Init(&a);
    Control_Init(&b);
    for (int c = 0; c < 200; c++)
    {
        RawInput_t in = At(c);
        in.speed = Num((c % 7) * 1.0);
        in.approach[c % 2] = Bool(c % 3 == 0 ? 1 : 0);
        if (c % 11 == 0)
        {
            SetCommand(&in, c % 3, c % 2);
        }
        CycleResult_t ra{}, rb{};
        ASSERT_EQ(STATUS_OK, Control_Step(&a, &in, &ra));
        ASSERT_EQ(STATUS_OK, Control_Step(&b, &in, &rb));
        EXPECT_EQ(0, memcmp(&ra, &rb, sizeof(ra))) << c;
    }
}

} // namespace
