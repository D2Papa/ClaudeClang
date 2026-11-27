/**
 * @file    rules_unit_test.cpp
 * @brief   규칙 7개(DD-M-004~010) 단위 테스트.
 * @trace   DD-F-010~017, SSR-001~006, SRS-F-001~004, SRS-F-014~016
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

typedef Status_t (*RuleFn)(const RuleInput_t *, RuleResult_t *);

/** 목적: 모든 규칙의 널 인자 거절. 입력: in/out NULL. 기대: ERR_PARAM. 추적: UT-040 -> DD-F-010~017. 기법: 오류 추정(MC/DC) */
TEST(RulesUnit, AllRules_NullPointers_ReturnParamError)
{
    const RuleFn fns[] = {Crash_Evaluate,      Emergency_Evaluate,     Ignition_Evaluate, ApproachRisk_Evaluate,
                          Isofix_Evaluate,     DriverCommand_Evaluate, AutoLock_Evaluate};
    Snapshot_t s = NormalSnapshot();
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};
    for (RuleFn f : fns)
    {
        EXPECT_EQ(STATUS_ERR_PARAM, f(nullptr, &out));
        EXPECT_EQ(STATUS_ERR_PARAM, f(&in, nullptr));
    }
}

/** 목적: 조건이 없을 때 모든 규칙이 결과를 비활성으로 초기화한다. 입력: 정상 스냅샷, 쓰레기 값으로 채운 out. 기대: 모든 필드 0(RELEASE/NONE). 추적: UT-041 -> DD-F-010~017. 기법: 동치 클래스 */
TEST(RulesUnit, AllRules_NormalSnapshot_ResultCleared)
{
    const RuleFn fns[] = {Crash_Evaluate,      Emergency_Evaluate,     Ignition_Evaluate, ApproachRisk_Evaluate,
                          Isofix_Evaluate,     DriverCommand_Evaluate, AutoLock_Evaluate};
    Snapshot_t s = NormalSnapshot();
    RuleInput_t in = MakeRuleInput(&s);
    for (RuleFn f : fns)
    {
        RuleResult_t out = DirtyResult();
        EXPECT_EQ(STATUS_OK, f(&in, &out));
        for (int d = 0; d < DOOR_COUNT; d++)
        {
            EXPECT_EQ(0, out.active[d]);
            EXPECT_EQ(REASON_NONE, out.reason[d]);
            EXPECT_EQ(REASON_NONE, out.info_reason[d]);
            EXPECT_EQ(0, out.suppressed[d]);
        }
    }
}

/** 목적: 충돌 규칙. 입력: CONFIRMED / PENDING / NONE / 무효 CONFIRMED 값. 기대: CONFIRMED만 양쪽 RELEASE+이유, PENDING은 정보 이유만, 나머지는 비활성. 추적: UT-042 -> DD-F-010, SSR-001, SSR-003. 기법: 동치 클래스, MC/DC */
TEST(RulesUnit, Crash_Statuses_MapToExpectedResult)
{
    Snapshot_t s = NormalSnapshot();
    RuleResult_t out{};

    s.crash = {CRASH_CONFIRMED, VALIDITY_VALID};
    RuleInput_t in = MakeRuleInput(&s);
    Crash_Evaluate(&in, &out);
    for (int d = 0; d < DOOR_COUNT; d++)
    {
        EXPECT_EQ(1, out.active[d]);
        EXPECT_EQ(OUTPUT_RELEASE, out.effect[d]);
        EXPECT_EQ(REASON_CRASH_CONFIRMED, out.reason[d]);
    }

    s.crash = {CRASH_PENDING, VALIDITY_VALID};
    Crash_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(REASON_CRASH_PENDING, out.info_reason[DOOR_LEFT]);
    EXPECT_EQ(REASON_CRASH_PENDING, out.info_reason[DOOR_RIGHT]);

    s.crash = {CRASH_NONE, VALIDITY_VALID};
    Crash_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, out.info_reason[DOOR_LEFT]);

    s.crash = {CRASH_CONFIRMED, VALIDITY_INVALID};
    Crash_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
}

/** 목적: 긴급 강제 해제 입력별 이유와 우선순위. 입력: fire/overtemp/adult 단독, 복수, 무효 조합. 기대: 해당 입력의 이유(화재>과온>성인), 무효는 무시. 추적: UT-043 -> DD-F-011, SRS-F-014. 기법: 결정표, MC/DC */
TEST(RulesUnit, Emergency_Inputs_SelectReasonByPriority)
{
    struct Case
    {
        Flag_t fire, over, adult;
        ReasonCode_t expect;
    };
    const Case cases[] = {
        {F(1), F(0), F(0), REASON_FIRE},
        {F(0), F(1), F(0), REASON_OVERTEMP},
        {F(0), F(0), F(1), REASON_ADULT_PRESENT},
        {F(1), F(1), F(1), REASON_FIRE},
        {F(0), F(1), F(1), REASON_OVERTEMP},
        {F(0), F(0), F(0), REASON_NONE},
        {F(0, VALIDITY_INVALID), F(0), F(0), REASON_NONE},
        {F(0, VALIDITY_INVALID), F(1), F(0), REASON_OVERTEMP},
        {F(0), F(0, VALIDITY_INVALID), F(1), REASON_ADULT_PRESENT},
        {F(0), F(0), F(0, VALIDITY_INVALID), REASON_NONE},
    };
    for (const Case &c : cases)
    {
        Snapshot_t s = NormalSnapshot();
        s.fire = c.fire;
        s.overtemp = c.over;
        s.adult = c.adult;
        RuleInput_t in = MakeRuleInput(&s);
        RuleResult_t out{};
        Emergency_Evaluate(&in, &out);
        EXPECT_EQ(c.expect != REASON_NONE ? 1 : 0, out.active[DOOR_LEFT]);
        EXPECT_EQ(c.expect, out.reason[DOOR_LEFT]);
        if (c.expect != REASON_NONE)
        {
            EXPECT_EQ(1, out.active[DOOR_RIGHT]);
            EXPECT_EQ(OUTPUT_RELEASE, out.effect[DOOR_RIGHT]);
        }
    }
}

/** 목적: ignition off 규칙. 입력: 유효 FALSE / 유효 TRUE / 무효. 기대: FALSE만 양쪽 RELEASE+IGNITION_OFF. 추적: UT-044 -> DD-F-012, SRS-F-016. 기법: MC/DC */
TEST(RulesUnit, Ignition_Values_OnlyValidFalseActive)
{
    Snapshot_t s = NormalSnapshot();
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};

    s.ignition = F(0);
    Ignition_Evaluate(&in, &out);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_RIGHT]);
    EXPECT_EQ(OUTPUT_RELEASE, out.effect[DOOR_RIGHT]);
    EXPECT_EQ(REASON_IGNITION_OFF, out.reason[DOOR_LEFT]);

    s.ignition = F(1);
    Ignition_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);

    s.ignition = F(0, VALIDITY_INVALID);
    Ignition_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
}

/** 목적: 접근위험 LOCK의 도어 독립. 입력: 좌만/우만/둘 다 TRUE, 무효 TRUE. 기대: TRUE인 도어만 LOCK+APPROACH_RISK_LOCK. 추적: UT-045 -> DD-F-013, SSR-004, SSR-006. 기법: 동치 클래스, MC/DC */
TEST(RulesUnit, ApproachRisk_PerDoor_LocksOnlyRiskDoor)
{
    Snapshot_t s = NormalSnapshot();
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};

    s.approach[DOOR_LEFT] = F(1);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, out.effect[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_LOCK, out.reason[DOOR_LEFT]);
    EXPECT_EQ(0, out.active[DOOR_RIGHT]);

    s.approach[DOOR_LEFT] = F(0);
    s.approach[DOOR_RIGHT] = F(1);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_RIGHT]);

    s.approach[DOOR_LEFT] = F(1);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_RIGHT]);

    s.approach[DOOR_LEFT] = F(0, VALIDITY_INVALID);
    s.approach[DOOR_RIGHT] = F(0);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
}

/** 목적: 접근위험 중 override가 지속되면 해당 도어는 평가하지 않는다. 입력: 좌 접근위험 TRUE + override active[좌]. 기대: 좌 비활성, 우 영향 없음. 추적: UT-046 -> DD-F-013, SRS-F-030. 기법: MC/DC */
TEST(RulesUnit, ApproachRisk_OverrideActive_DoorSkipped)
{
    Snapshot_t s = NormalSnapshot();
    s.approach[DOOR_LEFT] = F(1);
    s.approach[DOOR_RIGHT] = F(1);
    RuleInput_t in = MakeRuleInput(&s);
    in.override_status.active[DOOR_LEFT] = 1;
    RuleResult_t out{};
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_RIGHT]);
}

/** 목적: unlock 요청 억제 표시 조건. 입력: 위험 도어에 유효 unlock이 지정됨 / lock 명령 / 무효 명령 / 다른 도어만 지정. 기대: 지정된 unlock만 suppressed+APPROACH_RISK_SUPPRESSED. 추적: UT-047 -> DD-F-013, SSR-005. 기법: MC/DC(조건 독립성) */
TEST(RulesUnit, ApproachRisk_UnlockRequest_SuppressionConditions)
{
    Snapshot_t s = NormalSnapshot();
    s.approach[DOOR_LEFT] = F(1);
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};

    SetValidCommand(&s, SIDE_LEFT, ACTION_UNLOCK);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(1, out.suppressed[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, out.reason[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, out.effect[DOOR_LEFT]);

    SetValidCommand(&s, SIDE_ALL, ACTION_UNLOCK);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(1, out.suppressed[DOOR_LEFT]);

    SetValidCommand(&s, SIDE_LEFT, ACTION_LOCK);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(0, out.suppressed[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_LOCK, out.reason[DOOR_LEFT]);

    SetValidCommand(&s, SIDE_LEFT, ACTION_UNLOCK);
    s.cmd.validity = VALIDITY_INVALID;
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(0, out.suppressed[DOOR_LEFT]);

    SetValidCommand(&s, SIDE_RIGHT, ACTION_UNLOCK);
    ApproachRisk_Evaluate(&in, &out);
    EXPECT_EQ(0, out.suppressed[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);
}

/** 목적: ISOFIX 규칙의 쪽별 독립. 입력: 좌만/우만 TRUE, 무효. 기대: TRUE인 쪽만 LOCK+ISOFIX_LOCK. 추적: UT-048 -> DD-F-014, SRS-F-015. 기법: MC/DC */
TEST(RulesUnit, Isofix_PerSide_LocksOnlyConnectedSide)
{
    Snapshot_t s = NormalSnapshot();
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};

    s.isofix[DOOR_LEFT] = F(1);
    Isofix_Evaluate(&in, &out);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, out.effect[DOOR_LEFT]);
    EXPECT_EQ(REASON_ISOFIX_LOCK, out.reason[DOOR_LEFT]);
    EXPECT_EQ(0, out.active[DOOR_RIGHT]);

    s.isofix[DOOR_LEFT] = F(0);
    s.isofix[DOOR_RIGHT] = F(1);
    Isofix_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_RIGHT]);

    s.isofix[DOOR_RIGHT] = F(0, VALIDITY_INVALID);
    Isofix_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_RIGHT]);
}

/** 목적: 자동 잠금 속도 경계 (SRS-L-004). 입력: 2.9, 3.0, 300.0, 무효 속도. 기대: 3.0 이상만 양쪽 LOCK+AUTO_LOCK. 추적: UT-049 -> DD-F-017, SRS-F-004. 기법: 경계값 분석, MC/DC */
TEST(RulesUnit, AutoLock_SpeedBoundary_LocksFromThreshold)
{
    Snapshot_t s = NormalSnapshot();
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};

    s.speed = {2.9, VALIDITY_VALID};
    AutoLock_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);

    s.speed = {3.0, VALIDITY_VALID};
    AutoLock_Evaluate(&in, &out);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_RIGHT]);
    EXPECT_EQ(OUTPUT_LOCK, out.effect[DOOR_RIGHT]);
    EXPECT_EQ(REASON_AUTO_LOCK, out.reason[DOOR_LEFT]);

    s.speed = {300.0, VALIDITY_VALID};
    AutoLock_Evaluate(&in, &out);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);

    s.speed = {0.0, VALIDITY_INVALID};
    AutoLock_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
}

/** 목적: 명령이 없거나 무효일 때의 운전자 명령 규칙. 입력: 명령 없음(present 0) / present 1이지만 무효. 기대: 비활성, 무효는 INPUT_INVALID 정보 이유. 추적: UT-050 -> DD-F-015, SSR-007. 기법: 동치 클래스, MC/DC */
TEST(RulesUnit, DriverCommand_AbsentAndInvalid_NoEffect)
{
    Snapshot_t s = NormalSnapshot();
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};

    DriverCommand_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, out.info_reason[DOOR_LEFT]);

    s.cmd.present = 1;
    s.cmd.validity = VALIDITY_INVALID;
    DriverCommand_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(REASON_INPUT_INVALID, out.info_reason[DOOR_LEFT]);
    EXPECT_EQ(REASON_INPUT_INVALID, out.info_reason[DOOR_RIGHT]);
}

/** 목적: lock 명령은 속도와 무관하게 지정 도어에만 적용된다. 입력: side 좌/우/전체, action lock, 속도 0과 200. 기대: 지정 도어 LOCK+CMD_APPLIED+lock_requested. 추적: UT-051 -> DD-F-015, SRS-F-002. 기법: 동치 클래스 */
TEST(RulesUnit, DriverCommand_Lock_AppliesToTargetedDoorsAtAnySpeed)
{
    for (double speed : {0.0, 200.0})
    {
        for (int side = 0; side <= SIDE_MAX; side++)
        {
            Snapshot_t s = NormalSnapshot();
            s.speed = {speed, VALIDITY_VALID};
            SetValidCommand(&s, side, ACTION_LOCK);
            RuleInput_t in = MakeRuleInput(&s);
            RuleResult_t out{};
            DriverCommand_Evaluate(&in, &out);
            for (int d = 0; d < DOOR_COUNT; d++)
            {
                int targeted = (side == SIDE_ALL) || (side == d) ? 1 : 0;
                EXPECT_EQ(targeted, out.active[d]);
                EXPECT_EQ(targeted, out.lock_requested[d]);
                if (targeted == 1)
                {
                    EXPECT_EQ(OUTPUT_LOCK, out.effect[d]);
                    EXPECT_EQ(REASON_CMD_APPLIED, out.reason[d]);
                }
            }
        }
    }
}

/** 목적: unlock 명령은 속도 3.0 km/h 미만에서만 허용 (SRS-F-003). 입력: 2.9 / 3.0 / 무효 속도. 기대: 2.9는 RELEASE+허용, 그 외는 CMD_REJECTED_SPEED 정보 이유. 추적: UT-052 -> DD-F-015, SRS-F-003, SRS-L-004. 기법: 경계값 분석, MC/DC */
TEST(RulesUnit, DriverCommand_Unlock_AllowedOnlyBelowThreshold)
{
    Snapshot_t s = NormalSnapshot();
    SetValidCommand(&s, SIDE_ALL, ACTION_UNLOCK);
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};

    s.speed = {2.9, VALIDITY_VALID};
    DriverCommand_Evaluate(&in, &out);
    for (int d = 0; d < DOOR_COUNT; d++)
    {
        EXPECT_EQ(1, out.active[d]);
        EXPECT_EQ(OUTPUT_RELEASE, out.effect[d]);
        EXPECT_EQ(1, out.unlock_allowed[d]);
        EXPECT_EQ(REASON_CMD_APPLIED, out.reason[d]);
    }

    s.speed = {3.0, VALIDITY_VALID};
    DriverCommand_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(0, out.unlock_allowed[DOOR_LEFT]);
    EXPECT_EQ(REASON_CMD_REJECTED_SPEED, out.info_reason[DOOR_LEFT]);

    s.speed = {0.0, VALIDITY_INVALID};
    DriverCommand_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(REASON_CMD_REJECTED_SPEED, out.info_reason[DOOR_RIGHT]);
}

/** 목적: 지정되지 않은 도어는 명령의 영향을 받지 않는다. 입력: side=우, action=unlock, 속도 0. 기대: 좌 비활성, 우 활성. 추적: UT-053 -> DD-F-015, SRS-F-002. 기법: 동치 클래스 */
TEST(RulesUnit, DriverCommand_SingleSide_OtherDoorUntouched)
{
    Snapshot_t s = NormalSnapshot();
    SetValidCommand(&s, SIDE_RIGHT, ACTION_UNLOCK);
    RuleInput_t in = MakeRuleInput(&s);
    RuleResult_t out{};
    DriverCommand_Evaluate(&in, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, out.info_reason[DOOR_LEFT]);
    EXPECT_EQ(1, out.active[DOOR_RIGHT]);
}

} // namespace
