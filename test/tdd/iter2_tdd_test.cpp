/**
 * @file    iter2_tdd_test.cpp
 * @brief   표시·이력·Web TDD 테스트 (구현 단계 산출물, 단위 테스트와 분리).
 * @trace   SRS-F-018, SRS-F-019, SRS-F-020, SRS-F-021, SRS-F-023, SRS-F-025, SRS-C-004
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
#include "web/web_handler.h"
}

namespace
{

WebResponse_t Call(Web_t *ctx, const char *method, const char *path, const char *body = "")
{
    WebRequest_t req{method, path, body};
    WebResponse_t resp{};
    EXPECT_EQ(STATUS_OK, Web_Handle(ctx, &req, &resp));
    return resp;
}

TEST(Iter2Tdd, StatusBeforeAnyStep_Returns204)
{
    Web_t web{};
    ASSERT_EQ(STATUS_OK, Web_Init(&web));
    EXPECT_EQ(204, Call(&web, "GET", "/status").status);
}

TEST(Iter2Tdd, StepThenStatus_ReportsLockAndReason)
{
    Web_t web{};
    ASSERT_EQ(STATUS_OK, Web_Init(&web));
    WebResponse_t r = Call(&web, "POST", "/step", "speed=3.0");
    EXPECT_EQ(200, r.status);
    std::string body(r.body);
    EXPECT_NE(std::string::npos, body.find("\"lock_left\":\"LOCK\""));
    EXPECT_NE(std::string::npos, body.find("\"reason_code\":\"AUTO_LOCK\""));
    EXPECT_NE(std::string::npos, body.find("\"input_validity\":{\"speed\":\"VALID\""));
}

TEST(Iter2Tdd, HistoryRecordsOnlyChanges)
{
    Web_t web{};
    ASSERT_EQ(STATUS_OK, Web_Init(&web));
    Call(&web, "POST", "/step", "speed=0");
    Call(&web, "POST", "/step", "speed=0");
    Call(&web, "POST", "/step", "speed=3.0");
    std::string history(Call(&web, "GET", "/history").body);
    EXPECT_NE(std::string::npos, history.find("\"id\":1"));
    EXPECT_EQ(std::string::npos, history.find("\"id\":2"));
}

TEST(Iter2Tdd, UnknownFieldAndRoute_Rejected)
{
    Web_t web{};
    ASSERT_EQ(STATUS_OK, Web_Init(&web));
    EXPECT_EQ(400, Call(&web, "POST", "/step", "bogus=1").status);
    EXPECT_EQ(404, Call(&web, "GET", "/nope").status);
    EXPECT_EQ(405, Call(&web, "GET", "/step").status);
}

TEST(Iter2Tdd, TinyBodyLimit_Returns500)
{
    Web_t web{};
    ASSERT_EQ(STATUS_OK, Web_Init(&web));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(&web, 8));
    EXPECT_EQ(500, Call(&web, "POST", "/step", "speed=0").status);
}

TEST(Iter2Tdd, EventRing_KeepsLastHundred)
{
    Event_t ev{};
    ASSERT_EQ(STATUS_OK, Event_Init(&ev));
    for (int i = 0; i < 101; i++)
    {
        CycleResult_t r{};
        r.output[DOOR_LEFT] = (i % 2 == 0) ? OUTPUT_LOCK : OUTPUT_RELEASE;
        ASSERT_EQ(STATUS_OK, Event_Record(&ev, &r, i * 0.01));
    }
    EventEntry_t out[HISTORY_CAPACITY];
    size_t count = 0;
    ASSERT_EQ(STATUS_OK, Event_Query(&ev, out, HISTORY_CAPACITY, &count));
    EXPECT_EQ(100U, count);
    EXPECT_EQ(2U, out[0].id);
    EXPECT_EQ(101U, out[99].id);
}

} // namespace
