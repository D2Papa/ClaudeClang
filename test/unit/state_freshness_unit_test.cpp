/**
 * @file    state_freshness_unit_test.cpp
 * @brief   FreshnessMonitor(DD-M-002)와 StateManager(DD-M-003) 단위 테스트.
 * @trace   DD-F-006~009, SSR-008, SSR-009, SSR-012, SSR-014, SSR-015, SRS-F-016, SRS-F-027
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

Snapshot_t SnapAt(double ts, double source_ts)
{
    Snapshot_t s = NormalSnapshot();
    s.timestamp_s = ts;
    s.source_timestamp.value = source_ts;
    return s;
}

/** 목적: 초기화 널 거절과 정상 초기화. 입력: NULL / 유효 ctx. 기대: ERR_PARAM / OK, initialized 1. 추적: UT-020 -> DD-F-006. 기법: 오류 추정 */
TEST(FreshnessUnit, Init_NullAndValid_ReturnsExpected)
{
    Freshness_t ctx{};
    EXPECT_EQ(STATUS_ERR_PARAM, Freshness_Init(nullptr));
    EXPECT_EQ(STATUS_OK, Freshness_Init(&ctx));
    EXPECT_EQ(1, ctx.initialized);
}

/** 목적: 적용 함수의 인자 오류. 입력: ctx NULL / snap NULL. 기대: ERR_PARAM. 추적: UT-021 -> DD-F-007. 기법: 오류 추정(MC/DC) */
TEST(FreshnessUnit, Apply_NullPointers_ReturnsParamError)
{
    Freshness_t ctx{};
    Freshness_Init(&ctx);
    Snapshot_t s = NormalSnapshot();
    EXPECT_EQ(STATUS_ERR_PARAM, Freshness_Apply(nullptr, &s));
    EXPECT_EQ(STATUS_ERR_PARAM, Freshness_Apply(&ctx, nullptr));
}

/** 목적: 초기화하지 않은 컨텍스트 거절. 입력: 0으로 채운 ctx. 기대: ERR_STATE. 추적: UT-022 -> DD-F-007. 기법: 오류 추정 */
TEST(FreshnessUnit, Apply_Uninitialized_ReturnsStateError)
{
    Freshness_t ctx{};
    Snapshot_t s = NormalSnapshot();
    EXPECT_EQ(STATUS_ERR_STATE, Freshness_Apply(&ctx, &s));
}

/** 목적: 시각 역행 거절과 동일 시각 허용. 입력: 1.0 후 0.99 / 1.0 후 1.0. 기대: ERR_STATE / OK. 추적: UT-023 -> DD-F-007. 기법: 경계값 분석 */
TEST(FreshnessUnit, Apply_TimestampMonotonic_BackwardRejected)
{
    Freshness_t ctx{};
    Freshness_Init(&ctx);
    Snapshot_t a = SnapAt(1.0, 1.0);
    EXPECT_EQ(STATUS_OK, Freshness_Apply(&ctx, &a));
    Snapshot_t same = SnapAt(1.0, 1.0);
    EXPECT_EQ(STATUS_OK, Freshness_Apply(&ctx, &same));
    Snapshot_t back = SnapAt(0.99, 0.99);
    EXPECT_EQ(STATUS_ERR_STATE, Freshness_Apply(&ctx, &back));
}

/** 목적: 200 ms 신선도 경계 (SSR-008). 입력: 차이 0.2(경계), 0.25(초과), -0.05(미래 시각). 기대: 0.2는 유지, 나머지는 차속과 원천 시각 INVALID. 추적: UT-024 -> DD-F-007, SSR-008. 기법: 경계값 분석 */
TEST(FreshnessUnit, Apply_SpeedAge_StaleOnlyBeyondLimit)
{
    Freshness_t ctx{};
    Freshness_Init(&ctx);
    Snapshot_t edge = SnapAt(0.2, 0.0);
    Freshness_Apply(&ctx, &edge);
    EXPECT_EQ(VALIDITY_VALID, edge.speed.validity);
    EXPECT_EQ(VALIDITY_VALID, edge.source_timestamp.validity);

    Snapshot_t old = SnapAt(0.5, 0.25);
    Freshness_Apply(&ctx, &old);
    EXPECT_EQ(VALIDITY_INVALID, old.speed.validity);
    EXPECT_EQ(VALIDITY_INVALID, old.source_timestamp.validity);

    Snapshot_t future = SnapAt(1.0, 1.05);
    Freshness_Apply(&ctx, &future);
    EXPECT_EQ(VALIDITY_INVALID, future.speed.validity);
}

/** 목적: 차속 또는 원천 시각이 이미 무효이면 신선도 판정 대상이 아니다. 입력: 한쪽만 INVALID이고 시간 차이가 큼. 기대: 다른 쪽 유효성은 바뀌지 않음. 추적: UT-025 -> DD-F-007. 기법: 조건 독립성(MC/DC) */
TEST(FreshnessUnit, Apply_OneSideAlreadyInvalid_OtherUnchanged)
{
    Freshness_t ctx{};
    Freshness_Init(&ctx);
    Snapshot_t s = SnapAt(5.0, 0.0);
    s.speed.validity = VALIDITY_INVALID;
    Freshness_Apply(&ctx, &s);
    EXPECT_EQ(VALIDITY_VALID, s.source_timestamp.validity);

    Snapshot_t t = SnapAt(6.0, 0.0);
    t.source_timestamp.validity = VALIDITY_INVALID;
    Freshness_Apply(&ctx, &t);
    EXPECT_EQ(VALIDITY_VALID, t.speed.validity);
}

/** 목적: state 결정 인자 오류. 입력: snap/state/warning 각각 NULL. 기대: ERR_PARAM. 추적: UT-030 -> DD-F-008. 기법: 오류 추정(MC/DC) */
TEST(StateUnit, Determine_NullPointers_ReturnsParamError)
{
    Snapshot_t s = NormalSnapshot();
    State_t st;
    int w;
    EXPECT_EQ(STATUS_ERR_PARAM, State_Determine(nullptr, &st, &w));
    EXPECT_EQ(STATUS_ERR_PARAM, State_Determine(&s, nullptr, &w));
    EXPECT_EQ(STATUS_ERR_PARAM, State_Determine(&s, &st, nullptr));
}

/** 목적: 모든 입력 유효이면 NORMAL, 경고 0. 입력: NormalSnapshot. 기대: STATE_NORMAL, warning 0. 추적: UT-031 -> DD-F-008, SSR-015. 기법: 동치 클래스 */
TEST(StateUnit, Determine_AllValid_Normal)
{
    Snapshot_t s = NormalSnapshot();
    State_t st = STATE_OFF;
    int w = 1;
    EXPECT_EQ(STATUS_OK, State_Determine(&s, &st, &w));
    EXPECT_EQ(STATE_NORMAL, st);
    EXPECT_EQ(0, w);
}

/** 목적: 상태 우선순위 OFF > FAULT > DEGRADED (SSR-015). 입력: ignition/sensor_fault/stale 8가지 조합. 기대: 조합별 기대 state와 warning. 추적: UT-032 -> DD-F-008, SSR-015, SSR-012. 기법: 결정표(전수 2^3) */
TEST(StateUnit, Determine_PriorityTable_MatchesSsr015)
{
    for (int mask = 0; mask < 8; mask++)
    {
        int off = mask & 1;
        int fault = (mask >> 1) & 1;
        int stale = (mask >> 2) & 1;
        Snapshot_t s = NormalSnapshot();
        s.ignition = F(off ? 0 : 1);
        s.sensor_fault = F(fault);
        if (stale == 1)
        {
            s.speed.validity = VALIDITY_INVALID;
        }
        State_t expect = STATE_NORMAL;
        if (off == 1)
        {
            expect = STATE_OFF;
        }
        else if (fault == 1)
        {
            expect = STATE_FAULT;
        }
        else if (stale == 1)
        {
            expect = STATE_DEGRADED;
        }
        State_t st = STATE_NORMAL;
        int w = -1;
        EXPECT_EQ(STATUS_OK, State_Determine(&s, &st, &w));
        EXPECT_EQ(expect, st) << mask;
        EXPECT_EQ(fault, w) << mask;
    }
}

/** 목적: 필수 입력 12개 각각이 무효이면 DEGRADED, gear만 무효이면 NORMAL. 입력: 필드별 INVALID. 기대: DEGRADED / NORMAL. 추적: UT-033 -> DD-F-009, SSR-009, SSR-007. 기법: 동치 클래스 */
TEST(StateUnit, Determine_EachRequiredInputInvalid_Degraded)
{
    for (int field = 0; field < 13; field++)
    {
        Snapshot_t s = NormalSnapshot();
        switch (field)
        {
        case 0: s.speed.validity = VALIDITY_INVALID; break;
        case 1: s.source_timestamp.validity = VALIDITY_INVALID; break;
        case 2: s.crash.validity = VALIDITY_INVALID; break;
        case 3: s.approach[DOOR_LEFT].validity = VALIDITY_INVALID; break;
        case 4: s.approach[DOOR_RIGHT].validity = VALIDITY_INVALID; break;
        case 5: s.fire.validity = VALIDITY_INVALID; break;
        case 6: s.overtemp.validity = VALIDITY_INVALID; break;
        case 7: s.adult.validity = VALIDITY_INVALID; break;
        case 8: s.isofix[DOOR_LEFT].validity = VALIDITY_INVALID; break;
        case 9: s.isofix[DOOR_RIGHT].validity = VALIDITY_INVALID; break;
        case 10: s.ignition.validity = VALIDITY_INVALID; break;
        case 11: s.sensor_fault.validity = VALIDITY_INVALID; break;
        default: s.gear.validity = VALIDITY_INVALID; break;
        }
        State_t st = STATE_OFF;
        int w = 0;
        State_Determine(&s, &st, &w);
        EXPECT_EQ(field == 12 ? STATE_NORMAL : STATE_DEGRADED, st) << field;
    }
}

/** 목적: 무효한 ignition(값 0)은 OFF가 아니라 DEGRADED, 무효한 sensor_fault는 FAULT가 아니다. 입력: ignition INVALID / sensor_fault INVALID. 기대: DEGRADED, warning 0. 추적: UT-034 -> DD-F-008. 기법: 조건 독립성(MC/DC) */
TEST(StateUnit, Determine_InvalidFlags_NotTreatedAsValueZeroOrOne)
{
    Snapshot_t s = NormalSnapshot();
    s.ignition = F(0, VALIDITY_INVALID);
    State_t st = STATE_NORMAL;
    int w = 0;
    State_Determine(&s, &st, &w);
    EXPECT_EQ(STATE_DEGRADED, st);

    s = NormalSnapshot();
    s.sensor_fault = F(1, VALIDITY_INVALID);
    State_Determine(&s, &st, &w);
    EXPECT_EQ(STATE_DEGRADED, st);
    EXPECT_EQ(0, w);
}

} // namespace
