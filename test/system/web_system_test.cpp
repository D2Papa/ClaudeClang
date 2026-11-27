/**
 * @file    web_system_test.cpp
 * @brief   표시·이력·Web 시스템 테스트 (SWE.6): HTTP 요청에서 응답까지 SRS 합격 기준을 블랙박스로 검증한다.
 * @trace   SRS-F-018~026, SRS-F-032, SRS-Q-002, SRS-Q-003, SRS-L-006
 *
 * 각 테스트는 RecordProperty("requirements", ...)로 검증하는 요구사항 ID를 기록한다.
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

#include <memory>
#include <set>
#include <string>

extern "C" {
#include "web/web_handler.h"
}

namespace
{

class WebSystem : public ::testing::Test
{
protected:
    void SetUp() override
    {
        web_ = std::make_unique<Web_t>();
        resp_ = std::make_unique<WebResponse_t>();
        ASSERT_EQ(STATUS_OK, Web_Init(web_.get()));
    }

    int Call(const char *method, const char *path, const char *body = "")
    {
        WebRequest_t req{method, path, body};
        EXPECT_EQ(STATUS_OK, Web_Handle(web_.get(), &req, resp_.get()));
        return resp_->status;
    }

    std::string Body() const { return std::string(resp_->body); }

    static bool Has(const std::string &text, const std::string &part) { return text.find(part) != std::string::npos; }

    std::unique_ptr<Web_t> web_;
    std::unique_ptr<WebResponse_t> resp_;
};

/** ST-101 SRS-F-018/019/022: 상태 조회와 표시 항목. 평가 전 204(A-22), 평가 후 좌/우 출력, state, priority_reason, reason_code, input_validity(13항목)가 6.2 값으로 응답되고 priority_reason과 reason_code가 같다. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). */
TEST_F(WebSystem, StatusQuery_ReturnsAllDisplayItems)
{
    RecordProperty("requirements", "SRS-F-018,SRS-F-019,SRS-F-022");
    EXPECT_EQ(204, Call("GET", "/status"));
    ASSERT_EQ(200, Call("POST", "/step", "speed=3.0"));
    std::string step = Body();
    ASSERT_EQ(200, Call("GET", "/status"));
    EXPECT_EQ(step, Body());
    const char *keys[] = {"lock_left", "lock_right", "state", "priority_reason", "reason_code", "warning_code", "input_validity",
                          "speed", "gear", "source_timestamp", "crash", "approach_left", "approach_right", "fire", "overtemp",
                          "adult", "isofix_left", "isofix_right", "ignition", "sensor_fault"};
    for (const char *k : keys)
    {
        EXPECT_TRUE(Has(step, std::string("\"") + k + "\":")) << k;
    }
    EXPECT_TRUE(Has(step, "\"priority_reason\":\"AUTO_LOCK\",\"reason_code\":\"AUTO_LOCK\""));
}

/** ST-102 SRS-F-023: 직렬화가 응답 본문 상한을 넘으면 HTTP 500, 충분하면 200. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). */
TEST_F(WebSystem, SerializationFailure_Returns500)
{
    RecordProperty("requirements", "SRS-F-023");
    ASSERT_EQ(200, Call("POST", "/step"));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web_.get(), 100));
    EXPECT_EQ(500, Call("GET", "/status"));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web_.get(), WEB_BODY_CAPACITY));
    EXPECT_EQ(200, Call("GET", "/status"));
}

/** ST-103 SRS-F-020/021/024/025/026, SRS-L-006: 이력. 변화마다 1건, 변화 없는 100주기는 0건, 이벤트 ID 중복 없음, 6개 필드만, 250건 입력 후 최신 100건만 오래된 순서, 재초기화 후 0건. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). */
TEST_F(WebSystem, History_ChangeEventsRingAndVolatility)
{
    RecordProperty("requirements", "SRS-F-020,SRS-F-021,SRS-F-024,SRS-F-025,SRS-F-026,SRS-L-006");
    for (int i = 0; i < 100; i++)
    {
        ASSERT_EQ(200, Call("POST", "/step", "speed=0"));
    }
    ASSERT_EQ(200, Call("GET", "/history"));
    EXPECT_EQ("[]", Body()); // 변화 없는 100주기: 이벤트 0건

    for (int i = 0; i < 250; i++)
    {
        ASSERT_EQ(200, Call("POST", "/step", i % 2 == 0 ? "ignition=0" : "ignition=1"));
    }
    ASSERT_EQ(200, Call("GET", "/history"));
    std::string h = Body();
    std::set<std::string> ids;
    size_t pos = 0;
    size_t events = 0;
    while ((pos = h.find("{\"id\":", pos)) != std::string::npos)
    {
        size_t end = h.find('}', pos);
        std::string object = h.substr(pos, end - pos + 1);
        size_t colons = 0;
        for (size_t i = 0; i + 1 < object.size(); i++)
        {
            colons += (object[i] == '"' && object[i + 1] == ':') ? 1U : 0U;
        }
        EXPECT_EQ(6U, colons) << object; // id, timestamp_s, lock_left, lock_right, state, reason_code
        ids.insert(object.substr(0, object.find(',')));
        events++;
        pos = end;
    }
    EXPECT_EQ(100U, events);
    EXPECT_EQ(100U, ids.size());
    EXPECT_EQ(0U, h.find("[{\"id\":151,")); // 250건 중 최신 100건(151~250)부터 오래된 순서
    EXPECT_TRUE(Has(h, "{\"id\":250,"));

    ASSERT_EQ(200, Call("POST", "/reset"));
    ASSERT_EQ(200, Call("GET", "/history"));
    EXPECT_EQ("[]", Body());
}

/** ST-104 SRS-F-032/003/022 (A-19, A-23, A-25): 덮어쓰기 규칙. NORMAL에서 속도 3.0 unlock은 priority AUTO_LOCK, reason_code CMD_REJECTED_SPEED. 형식 오류 명령은 NORMAL에서만 INPUT_INVALID이고 FAULT 첫 주기, DEGRADED, OFF에서는 상태 이유. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). */
TEST_F(WebSystem, ReasonOverlay_RejectedCommandsAndStatePriority)
{
    RecordProperty("requirements", "SRS-F-032,SRS-F-003,SRS-F-022");
    ASSERT_EQ(200, Call("POST", "/step", "speed=3.0&cmd_side=2&cmd_action=1&cmd_source=1"));
    std::string json = Body();
    EXPECT_TRUE(Has(json, "\"priority_reason\":\"AUTO_LOCK\""));
    EXPECT_TRUE(Has(json, "\"reason_code\":\"CMD_REJECTED_SPEED\""));

    const char *bad_command = "cmd_side=7&cmd_action=0&cmd_source=0";
    struct Case
    {
        std::string body;
        const char *state;
        const char *reason;
    };
    const Case cases[] = {
        {std::string(bad_command), "NORMAL", "INPUT_INVALID"},
        {std::string("sensor_fault=1&") + bad_command, "FAULT", "SENSOR_FAULT"},
        {std::string("fire=x&") + bad_command, "DEGRADED", "DEGRADED_STALE"},
        {std::string("ignition=0&") + bad_command, "OFF", "IGNITION_OFF"},
    };
    for (const Case &c : cases)
    {
        ASSERT_EQ(200, Call("POST", "/reset"));
        ASSERT_EQ(200, Call("POST", "/step", c.body.c_str()));
        EXPECT_TRUE(Has(Body(), std::string("\"state\":\"") + c.state + "\"")) << c.state;
        EXPECT_TRUE(Has(Body(), std::string("\"reason_code\":\"") + c.reason + "\"")) << c.state;
    }
}

/** ST-105 SRS-Q-002: 표시와 이력에는 영상, 음성, 개인 식별자 필드가 없다. 허용된 키만 나타난다. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). */
TEST_F(WebSystem, NoPersonalDataFields)
{
    RecordProperty("requirements", "SRS-Q-002");
    Call("POST", "/step", "adult=1");
    std::string status = Body();
    Call("GET", "/history");
    std::string history = Body();
    const std::set<std::string> allowed = {"lock_left", "lock_right", "state", "priority_reason", "reason_code", "warning_code",
                                           "input_validity", "speed", "gear", "source_timestamp", "crash", "approach_left",
                                           "approach_right", "fire", "overtemp", "adult", "isofix_left", "isofix_right",
                                           "ignition", "sensor_fault", "id", "timestamp_s"};
    for (const std::string &text : {status, history})
    {
        size_t pos = 0;
        while ((pos = text.find('"', pos)) != std::string::npos)
        {
            size_t end = text.find('"', pos + 1);
            if (end != std::string::npos && end + 1 < text.size() && text[end + 1] == ':')
            {
                EXPECT_TRUE(allowed.count(text.substr(pos + 1, end - pos - 1)) == 1) << text.substr(pos + 1, end - pos - 1);
            }
            pos = end + 1;
        }
    }
    EXPECT_TRUE(Has(status, "\"adult\":\"VALID\"")); // 성인 존재는 boolean 신호로만 다룬다
}

/** ST-106 SRS-Q-003: 인터페이스 값 집합. 입력 열거(gear 4, crash 3, side 3, action 2, source 4)가 모두 수용되고 출력은 LOCK/RELEASE, state는 OFF/NORMAL/DEGRADED/FAULT 집합에 속한다. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). */
TEST_F(WebSystem, InterfaceValueSets_AcceptedAndEmitted)
{
    RecordProperty("requirements", "SRS-Q-003");
    const std::set<std::string> states = {"\"OFF\"", "\"NORMAL\"", "\"DEGRADED\"", "\"FAULT\""};
    int steps = 0;
    for (int gear = 0; gear <= 3; gear++)
    {
        for (int crash = 0; crash <= 2; crash++)
        {
            for (int source = 0; source <= 3; source++)
            {
                std::string body = "gear=" + std::to_string(gear) + "&crash=" + std::to_string(crash) +
                                   "&cmd_side=" + std::to_string(source % 3) + "&cmd_action=" + std::to_string(source % 2) +
                                   "&cmd_source=" + std::to_string(source);
                ASSERT_EQ(200, Call("POST", "/reset"));
                ASSERT_EQ(200, Call("POST", "/step", body.c_str())) << body;
                std::string json = Body();
                EXPECT_TRUE(Has(json, "\"gear\":\"VALID\"")) << body;
                EXPECT_TRUE(Has(json, "\"crash\":\"VALID\"")) << body;
                EXPECT_TRUE(Has(json, "\"lock_left\":\"LOCK\"") || Has(json, "\"lock_left\":\"RELEASE\""));
                bool state_ok = false;
                for (const std::string &s : states)
                {
                    state_ok = state_ok || Has(json, "\"state\":" + s);
                }
                EXPECT_TRUE(state_ok) << body;
                steps++;
            }
        }
    }
    EXPECT_EQ(48, steps);
}

} // namespace
