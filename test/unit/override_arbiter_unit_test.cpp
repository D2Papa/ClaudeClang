/**
 * @file    override_arbiter_unit_test.cpp
 * @brief   OverrideManager(DD-M-011)와 Arbiter(DD-M-012) 단위 테스트.
 * @trace   DD-F-018~024, SRS-F-007, SRS-F-017, SRS-F-028~031, SSR-010, SSR-011, SSR-013, SSR-016
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

OverrideInput_t Inp(uint32_t cycle)
{
    OverrideInput_t in{};
    in.cycle = cycle;
    in.state = STATE_NORMAL;
    return in;
}

/** 목적: 컨텍스트 초기화/갱신의 널 거절. 입력: NULL. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: ERR_PARAM, 정상 초기화는 0. 추적: UT-060 -> DD-F-018, DD-F-019. 기법: 오류 추정(MC/DC) */
TEST(OverrideUnit, InitAndUpdate_NullPointers_ReturnParamError)
{
    Override_t ctx;
    OverrideInput_t in = Inp(1);
    OverrideStatus_t out{};
    EXPECT_EQ(STATUS_ERR_PARAM, Override_Init(nullptr));
    EXPECT_EQ(STATUS_OK, Override_Init(&ctx));
    EXPECT_EQ(0, ctx.active[DOOR_LEFT]);
    EXPECT_EQ(0, ctx.suppressed_valid[DOOR_RIGHT]);
    EXPECT_EQ(STATUS_ERR_PARAM, Override_Update(nullptr, &in, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Override_Update(&ctx, nullptr, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Override_Update(&ctx, &in, nullptr));
}

/** 목적: 억제 기록 조건 (직전 주기 억제 + NORMAL + 기록 없음/만료). 입력: prev_suppressed, state, 기존 기록 조합. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 기록 주기 = 현재-1 또는 기존 유지. 추적: UT-061 -> DD-F-020, SRS-F-007. 기법: 결정표, MC/DC */
TEST(OverrideUnit, Record_Conditions_RecordsOnlyWhenNeeded)
{
    Override_t ctx;
    OverrideStatus_t out{};

    Override_Init(&ctx);
    OverrideInput_t in = Inp(10);
    in.risk_true[DOOR_LEFT] = 1;
    in.prev_suppressed[DOOR_LEFT] = 1;
    Override_Update(&ctx, &in, &out);
    EXPECT_EQ(1, ctx.suppressed_valid[DOOR_LEFT]);
    EXPECT_EQ(9U, ctx.suppressed_cycle[DOOR_LEFT]);
    EXPECT_EQ(0, ctx.suppressed_valid[DOOR_RIGHT]);

    Override_Init(&ctx);
    in = Inp(10);
    in.risk_true[DOOR_LEFT] = 1;
    Override_Update(&ctx, &in, &out); // prev_suppressed 0
    EXPECT_EQ(0, ctx.suppressed_valid[DOOR_LEFT]);

    in.prev_suppressed[DOOR_LEFT] = 1;
    in.state = STATE_DEGRADED;
    Override_Update(&ctx, &in, &out); // NORMAL이 아님
    EXPECT_EQ(0, ctx.suppressed_valid[DOOR_LEFT]);

    Override_Init(&ctx);
    ctx.suppressed_valid[DOOR_LEFT] = 1;
    ctx.suppressed_cycle[DOOR_LEFT] = 100;
    in = Inp(105);
    in.risk_true[DOOR_LEFT] = 1;
    in.prev_suppressed[DOOR_LEFT] = 1;
    Override_Update(&ctx, &in, &out); // 유효한 기록이 있고 만료되지 않음 -> 유지
    EXPECT_EQ(100U, ctx.suppressed_cycle[DOOR_LEFT]);

    in = Inp(1200);
    in.risk_true[DOOR_LEFT] = 1;
    in.prev_suppressed[DOOR_LEFT] = 1;
    Override_Update(&ctx, &in, &out); // 만료 -> 새로 기록
    EXPECT_EQ(1199U, ctx.suppressed_cycle[DOOR_LEFT]);
    EXPECT_EQ(1, ctx.suppressed_valid[DOOR_LEFT]);
}

/** 목적: 두 번째 unlock 재입력으로 override 성립. 입력: 주기 10 억제 후 주기 11 허용된 unlock. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 주기 11에 active, 기록 해제. 추적: UT-062 -> DD-F-020, SRS-F-007. 기법: 시나리오 */
TEST(OverrideUnit, Establish_SecondUnlockAfterSuppression_Active)
{
    Override_t ctx;
    OverrideStatus_t out{};
    Override_Init(&ctx);

    OverrideInput_t first = Inp(10);
    first.risk_true[DOOR_LEFT] = 1;
    first.unlock_allowed[DOOR_LEFT] = 1;
    Override_Update(&ctx, &first, &out);
    EXPECT_EQ(0, out.active[DOOR_LEFT]);

    OverrideInput_t second = Inp(11);
    second.risk_true[DOOR_LEFT] = 1;
    second.unlock_allowed[DOOR_LEFT] = 1;
    second.prev_suppressed[DOOR_LEFT] = 1;
    Override_Update(&ctx, &second, &out);
    EXPECT_EQ(1, out.active[DOOR_LEFT]);
    EXPECT_EQ(0, out.active[DOOR_RIGHT]);
    EXPECT_EQ(0, ctx.suppressed_valid[DOOR_LEFT]);
}

/** 목적: override 성립 조건 각각의 필요성과 허용 주기 경계(1,000/1,001). 입력: 기록(주기 100)이 있는 상태에서 조건을 하나씩 깨뜨림. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 모두 만족할 때만 active. 추적: UT-063 -> DD-F-020, SRS-L-005. 기법: 경계값 분석, MC/DC */
TEST(OverrideUnit, Establish_EachConditionRequired_AndWindowBoundary)
{
    struct Variant
    {
        uint32_t cycle;
        State_t state;
        int unlock;
        int risk;
        int record_valid;
        int expect_active;
    };
    const Variant variants[] = {
        {101, STATE_NORMAL, 1, 1, 1, 1},   // 모두 만족
        {1100, STATE_NORMAL, 1, 1, 1, 1},  // 경계 1,000주기
        {1101, STATE_NORMAL, 1, 1, 1, 0},  // 경계 1,001주기
        {101, STATE_DEGRADED, 1, 1, 1, 0}, // state
        {101, STATE_NORMAL, 0, 1, 1, 0},   // unlock 요청 없음
        {101, STATE_NORMAL, 1, 0, 1, 0},   // 접근위험 없음
        {101, STATE_NORMAL, 1, 1, 0, 0},   // 억제 기록 없음
    };
    for (const Variant &v : variants)
    {
        Override_t ctx;
        Override_Init(&ctx);
        ctx.suppressed_valid[DOOR_LEFT] = v.record_valid;
        ctx.suppressed_cycle[DOOR_LEFT] = 100;
        OverrideInput_t in = Inp(v.cycle);
        in.state = v.state;
        in.unlock_allowed[DOOR_LEFT] = v.unlock;
        in.risk_true[DOOR_LEFT] = v.risk;
        OverrideStatus_t out{};
        Override_Update(&ctx, &in, &out);
        EXPECT_EQ(v.expect_active, out.active[DOOR_LEFT]) << v.cycle << " " << v.state << v.unlock << v.risk << v.record_valid;
    }
}

/** 목적: override 지속과 종료 조건 (SRS-F-030, A-18). 입력: active 상태에서 접근위험 FALSE / 속도 3.0 이상 / NORMAL에서 lock 적용 / DEGRADED에서 lock / 변화 없음. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 앞의 3개는 종료, 뒤의 2개는 지속. 추적: UT-064 -> DD-F-020, SRS-F-030. 기법: 결정표, MC/DC */
TEST(OverrideUnit, EndConditions_EndOnlyWhenConditionHolds)
{
    struct Variant
    {
        int risk;
        int speed_ge;
        int lock;
        State_t state;
        int expect_active;
    };
    const Variant variants[] = {
        {0, 0, 0, STATE_NORMAL, 0},   // 접근위험 해소
        {1, 1, 0, STATE_NORMAL, 0},   // 속도 3.0 이상
        {1, 0, 1, STATE_NORMAL, 0},   // 적용되는 lock 명령
        {1, 0, 1, STATE_DEGRADED, 1}, // 적용되지 않는 lock 명령
        {1, 0, 0, STATE_NORMAL, 1},   // 변화 없음
    };
    for (const Variant &v : variants)
    {
        Override_t ctx;
        Override_Init(&ctx);
        ctx.active[DOOR_LEFT] = 1;
        ctx.suppressed_valid[DOOR_LEFT] = 1;
        OverrideInput_t in = Inp(500);
        in.state = v.state;
        in.risk_true[DOOR_LEFT] = v.risk;
        in.speed_ge[DOOR_LEFT] = v.speed_ge;
        in.lock_requested[DOOR_LEFT] = v.lock;
        OverrideStatus_t out{};
        Override_Update(&ctx, &in, &out);
        EXPECT_EQ(v.expect_active, out.active[DOOR_LEFT]);
        if (v.expect_active == 0)
        {
            EXPECT_EQ(0, ctx.suppressed_valid[DOOR_LEFT]);
        }
    }
}

/** 목적: 접근위험이 사라지면 억제 기록을 지운다. 입력: 기록이 있고 active가 아닌 상태에서 risk 0. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 기록 해제. 추적: UT-065 -> DD-F-020. 기법: MC/DC */
TEST(OverrideUnit, RiskGone_ClearsSuppressionRecord)
{
    Override_t ctx;
    Override_Init(&ctx);
    ctx.suppressed_valid[DOOR_RIGHT] = 1;
    ctx.suppressed_cycle[DOOR_RIGHT] = 50;
    OverrideInput_t in = Inp(51);
    OverrideStatus_t out{};
    Override_Update(&ctx, &in, &out);
    EXPECT_EQ(0, ctx.suppressed_valid[DOOR_RIGHT]);
}

// ---------------------------------------------------------------- Arbiter

ArbiterInput_t ArbIn(State_t st = STATE_NORMAL)
{
    ArbiterInput_t in{};
    in.state = st;
    in.previous[DOOR_LEFT] = OUTPUT_RELEASE;
    in.previous[DOOR_RIGHT] = OUTPUT_RELEASE;
    return in;
}

void Act(ArbiterInput_t *in, int rank, Output_t effect, ReasonCode_t reason, int door = -1)
{
    for (int d = 0; d < DOOR_COUNT; d++)
    {
        if ((door < 0) || (door == d))
        {
            in->rules[rank].active[d] = 1;
            in->rules[rank].effect[d] = effect;
            in->rules[rank].reason[d] = reason;
        }
    }
}

const ReasonCode_t kRankReason[RULE_COUNT] = {REASON_CRASH_CONFIRMED, REASON_FIRE,      REASON_IGNITION_OFF,
                                              REASON_APPROACH_RISK_LOCK, REASON_ISOFIX_LOCK, REASON_CMD_APPLIED,
                                              REASON_AUTO_LOCK};

/** 목적: 중재 입력 널 거절. 입력: in/out NULL. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: ERR_PARAM. 추적: UT-070 -> DD-F-021. 기법: 오류 추정(MC/DC) */
TEST(ArbiterUnit, Resolve_NullPointers_ReturnParamError)
{
    ArbiterInput_t in = ArbIn();
    Decision_t out{};
    EXPECT_EQ(STATUS_ERR_PARAM, Arbiter_Resolve(nullptr, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Arbiter_Resolve(&in, nullptr));
}

/** 목적: 활성 규칙이 없으면 직전 출력 유지, 이유 NONE (순위 8). 입력: previous {LOCK, RELEASE}. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 같은 출력, priority/reason NONE. 추적: UT-071 -> DD-F-021, SRS-F-017. 기법: 동치 클래스 */
TEST(ArbiterUnit, Resolve_NoActiveRule_HoldsPreviousOutput)
{
    ArbiterInput_t in = ArbIn();
    in.previous[DOOR_LEFT] = OUTPUT_LOCK;
    Decision_t out{};
    EXPECT_EQ(STATUS_OK, Arbiter_Resolve(&in, &out));
    EXPECT_EQ(OUTPUT_LOCK, out.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, out.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_NONE, out.priority_reason[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, out.reason_code[DOOR_RIGHT]);
}

/** 목적: 순위 1~7 각 규칙이 단독 활성일 때의 결과. 입력: 한 순위만 활성(효과 LOCK). 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 해당 효과와 이유가 출력과 priority_reason으로 나온다. 추적: UT-072 -> DD-F-021, SRS-F-028. 기법: 동치 클래스 */
TEST(ArbiterUnit, Resolve_SingleRankActive_AppliesItsEffectAndReason)
{
    for (int rank = 0; rank < RULE_COUNT; rank++)
    {
        ArbiterInput_t in = ArbIn();
        Act(&in, rank, OUTPUT_LOCK, kRankReason[rank]);
        Decision_t out{};
        Arbiter_Resolve(&in, &out);
        EXPECT_EQ(OUTPUT_LOCK, out.output[DOOR_LEFT]) << rank;
        EXPECT_EQ(kRankReason[rank], out.priority_reason[DOOR_LEFT]) << rank;
        EXPECT_EQ(kRankReason[rank], out.reason_code[DOOR_RIGHT]) << rank;
    }
}

/** 목적: 두 규칙이 서로 다른 출력을 요구하면 높은 순위가 이긴다 (SRS-F-017). 입력: 순위 i<j 21쌍 각각, i는 LOCK, j는 RELEASE (그리고 반대). 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 순위 i의 효과와 이유. 추적: UT-073 -> DD-F-021, SRS-F-017, SRS-F-028. 기법: 쌍 조합(전수 21x2) */
TEST(ArbiterUnit, Resolve_AllRankPairs_HigherRankWins)
{
    int pairs = 0;
    for (int i = 0; i < RULE_COUNT; i++)
    {
        for (int j = i + 1; j < RULE_COUNT; j++)
        {
            for (int flip = 0; flip < 2; flip++)
            {
                Output_t hi = (flip == 0) ? OUTPUT_LOCK : OUTPUT_RELEASE;
                Output_t lo = (flip == 0) ? OUTPUT_RELEASE : OUTPUT_LOCK;
                ArbiterInput_t in = ArbIn();
                Act(&in, i, hi, kRankReason[i]);
                Act(&in, j, lo, kRankReason[j]);
                Decision_t out{};
                Arbiter_Resolve(&in, &out);
                EXPECT_EQ(hi, out.output[DOOR_LEFT]) << i << "," << j;
                EXPECT_EQ(kRankReason[i], out.priority_reason[DOOR_RIGHT]) << i << "," << j;
            }
            pairs++;
        }
    }
    EXPECT_EQ(21, pairs);
}

/** 목적: 도어별 독립 중재. 입력: 좌는 순위 2만, 우는 순위 5만 활성. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 좌/우가 각자의 규칙 결과를 따른다. 추적: UT-074 -> DD-F-021. 기법: 동치 클래스 */
TEST(ArbiterUnit, Resolve_DoorsIndependent)
{
    ArbiterInput_t in = ArbIn();
    Act(&in, 1, OUTPUT_LOCK, REASON_FIRE, DOOR_LEFT);
    Act(&in, 4, OUTPUT_LOCK, REASON_ISOFIX_LOCK, DOOR_RIGHT);
    Decision_t out{};
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(REASON_FIRE, out.priority_reason[DOOR_LEFT]);
    EXPECT_EQ(REASON_ISOFIX_LOCK, out.priority_reason[DOOR_RIGHT]);
}

/** 목적: OFF에서는 접근위험 이하 순위(4~7)를 적용하지 않고 순위 1~3은 적용한다 (SRS-F-031). 입력: state OFF, 순위 4~7만 활성 / 순위 3 포함 / 순위 1 포함. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 4~7만이면 직전 출력 유지, 3 또는 1이 있으면 그 규칙. 추적: UT-075 -> DD-F-022, SRS-F-031. 기법: 결정표, MC/DC */
TEST(ArbiterUnit, Resolve_StateOff_GatesRanksFourToSeven)
{
    ArbiterInput_t in = ArbIn(STATE_OFF);
    in.previous[DOOR_LEFT] = OUTPUT_RELEASE;
    for (int rank = 3; rank < RULE_COUNT; rank++)
    {
        Act(&in, rank, OUTPUT_LOCK, kRankReason[rank]);
    }
    Decision_t out{};
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(OUTPUT_RELEASE, out.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, out.priority_reason[DOOR_LEFT]);

    Act(&in, 2, OUTPUT_RELEASE, REASON_IGNITION_OFF);
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(REASON_IGNITION_OFF, out.priority_reason[DOOR_LEFT]);

    Act(&in, 0, OUTPUT_RELEASE, REASON_CRASH_CONFIRMED);
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(REASON_CRASH_CONFIRMED, out.priority_reason[DOOR_LEFT]);
}

/** 목적: DEGRADED, FAULT에서는 운전자 명령(순위 6)만 제외하고 나머지는 적용한다 (SSR-010, 011, 013, 016). 입력: 각 state에서 순위 6 단독 / 순위 4, 5, 7 단독. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 6은 직전 출력 유지+상태 이유, 4/5/7은 적용. 추적: UT-076 -> DD-F-022, SSR-010, SSR-011, SSR-016. 기법: 결정표, MC/DC */
TEST(ArbiterUnit, Resolve_DegradedAndFault_GateOnlyDriverCommand)
{
    const State_t states[] = {STATE_DEGRADED, STATE_FAULT};
    const ReasonCode_t expect_reason[] = {REASON_DEGRADED_STALE, REASON_SENSOR_FAULT};
    for (int k = 0; k < 2; k++)
    {
        ArbiterInput_t in = ArbIn(states[k]);
        Act(&in, 5, OUTPUT_LOCK, REASON_CMD_APPLIED);
        Decision_t out{};
        Arbiter_Resolve(&in, &out);
        EXPECT_EQ(OUTPUT_RELEASE, out.output[DOOR_LEFT]) << k;
        EXPECT_EQ(REASON_NONE, out.priority_reason[DOOR_LEFT]) << k;
        EXPECT_EQ(expect_reason[k], out.reason_code[DOOR_LEFT]) << k;

        const int applied[] = {0, 1, 2, 3, 4, 6};
        for (int rank : applied)
        {
            ArbiterInput_t one = ArbIn(states[k]);
            Act(&one, rank, OUTPUT_LOCK, kRankReason[rank]);
            Arbiter_Resolve(&one, &out);
            EXPECT_EQ(OUTPUT_LOCK, out.output[DOOR_LEFT]) << k << " " << rank;
            EXPECT_EQ(kRankReason[rank], out.reason_code[DOOR_LEFT]) << k << " " << rank;
        }
    }
}

/** 목적: override 지속은 순위 6 효과(RELEASE, OVERRIDE)로 반영되고 더 높은 순위에는 지므로 ISOFIX가 우선한다. 입력: override active + 순위 6 비활성 / 순위 6 활성(LOCK) / ISOFIX 활성 / DEGRADED / override 없음. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: RELEASE+OVERRIDE / OVERRIDE / ISOFIX LOCK / 직전 유지 / 직전 유지. 추적: UT-077 -> DD-F-023, SRS-F-029, SRS-F-030. 기법: 결정표, MC/DC */
TEST(ArbiterUnit, Resolve_Override_AppliesAtDriverRankOnly)
{
    Decision_t out{};
    ArbiterInput_t in = ArbIn();
    in.previous[DOOR_LEFT] = OUTPUT_LOCK;
    in.override_status.active[DOOR_LEFT] = 1;
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(OUTPUT_RELEASE, out.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, out.priority_reason[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, out.reason_code[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, out.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_NONE, out.priority_reason[DOOR_RIGHT]);

    Act(&in, 5, OUTPUT_LOCK, REASON_CMD_APPLIED);
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(OUTPUT_RELEASE, out.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, out.priority_reason[DOOR_LEFT]);

    Act(&in, 4, OUTPUT_LOCK, REASON_ISOFIX_LOCK, DOOR_LEFT);
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(OUTPUT_LOCK, out.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_ISOFIX_LOCK, out.priority_reason[DOOR_LEFT]);

    ArbiterInput_t degraded = ArbIn(STATE_DEGRADED);
    degraded.previous[DOOR_LEFT] = OUTPUT_LOCK;
    degraded.override_status.active[DOOR_LEFT] = 1;
    Arbiter_Resolve(&degraded, &out);
    EXPECT_EQ(OUTPUT_LOCK, out.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_DEGRADED_STALE, out.reason_code[DOOR_LEFT]);

    ArbiterInput_t none = ArbIn();
    none.previous[DOOR_LEFT] = OUTPUT_LOCK;
    Arbiter_Resolve(&none, &out);
    EXPECT_EQ(OUTPUT_LOCK, out.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, out.reason_code[DOOR_LEFT]);
}

/** 목적: 선택된 규칙이 없을 때 정보성 이유를 가장 높은 순위부터 고른다. 입력: info 순위 1 / 순위 6 / 둘 다 / OFF에서 제외된 순위 6 / 전부 NONE. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 각각 CRASH_PENDING / CMD_REJECTED_SPEED / CMD_REJECTED_SPEED(거절 코드가 우선) / NONE(제외된 순위) / NONE. 추적: UT-078 -> DD-F-024, SSR-003, SRS-F-003. 기법: 결정표, MC/DC */
TEST(ArbiterUnit, Fallback_InfoReasons_HighestRankFirst)
{
    Decision_t out{};
    ArbiterInput_t in = ArbIn();
    in.rules[0].info_reason[DOOR_LEFT] = REASON_CRASH_PENDING;
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(REASON_CRASH_PENDING, out.reason_code[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, out.priority_reason[DOOR_LEFT]);

    in = ArbIn();
    in.rules[5].info_reason[DOOR_LEFT] = REASON_CMD_REJECTED_SPEED;
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(REASON_CMD_REJECTED_SPEED, out.reason_code[DOOR_LEFT]);

    in.rules[0].info_reason[DOOR_LEFT] = REASON_CRASH_PENDING;
    Arbiter_Resolve(&in, &out);
    EXPECT_EQ(REASON_CMD_REJECTED_SPEED, out.reason_code[DOOR_LEFT]); // 속도 거절 코드는 항상 덮어쓴다 (SRS-F-003)

    ArbiterInput_t off = ArbIn(STATE_OFF);
    off.rules[5].info_reason[DOOR_LEFT] = REASON_INPUT_INVALID;
    Arbiter_Resolve(&off, &out);
    EXPECT_EQ(REASON_NONE, out.reason_code[DOOR_LEFT]);

    ArbiterInput_t none = ArbIn();
    Arbiter_Resolve(&none, &out);
    EXPECT_EQ(REASON_NONE, out.reason_code[DOOR_LEFT]);
}

/** 목적: 운전자 명령 관련 이유 덮어쓰기 (SRS-F-032). 입력: state NORMAL에서 순위 6 info가 CMD_REJECTED_SPEED / INPUT_INVALID / 다른 정보성 이유 / 없음, 그리고 OFF, DEGRADED, FAULT에서 같은 info. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: NORMAL의 두 거절 이유만 reason_code를 덮어쓰고 priority_reason은 결정 규칙의 값을 유지, 다른 state에서는 덮어쓰지 않는다. 추적: UT-079 -> DD-F-021, DD-F-029, SRS-F-032, SSR-007. 기법: 결정표, MC/DC */
TEST(ArbiterUnit, Resolve_OverlayReason_OnlyInNormalState)
{
    Decision_t out{};
    const ReasonCode_t overlays[] = {REASON_CMD_REJECTED_SPEED, REASON_INPUT_INVALID};
    for (ReasonCode_t info : overlays)
    {
        ArbiterInput_t in = ArbIn();
        Act(&in, 6, OUTPUT_LOCK, REASON_AUTO_LOCK);
        in.rules[5].info_reason[DOOR_LEFT] = info;
        Arbiter_Resolve(&in, &out);
        EXPECT_EQ(REASON_AUTO_LOCK, out.priority_reason[DOOR_LEFT]);
        EXPECT_EQ(info, out.reason_code[DOOR_LEFT]);
        EXPECT_EQ(REASON_AUTO_LOCK, out.reason_code[DOOR_RIGHT]);
    }

    ArbiterInput_t other = ArbIn();
    Act(&other, 6, OUTPUT_LOCK, REASON_AUTO_LOCK);
    other.rules[5].info_reason[DOOR_LEFT] = REASON_CRASH_PENDING;
    Arbiter_Resolve(&other, &out);
    EXPECT_EQ(REASON_AUTO_LOCK, out.reason_code[DOOR_LEFT]);

    const State_t gated[] = {STATE_DEGRADED, STATE_FAULT};
    for (State_t st : gated)
    {
        ArbiterInput_t in = ArbIn(st);
        Act(&in, 6, OUTPUT_LOCK, REASON_AUTO_LOCK);
        in.rules[5].info_reason[DOOR_LEFT] = REASON_INPUT_INVALID;
        Arbiter_Resolve(&in, &out);
        EXPECT_EQ(REASON_AUTO_LOCK, out.reason_code[DOOR_LEFT]) << st;
    }
    ArbiterInput_t off = ArbIn(STATE_OFF);
    Act(&off, 2, OUTPUT_RELEASE, REASON_IGNITION_OFF);
    off.rules[5].info_reason[DOOR_LEFT] = REASON_INPUT_INVALID;
    Arbiter_Resolve(&off, &out);
    EXPECT_EQ(REASON_IGNITION_OFF, out.reason_code[DOOR_LEFT]);
}

/** 목적: 상태 이유 우선 (A-25). 입력: 형식 오류 명령 info + FAULT / DEGRADED에서 규칙 없음. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: reason_code가 INPUT_INVALID가 아니라 SENSOR_FAULT / DEGRADED_STALE. 추적: UT-080A -> DD-F-024, DD-F-029, SSR-009, SSR-012. 기법: 시나리오 */
TEST(ArbiterUnit, Resolve_StateReasonBeatsInvalidCommand)
{
    Decision_t out{};
    ArbiterInput_t fault = ArbIn(STATE_FAULT);
    fault.rules[5].info_reason[DOOR_LEFT] = REASON_INPUT_INVALID;
    Arbiter_Resolve(&fault, &out);
    EXPECT_EQ(REASON_SENSOR_FAULT, out.reason_code[DOOR_LEFT]);
    ArbiterInput_t degraded = ArbIn(STATE_DEGRADED);
    degraded.rules[5].info_reason[DOOR_LEFT] = REASON_INPUT_INVALID;
    Arbiter_Resolve(&degraded, &out);
    EXPECT_EQ(REASON_DEGRADED_STALE, out.reason_code[DOOR_LEFT]);
}

} // namespace
