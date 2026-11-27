/**
 * @file    web_integration_test.cpp
 * @brief   WebSimulator(ARCH-C-016)가 ControlCycle, EventRecorder, StatusPresenter를 인터페이스로 조합하는 통합 테스트.
 * @trace   ARCH-I-007, ARCH-I-008, ARCH-I-009, ARCH-I-010
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
#include <string>

extern "C" {
#include "web/web_handler.h"
}

namespace
{

class WebIntegration : public ::testing::Test
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

    std::unique_ptr<Web_t> web_;
    std::unique_ptr<WebResponse_t> resp_;
};

/** IT-020 목적: 입력 본문 → 제어 주기 → 표시 JSON 조합 (ARCH-I-010, 007, 008). 입력: 접근위험(좌) + 유효한 unlock 명령 두 번. 기대: SUPPRESSED(LOCK) → OVERRIDE(RELEASE)가 JSON에 나타남. 추적: ARCH-I-007, ARCH-I-008, ARCH-I-010, SRS-F-007, SRS-F-029 */
TEST_F(WebIntegration, StepJson_ShowsOverrideSequence)
{
    const char *body = "approach_left=1&cmd_side=0&cmd_action=1&cmd_source=0";
    ASSERT_EQ(200, Call("POST", "/step", body));
    EXPECT_NE(std::string::npos, Body().find("\"lock_left\":\"LOCK\""));
    EXPECT_NE(std::string::npos, Body().find("\"reason_code\":\"APPROACH_RISK_SUPPRESSED\""));
    ASSERT_EQ(200, Call("POST", "/step", body));
    EXPECT_NE(std::string::npos, Body().find("\"lock_left\":\"RELEASE\""));
    EXPECT_NE(std::string::npos, Body().find("\"priority_reason\":\"OVERRIDE\""));
}

/** IT-021 목적: 값 해석(숫자, 정수, absent, 형식 오류)이 검증기를 거쳐 입력 유효성 표시로 이어진다 (ARCH-I-001, 010, 008). 입력: speed=abc, gear=9, fire=2, crash=absent. 기대: 해당 항목 INVALID, DEGRADED. 추적: ARCH-I-001, ARCH-I-008, SSR-007 */
TEST_F(WebIntegration, ValueSyntax_ReachesValidityDisplay)
{
    ASSERT_EQ(200, Call("POST", "/step", "speed=abc&gear=9&fire=2&crash=absent&approach_left=-1"));
    std::string json = Body();
    EXPECT_NE(std::string::npos, json.find("\"state\":\"DEGRADED\""));
    EXPECT_NE(std::string::npos, json.find("\"speed\":\"INVALID\""));
    EXPECT_NE(std::string::npos, json.find("\"gear\":\"INVALID\""));
    EXPECT_NE(std::string::npos, json.find("\"fire\":\"INVALID\""));
    EXPECT_NE(std::string::npos, json.find("\"crash\":\"INVALID\""));
    EXPECT_NE(std::string::npos, json.find("\"approach_left\":\"INVALID\""));
    EXPECT_NE(std::string::npos, json.find("\"adult\":\"VALID\""));
}

/** IT-022 목적: 제어 결과가 이력에 반영되고 조회 JSON과 일치한다 (ARCH-I-009, 010). 입력: 변화가 있는 step 3회, 이력 조회, reset, 이력 조회. 기대: 이벤트 3건, reset 후 0건. 추적: ARCH-I-009, ARCH-I-010, SRS-F-020, SRS-F-025, SRS-F-026 */
TEST_F(WebIntegration, History_MirrorsControlChanges_AndResetClears)
{
    Call("POST", "/step", "speed=3.0");
    Call("POST", "/step", "speed=3.0&ignition=0");
    Call("POST", "/step", "ignition=1");
    ASSERT_EQ(200, Call("GET", "/history"));
    std::string h = Body();
    EXPECT_NE(std::string::npos, h.find("\"id\":1,"));
    EXPECT_NE(std::string::npos, h.find("\"id\":3,"));
    EXPECT_EQ(std::string::npos, h.find("\"id\":4,"));
    ASSERT_EQ(200, Call("POST", "/reset"));
    ASSERT_EQ(200, Call("GET", "/history"));
    EXPECT_EQ("[]", Body());
    EXPECT_EQ(204, Call("GET", "/status"));
}

/** IT-023 목적: 상태 이유 우선 규칙이 표시까지 전달된다 (A-25). 입력: sensor_fault + 형식 오류 명령(첫 주기). 기대: reason_code SENSOR_FAULT, 경고 W-001. 추적: ARCH-I-006, ARCH-I-008, SRS-F-032, SSR-012 */
TEST_F(WebIntegration, StateReason_BeatsInvalidCommand_InDisplay)
{
    ASSERT_EQ(200, Call("POST", "/step", "sensor_fault=1&cmd_side=7&cmd_action=0&cmd_source=0"));
    std::string json = Body();
    EXPECT_NE(std::string::npos, json.find("\"state\":\"FAULT\""));
    EXPECT_NE(std::string::npos, json.find("\"reason_code\":\"SENSOR_FAULT\""));
    EXPECT_NE(std::string::npos, json.find("\"warning_code\":\"W-001\""));
}

/** IT-024 목적: 화면, 경로 오류, 본문 상한이 조합된 HTTP 응답으로 나온다. 입력: GET /, 알 수 없는 경로, 잘못된 메서드, 잘못된 본문, 상한 초과. 기대: 200, 404, 405, 400, 500. 추적: ARCH-I-010, SRS-F-023 */
TEST_F(WebIntegration, HttpStatusCodes_AcrossRoutes)
{
    EXPECT_EQ(200, Call("GET", "/"));
    EXPECT_EQ(404, Call("GET", "/unknown"));
    EXPECT_EQ(405, Call("DELETE", "/status"));
    EXPECT_EQ(400, Call("POST", "/step", "speed"));
    ASSERT_EQ(200, Call("POST", "/step", "speed=3.0"));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web_.get(), 16));
    EXPECT_EQ(500, Call("GET", "/status"));
    EXPECT_EQ(500, Call("GET", "/history"));
}

} // namespace
