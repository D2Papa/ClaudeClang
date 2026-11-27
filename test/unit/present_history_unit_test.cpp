/**
 * @file    present_history_unit_test.cpp
 * @brief   StatusPresenter(DD-M-014)와 EventRecorder(DD-M-015) 단위 테스트.
 * @trace   DD-F-040~050, SRS-F-018~026, SRS-L-006, SRS-Q-002
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

#include <cstring>
#include <string>

extern "C" {
#include "history/event_recorder.h"
#include "present/status_presenter.h"
}

namespace
{

CycleResult_t Result(Output_t left, Output_t right, State_t state, ReasonCode_t reason)
{
    CycleResult_t r{};
    r.output[DOOR_LEFT] = left;
    r.output[DOOR_RIGHT] = right;
    r.state = state;
    r.priority_reason_summary = reason;
    r.reason_code_summary = reason;
    r.input_validity = 0x1FFFU;
    return r;
}

/** 목적: Build 널 거절. 입력: res/out NULL. 기대: ERR_PARAM. 추적: UT-100 -> DD-F-040. 기법: 오류 추정(MC/DC) */
TEST(PresenterUnit, Build_NullPointers_ReturnParamError)
{
    CycleResult_t r = Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, REASON_NONE);
    PresentedStatus_t out{};
    EXPECT_EQ(STATUS_ERR_PARAM, Presenter_Build(nullptr, &out));
    EXPECT_EQ(STATUS_ERR_PARAM, Presenter_Build(&r, nullptr));
}

/** 목적: 결과가 SRS 6.2 열거형 이름으로 표시된다 (SRS-F-019, F-022). 입력: LOCK/RELEASE, FAULT, SENSOR_FAULT, 경고 1. 기대: 이름과 "W-001". 추적: UT-101 -> DD-F-040, SRS-F-019. 기법: 동치 클래스 */
TEST(PresenterUnit, Build_Values_MappedToNames)
{
    CycleResult_t r = Result(OUTPUT_LOCK, OUTPUT_RELEASE, STATE_FAULT, REASON_SENSOR_FAULT);
    r.warning_code = 1;
    PresentedStatus_t p{};
    ASSERT_EQ(STATUS_OK, Presenter_Build(&r, &p));
    EXPECT_STREQ("LOCK", p.lock_left);
    EXPECT_STREQ("RELEASE", p.lock_right);
    EXPECT_STREQ("FAULT", p.state);
    EXPECT_STREQ("SENSOR_FAULT", p.priority_reason);
    EXPECT_STREQ("SENSOR_FAULT", p.reason_code);
    EXPECT_STREQ("W-001", p.warning_code);
    r.warning_code = 0;
    Presenter_Build(&r, &p);
    EXPECT_STREQ("NONE", p.warning_code);
}

/** 목적: 모든 열거 값의 이름 (17개 이유, 4개 state, 2개 출력). 입력: 값 전수. 기대: 이름 표와 일치하는 문자열. 추적: UT-102 -> DD-F-041. 기법: 전수 */
TEST(PresenterUnit, Build_AllEnumValues_HaveNames)
{
    const char *reasons[] = {"NONE", "CMD_APPLIED", "CMD_REJECTED_SPEED", "INPUT_INVALID", "CRASH_CONFIRMED",
                             "CRASH_PENDING", "FIRE", "OVERTEMP", "ADULT_PRESENT", "IGNITION_OFF", "SENSOR_FAULT",
                             "DEGRADED_STALE", "APPROACH_RISK_LOCK", "APPROACH_RISK_SUPPRESSED", "OVERRIDE",
                             "ISOFIX_LOCK", "AUTO_LOCK"};
    for (int i = 0; i < 17; i++)
    {
        CycleResult_t r = Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, static_cast<ReasonCode_t>(i));
        PresentedStatus_t p{};
        Presenter_Build(&r, &p);
        EXPECT_STREQ(reasons[i], p.reason_code) << i;
    }
    const char *states[] = {"OFF", "NORMAL", "DEGRADED", "FAULT"};
    for (int i = 0; i < 4; i++)
    {
        CycleResult_t r = Result(OUTPUT_RELEASE, OUTPUT_RELEASE, static_cast<State_t>(i), REASON_NONE);
        PresentedStatus_t p{};
        Presenter_Build(&r, &p);
        EXPECT_STREQ(states[i], p.state);
    }
}

/** 목적: 범위 밖 열거 값은 "UNKNOWN". 입력: 음수와 상한 초과 값. 기대: UNKNOWN. 추적: UT-103 -> DD-F-041. 기법: 경계값 분석, MC/DC */
TEST(PresenterUnit, Build_OutOfRangeEnums_ShowUnknown)
{
    PresentedStatus_t p{};
    CycleResult_t low = Result(static_cast<Output_t>(-1), static_cast<Output_t>(2), static_cast<State_t>(-1), static_cast<ReasonCode_t>(-1));
    Presenter_Build(&low, &p);
    EXPECT_STREQ("UNKNOWN", p.lock_left);
    EXPECT_STREQ("UNKNOWN", p.lock_right);
    EXPECT_STREQ("UNKNOWN", p.state);
    EXPECT_STREQ("UNKNOWN", p.reason_code);
    CycleResult_t high = Result(OUTPUT_LOCK, OUTPUT_LOCK, static_cast<State_t>(4), static_cast<ReasonCode_t>(17));
    Presenter_Build(&high, &p);
    EXPECT_STREQ("UNKNOWN", p.state);
    EXPECT_STREQ("UNKNOWN", p.priority_reason);
    EXPECT_STREQ("LOCK", p.lock_left);
}

/** 목적: 입력 유효성 비트 13개가 항목별 VALID/INVALID로 표시된다. 입력: 마스크 0, 0x1FFF, 비트 하나씩. 기대: 해당 항목만 VALID/INVALID. 추적: UT-104 -> DD-F-040, SRS-F-019. 기법: 전수 */
TEST(PresenterUnit, Build_ValidityBits_MappedPerItem)
{
    CycleResult_t r = Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, REASON_NONE);
    PresentedStatus_t p{};
    r.input_validity = 0U;
    Presenter_Build(&r, &p);
    for (int i = 0; i < VALIDITY_ITEM_COUNT; i++)
    {
        EXPECT_STREQ("INVALID", p.validity[i]);
    }
    for (int bit = 0; bit < VALIDITY_ITEM_COUNT; bit++)
    {
        r.input_validity = 1U << bit;
        Presenter_Build(&r, &p);
        for (int i = 0; i < VALIDITY_ITEM_COUNT; i++)
        {
            EXPECT_STREQ(i == bit ? "VALID" : "INVALID", p.validity[i]) << bit << "," << i;
        }
    }
}

/** 목적: Serialize 인자 오류. 입력: http_status NULL / st NULL / buf NULL / cap 0. 기대: ERR_PARAM, http_status가 널이 아니면 500. 추적: UT-105 -> DD-F-042, SRS-F-023. 기법: 오류 추정(MC/DC) */
TEST(PresenterUnit, Serialize_InvalidArguments_ReturnParamError)
{
    CycleResult_t r = Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, REASON_NONE);
    PresentedStatus_t p{};
    Presenter_Build(&r, &p);
    char buf[512];
    int http = 0;
    EXPECT_EQ(STATUS_ERR_PARAM, Presenter_Serialize(&p, buf, sizeof(buf), nullptr));
    EXPECT_EQ(STATUS_ERR_PARAM, Presenter_Serialize(nullptr, buf, sizeof(buf), &http));
    EXPECT_EQ(500, http);
    http = 0;
    EXPECT_EQ(STATUS_ERR_PARAM, Presenter_Serialize(&p, nullptr, sizeof(buf), &http));
    EXPECT_EQ(500, http);
    http = 0;
    EXPECT_EQ(STATUS_ERR_PARAM, Presenter_Serialize(&p, buf, 0, &http));
    EXPECT_EQ(500, http);
}

/** 목적: 직렬화 결과 JSON의 정확한 내용 (SRS-F-018, 019). 입력: LOCK/RELEASE, NORMAL, AUTO_LOCK, 모든 항목 VALID. 기대: 고정 순서 JSON, HTTP 200. 추적: UT-106 -> DD-F-043, SRS-F-018. 기법: 동치 클래스 */
TEST(PresenterUnit, Serialize_Normal_ProducesExpectedJson)
{
    CycleResult_t r = Result(OUTPUT_LOCK, OUTPUT_RELEASE, STATE_NORMAL, REASON_AUTO_LOCK);
    PresentedStatus_t p{};
    Presenter_Build(&r, &p);
    char buf[1024];
    int http = 0;
    ASSERT_EQ(STATUS_OK, Presenter_Serialize(&p, buf, sizeof(buf), &http));
    EXPECT_EQ(200, http);
    EXPECT_STREQ("{\"lock_left\":\"LOCK\",\"lock_right\":\"RELEASE\",\"state\":\"NORMAL\",\"priority_reason\":\"AUTO_LOCK\","
                 "\"reason_code\":\"AUTO_LOCK\",\"warning_code\":\"NONE\",\"input_validity\":{\"speed\":\"VALID\",\"gear\":\"VALID\","
                 "\"source_timestamp\":\"VALID\",\"crash\":\"VALID\",\"approach_left\":\"VALID\",\"approach_right\":\"VALID\","
                 "\"fire\":\"VALID\",\"overtemp\":\"VALID\",\"adult\":\"VALID\",\"isofix_left\":\"VALID\",\"isofix_right\":\"VALID\","
                 "\"ignition\":\"VALID\",\"sensor_fault\":\"VALID\"}}",
                 buf);
}

/** 목적: 버퍼 용량 경계 (SRS-F-023). 입력: 필요 길이+1 용량(성공)부터 1까지의 모든 용량. 기대: 성공은 200, 그보다 작은 모든 용량은 500이고 buf는 빈 문자열. 추적: UT-107 -> DD-F-042, DD-F-044, SRS-F-023. 기법: 경계값 분석 전수 */
TEST(PresenterUnit, Serialize_CapacityBoundary_FailsBelowRequired)
{
    CycleResult_t r = Result(OUTPUT_LOCK, OUTPUT_RELEASE, STATE_NORMAL, REASON_AUTO_LOCK);
    PresentedStatus_t p{};
    Presenter_Build(&r, &p);
    char big[1024];
    int http = 0;
    Presenter_Serialize(&p, big, sizeof(big), &http);
    size_t needed = std::strlen(big) + 1U;
    char small[1024];
    for (size_t cap = 1; cap <= needed; cap++)
    {
        std::memset(small, 'x', sizeof(small));
        ASSERT_EQ(STATUS_OK, Presenter_Serialize(&p, small, cap, &http));
        if (cap < needed)
        {
            EXPECT_EQ(500, http) << cap;
            EXPECT_EQ('\0', small[0]) << cap;
        }
        else
        {
            EXPECT_EQ(200, http);
            EXPECT_STREQ(big, small);
        }
    }
}

// ----------------------------------------------------------------- EventRecorder

/** 목적: Init/Record/Query 널 거절과 미초기화 (SRS-F-026). 입력: NULL 조합, 0으로 채운 ctx. 기대: ERR_PARAM / ERR_STATE. 추적: UT-110 -> DD-F-046, 047, 050. 기법: 오류 추정(MC/DC) */
TEST(EventUnit, NullAndUninitialized_AreRejected)
{
    Event_t ev{};
    Event_t raw{};
    CycleResult_t r = Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, REASON_NONE);
    EventEntry_t out[HISTORY_CAPACITY];
    size_t count = 0;
    EXPECT_EQ(STATUS_ERR_PARAM, Event_Init(nullptr));
    ASSERT_EQ(STATUS_OK, Event_Init(&ev));
    EXPECT_EQ(STATUS_ERR_PARAM, Event_Record(nullptr, &r, 0.0));
    EXPECT_EQ(STATUS_ERR_PARAM, Event_Record(&ev, nullptr, 0.0));
    EXPECT_EQ(STATUS_ERR_STATE, Event_Record(&raw, &r, 0.0));
    EXPECT_EQ(STATUS_ERR_PARAM, Event_Query(nullptr, out, HISTORY_CAPACITY, &count));
    EXPECT_EQ(STATUS_ERR_PARAM, Event_Query(&ev, nullptr, HISTORY_CAPACITY, &count));
    EXPECT_EQ(STATUS_ERR_PARAM, Event_Query(&ev, out, HISTORY_CAPACITY, nullptr));
    EXPECT_EQ(STATUS_ERR_STATE, Event_Query(&raw, out, HISTORY_CAPACITY, &count));
}

/** 목적: 변화가 없으면 기록하지 않는다 (SRS-F-025). 입력: 초기 상태와 같은 결과 100주기. 기대: 이벤트 0건. 추적: UT-111 -> DD-F-048, SRS-F-025. 기법: 동치 클래스 */
TEST(EventUnit, NoChange_NoEvents)
{
    Event_t ev;
    Event_Init(&ev);
    CycleResult_t r = Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, REASON_NONE);
    for (int i = 0; i < 100; i++)
    {
        ASSERT_EQ(STATUS_OK, Event_Record(&ev, &r, i * 0.01));
    }
    EventEntry_t out[HISTORY_CAPACITY];
    size_t count = 99;
    ASSERT_EQ(STATUS_OK, Event_Query(&ev, out, HISTORY_CAPACITY, &count));
    EXPECT_EQ(0U, count);
}

/** 목적: 변화 조건 네 가지 각각이 단독으로 이벤트를 만든다 (좌 출력, 우 출력, state, reason_code). 입력: 한 항목만 바꾼 결과. 기대: 이벤트 1건, 필드 값 일치. 추적: UT-112 -> DD-F-048, DD-F-049, SRS-F-024. 기법: 조건 독립성(MC/DC) */
TEST(EventUnit, EachChangeKind_CreatesOneEventWithFields)
{
    const CycleResult_t cases[] = {
        Result(OUTPUT_LOCK, OUTPUT_RELEASE, STATE_NORMAL, REASON_NONE),
        Result(OUTPUT_RELEASE, OUTPUT_LOCK, STATE_NORMAL, REASON_NONE),
        Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_FAULT, REASON_NONE),
        Result(OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, REASON_CRASH_PENDING),
    };
    for (const CycleResult_t &c : cases)
    {
        Event_t ev;
        Event_Init(&ev);
        ASSERT_EQ(STATUS_OK, Event_Record(&ev, &c, 1.25));
        EventEntry_t out[HISTORY_CAPACITY];
        size_t count = 0;
        ASSERT_EQ(STATUS_OK, Event_Query(&ev, out, HISTORY_CAPACITY, &count));
        ASSERT_EQ(1U, count);
        EXPECT_EQ(1U, out[0].id);
        EXPECT_DOUBLE_EQ(1.25, out[0].timestamp_s);
        EXPECT_EQ(c.output[DOOR_LEFT], out[0].lock_left);
        EXPECT_EQ(c.output[DOOR_RIGHT], out[0].lock_right);
        EXPECT_EQ(c.state, out[0].state);
        EXPECT_EQ(c.reason_code_summary, out[0].reason_code);
        ASSERT_EQ(STATUS_OK, Event_Record(&ev, &c, 1.26)); // 같은 값 반복은 기록하지 않는다
        Event_Query(&ev, out, HISTORY_CAPACITY, &count);
        EXPECT_EQ(1U, count);
    }
}

/** 목적: 최근 100건 보존과 ID 유일성 (SRS-F-020, 021, SRS-L-006). 입력: 값이 계속 바뀌는 250건. 기대: 최신 100건만 오래된 순서로 조회되고 ID는 151~250, 중복 없음, cap 부족이면 ERR_PARAM. 추적: UT-113 -> DD-F-049, DD-F-050. 기법: 경계값 분석 */
TEST(EventUnit, RingBuffer_KeepsLastHundredInOrder)
{
    Event_t ev;
    Event_Init(&ev);
    for (int i = 0; i < 250; i++)
    {
        CycleResult_t r = Result(i % 2 == 0 ? OUTPUT_LOCK : OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_NORMAL, REASON_NONE);
        ASSERT_EQ(STATUS_OK, Event_Record(&ev, &r, i * 0.01));
    }
    EventEntry_t out[HISTORY_CAPACITY];
    size_t count = 0;
    ASSERT_EQ(STATUS_OK, Event_Query(&ev, out, HISTORY_CAPACITY, &count));
    ASSERT_EQ(100U, count);
    for (size_t i = 0; i < count; i++)
    {
        EXPECT_EQ(151U + i, out[i].id);
    }
    EXPECT_EQ(STATUS_ERR_PARAM, Event_Query(&ev, out, 99, &count));
}

/** 목적: 재초기화하면 이력이 사라진다 (휘발성, SRS-F-026). 입력: 기록 후 Event_Init. 기대: 0건, 다음 ID 1. 추적: UT-114 -> DD-F-046, SRS-F-026. 기법: 동치 클래스 */
TEST(EventUnit, Reinitialize_ClearsHistory)
{
    Event_t ev;
    Event_Init(&ev);
    CycleResult_t r = Result(OUTPUT_LOCK, OUTPUT_LOCK, STATE_NORMAL, REASON_AUTO_LOCK);
    Event_Record(&ev, &r, 0.0);
    Event_Init(&ev);
    EventEntry_t out[HISTORY_CAPACITY];
    size_t count = 5;
    ASSERT_EQ(STATUS_OK, Event_Query(&ev, out, HISTORY_CAPACITY, &count));
    EXPECT_EQ(0U, count);
    Event_Record(&ev, &r, 0.0);
    Event_Query(&ev, out, HISTORY_CAPACITY, &count);
    EXPECT_EQ(1U, out[0].id);
}

} // namespace
