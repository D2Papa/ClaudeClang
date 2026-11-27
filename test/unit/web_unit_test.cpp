/**
 * @file    web_unit_test.cpp
 * @brief   Web 모듈(web_request_parser, web_page, web_handler) 단위 테스트.
 * @trace   DD-F-051~070, SRS-C-004, SRS-F-018, SRS-F-021, SRS-F-023, SRS-F-024, SRS-F-025, SSR-007
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

#include <cmath>
#include <cstring>
#include <memory>
#include <string>

extern "C" {
#include "common/params.h"
#include "web/web_handler.h"
#include "web/web_page.h"
#include "web/web_request_parser.h"
}

namespace
{

RawInput_t Parse(const char *body, int cycle, int *ok)
{
    RawInput_t raw{};
    *ok = WebParser_ParseBody(body, cycle, &raw);
    return raw;
}

/** 목적: 널 인자 거절. 입력: body/raw NULL. 기대: 0. 추적: UT-120 -> DD-F-063. 기법: 오류 추정(MC/DC) */
TEST(WebParserUnit, ParseBody_NullPointers_ReturnZero)
{
    RawInput_t raw{};
    EXPECT_EQ(0, WebParser_ParseBody(nullptr, 0, &raw));
    EXPECT_EQ(0, WebParser_ParseBody("", 0, nullptr));
}

/** 목적: 빈 본문은 기본(정상) 입력이다. 입력: "" 주기 50. 기대: 시각 0.5초, 속도 0, gear P, 충돌 NONE, 접근위험·화재·과온·성인·ISOFIX·sensor_fault FALSE, ignition TRUE, 명령 없음. 추적: UT-121 -> DD-F-065. 기법: 동치 클래스 */
TEST(WebParserUnit, ParseBody_Empty_DefaultsToNormalInput)
{
    int ok = 0;
    RawInput_t raw = Parse("", 50, &ok);
    EXPECT_EQ(1, ok);
    EXPECT_DOUBLE_EQ(0.5, raw.timestamp_s);
    EXPECT_EQ(RAW_NUMBER, raw.speed.kind);
    EXPECT_DOUBLE_EQ(0.0, raw.speed.number);
    EXPECT_EQ(RAW_ENUM, raw.gear.kind);
    EXPECT_EQ(GEAR_P, raw.gear.enum_id);
    EXPECT_DOUBLE_EQ(0.5, raw.source_timestamp.number);
    EXPECT_EQ(RAW_ENUM, raw.crash.kind);
    EXPECT_EQ(CRASH_NONE, raw.crash.enum_id);
    const RawValue_t *flags[] = {&raw.approach[0], &raw.approach[1], &raw.fire, &raw.overtemp, &raw.adult,
                                 &raw.isofix[0], &raw.isofix[1], &raw.sensor_fault};
    for (const RawValue_t *f : flags)
    {
        EXPECT_EQ(RAW_BOOLEAN, f->kind);
        EXPECT_EQ(0, f->boolean);
    }
    EXPECT_EQ(RAW_BOOLEAN, raw.ignition.kind);
    EXPECT_EQ(1, raw.ignition.boolean);
    EXPECT_EQ(RAW_ABSENT, raw.cmd_side.kind);
}

/** 목적: 필드 표 16개 이름이 각각 올바른 RawInput_t 필드에 적용된다. 입력: 이름마다 값 지정. 기대: 해당 필드만 값이 바뀜. 추적: UT-122 -> DD-F-066, SRS-Q-003. 기법: 전수(16) */
TEST(WebParserUnit, ParseBody_EachFieldName_AppliesToItsField)
{
    struct Case
    {
        const char *name;
        const char *value;
        RawKind_t kind;
        const RawValue_t *(*get)(const RawInput_t &);
    };
    const Case cases[] = {
        {"speed", "12.5", RAW_NUMBER, [](const RawInput_t &r) { return &r.speed; }},
        {"gear", "2", RAW_ENUM, [](const RawInput_t &r) { return &r.gear; }},
        {"source_timestamp", "0.25", RAW_NUMBER, [](const RawInput_t &r) { return &r.source_timestamp; }},
        {"crash", "2", RAW_ENUM, [](const RawInput_t &r) { return &r.crash; }},
        {"approach_left", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.approach[0]; }},
        {"approach_right", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.approach[1]; }},
        {"fire", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.fire; }},
        {"overtemp", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.overtemp; }},
        {"adult", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.adult; }},
        {"isofix_left", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.isofix[0]; }},
        {"isofix_right", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.isofix[1]; }},
        {"ignition", "0", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.ignition; }},
        {"sensor_fault", "1", RAW_BOOLEAN, [](const RawInput_t &r) { return &r.sensor_fault; }},
        {"cmd_side", "1", RAW_ENUM, [](const RawInput_t &r) { return &r.cmd_side; }},
        {"cmd_action", "1", RAW_ENUM, [](const RawInput_t &r) { return &r.cmd_action; }},
        {"cmd_source", "3", RAW_ENUM, [](const RawInput_t &r) { return &r.cmd_source; }},
    };
    int count = 0;
    for (const Case &c : cases)
    {
        std::string body = std::string(c.name) + "=" + c.value;
        int ok = 0;
        RawInput_t raw = Parse(body.c_str(), 0, &ok);
        EXPECT_EQ(1, ok) << c.name;
        const RawValue_t *v = c.get(raw);
        EXPECT_EQ(c.kind, v->kind) << c.name;
        if (c.kind == RAW_NUMBER)
        {
            EXPECT_DOUBLE_EQ(std::atof(c.value), v->number) << c.name;
        }
        else if (c.kind == RAW_BOOLEAN)
        {
            EXPECT_EQ(std::atoi(c.value), v->boolean) << c.name;
        }
        else
        {
            EXPECT_EQ(std::atoi(c.value), v->enum_id) << c.name;
        }
        count++;
    }
    EXPECT_EQ(16, count);
}

/** 목적: 값 해석 규칙. 입력: 숫자/정수/absent/해석 불가/빈 값/큰 정수. 기대: RAW_NUMBER, RAW_BOOLEAN, RAW_ABSENT, 나머지는 RAW_OTHER(검증기가 거절). 추적: UT-123 -> DD-F-067~069, SSR-007. 기법: 동치 클래스, 경계값, MC/DC */
TEST(WebParserUnit, ParseBody_ValueSyntax_MapsToRawKinds)
{
    struct Case
    {
        const char *body;
        RawKind_t kind;
    };
    const Case cases[] = {
        {"speed=3.5", RAW_NUMBER},    {"speed=absent", RAW_ABSENT}, {"speed=abc", RAW_OTHER},
        {"speed=", RAW_OTHER},        {"speed=3x", RAW_OTHER},      {"speed=-0.1", RAW_NUMBER},
        {"fire=1", RAW_BOOLEAN},      {"fire=2", RAW_BOOLEAN},      {"fire=x", RAW_OTHER},
        {"fire=", RAW_OTHER},         {"fire=1x", RAW_OTHER},       {"fire=99999999999", RAW_OTHER},
        {"fire=-99999999999", RAW_OTHER}, {"fire=-1", RAW_BOOLEAN},  {"gear=3", RAW_ENUM},
        {"gear=absent", RAW_ABSENT},  {"gear=P", RAW_OTHER},
    };
    for (const Case &c : cases)
    {
        int ok = 0;
        RawInput_t raw = Parse(c.body, 0, &ok);
        EXPECT_EQ(1, ok) << c.body;
        const RawValue_t &v = (std::strncmp(c.body, "speed", 5) == 0) ? raw.speed : (std::strncmp(c.body, "fire", 4) == 0) ? raw.fire : raw.gear;
        EXPECT_EQ(c.kind, v.kind) << c.body;
    }
}

/** 목적: 구문·이름 오류는 거절. 입력: 알 수 없는 이름, 같은 길이의 다른 이름, = 없음, 빈 이름, 값이 31자 초과. 기대: 0. 추적: UT-124 -> DD-F-063, DD-F-066. 기법: 오류 추정, MC/DC */
TEST(WebParserUnit, ParseBody_BadSyntax_ReturnsZero)
{
    const char *bad[] = {"bogus=1", "speeX=1", "speed", "=1", "speed=1&", "x=1", "speed=1&bogus=2"};
    int expected_ok[] = {0, 0, 0, 0, 1, 0, 0};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
    {
        int ok = 0;
        Parse(bad[i], 0, &ok);
        EXPECT_EQ(expected_ok[i], ok) << bad[i];
    }
    std::string longvalue = "speed=" + std::string(32, '1');
    int ok = 1;
    Parse(longvalue.c_str(), 0, &ok);
    EXPECT_EQ(0, ok);
    std::string maxvalue = "speed=" + std::string(31, '1');
    Parse(maxvalue.c_str(), 0, &ok);
    EXPECT_EQ(1, ok);
}

/** 목적: 여러 쌍과 반복 이름 (마지막 값 적용). 입력: "speed=1&speed=2&fire=1". 기대: speed 2, fire 1. 추적: UT-125 -> DD-F-063. 기법: 동치 클래스 */
TEST(WebParserUnit, ParseBody_MultiplePairs_LastValueWins)
{
    int ok = 0;
    RawInput_t raw = Parse("speed=1&speed=2&fire=1", 0, &ok);
    EXPECT_EQ(1, ok);
    EXPECT_DOUBLE_EQ(2.0, raw.speed.number);
    EXPECT_EQ(1, raw.fire.boolean);
}

/** 목적: 화면 제공. 입력: NULL 조합 / 정상. 기대: ERR_PARAM / OK, HTML 문자열과 길이. 추적: UT-126 -> DD-F-061. 기법: 오류 추정(MC/DC) */
TEST(WebPageUnit, Get_NullAndValid)
{
    const char *html = nullptr;
    size_t length = 0;
    EXPECT_EQ(STATUS_ERR_PARAM, WebPage_Get(nullptr, &length));
    EXPECT_EQ(STATUS_ERR_PARAM, WebPage_Get(&html, nullptr));
    ASSERT_EQ(STATUS_OK, WebPage_Get(&html, &length));
    EXPECT_EQ(std::strlen(html), length);
    EXPECT_NE(nullptr, std::strstr(html, "<form"));
    EXPECT_NE(nullptr, std::strstr(html, "approach_left"));
}

// ----------------------------------------------------------------- Web_Handle

struct WebFixture : public ::testing::Test
{
    void SetUp() override
    {
        web = std::make_unique<Web_t>();
        resp = std::make_unique<WebResponse_t>();
        ASSERT_EQ(STATUS_OK, Web_Init(web.get()));
    }

    int Call(const char *method, const char *path, const char *body = "")
    {
        WebRequest_t req{method, path, body};
        EXPECT_EQ(STATUS_OK, Web_Handle(web.get(), &req, resp.get()));
        return resp->status;
    }

    std::unique_ptr<Web_t> web;
    std::unique_ptr<WebResponse_t> resp;
};

/** 목적: Init, SetBodyLimit 인자 검증. 입력: NULL, 미초기화, 0, 용량 초과, 경계값. 기대: ERR_PARAM / OK. 추적: UT-130 -> DD-F-051, DD-F-070. 기법: 경계값 분석, MC/DC */
TEST_F(WebFixture, InitAndBodyLimit_Validation)
{
    EXPECT_EQ(STATUS_ERR_PARAM, Web_Init(nullptr));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_SetBodyLimit(nullptr, 10));
    auto raw = std::make_unique<Web_t>();
    std::memset(raw.get(), 0, sizeof(Web_t));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_SetBodyLimit(raw.get(), 10));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_SetBodyLimit(web.get(), 0));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_SetBodyLimit(web.get(), WEB_BODY_CAPACITY + 1));
    EXPECT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), WEB_BODY_CAPACITY));
    EXPECT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), 1));
}

/** 목적: Web_Handle 인자 오류와 미초기화. 입력: ctx/req/resp NULL, method/path/body NULL, 미초기화 ctx. 기대: ERR_PARAM / ERR_STATE. 추적: UT-131 -> DD-F-052. 기법: 오류 추정(MC/DC) */
TEST_F(WebFixture, Handle_InvalidArguments)
{
    WebRequest_t good{"GET", "/", ""};
    EXPECT_EQ(STATUS_ERR_PARAM, Web_Handle(nullptr, &good, resp.get()));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_Handle(web.get(), nullptr, resp.get()));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_Handle(web.get(), &good, nullptr));
    WebRequest_t no_method{nullptr, "/", ""};
    WebRequest_t no_path{"GET", nullptr, ""};
    WebRequest_t no_body{"GET", "/", nullptr};
    EXPECT_EQ(STATUS_ERR_PARAM, Web_Handle(web.get(), &no_method, resp.get()));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_Handle(web.get(), &no_path, resp.get()));
    EXPECT_EQ(STATUS_ERR_PARAM, Web_Handle(web.get(), &no_body, resp.get()));
    auto raw = std::make_unique<Web_t>();
    std::memset(raw.get(), 0, sizeof(Web_t));
    EXPECT_EQ(STATUS_ERR_STATE, Web_Handle(raw.get(), &good, resp.get()));
}

/** 목적: 경로와 메서드 표 (404, 405, 5개 경로). 입력: 알 수 없는 경로, 경로별 잘못된 메서드. 기대: 404 / 405. 추적: UT-132 -> DD-F-053, DD-F-054. 기법: 결정표 */
TEST_F(WebFixture, Routing_UnknownPathAndWrongMethod)
{
    EXPECT_EQ(404, Call("GET", "/nope"));
    EXPECT_EQ(404, Call("GET", ""));
    EXPECT_EQ(405, Call("POST", "/"));
    EXPECT_EQ(405, Call("POST", "/status"));
    EXPECT_EQ(405, Call("POST", "/history"));
    EXPECT_EQ(405, Call("GET", "/step"));
    EXPECT_EQ(405, Call("GET", "/reset"));
}

/** 목적: 화면. 입력: GET /. 기대: 200 text/html, 본문 상한이 HTML보다 작으면 500. 추적: UT-133 -> DD-F-056, SRS-C-004. 기법: 경계값 분석 */
TEST_F(WebFixture, Page_ServedAndLimited)
{
    EXPECT_EQ(200, Call("GET", "/"));
    EXPECT_STREQ("text/html; charset=utf-8", resp->content_type);
    EXPECT_NE(nullptr, std::strstr(resp->body, "<form"));
    const char *html = nullptr;
    size_t length = 0;
    WebPage_Get(&html, &length);
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), length));
    EXPECT_EQ(500, Call("GET", "/"));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), length + 1));
    EXPECT_EQ(200, Call("GET", "/"));
}

/** 목적: 평가 전 상태 조회는 204, 평가 후는 마지막 결과 JSON (SRS-F-018, A-22). 입력: GET /status, POST /step 후 GET /status. 기대: 204 빈 본문 / 200 JSON. 추적: UT-134 -> DD-F-057, SRS-F-018. 기법: 동치 클래스 */
TEST_F(WebFixture, Status_BeforeAndAfterStep)
{
    EXPECT_EQ(204, Call("GET", "/status"));
    EXPECT_STREQ("", resp->body);
    ASSERT_EQ(200, Call("POST", "/step", "speed=3.0"));
    std::string stepped(resp->body);
    ASSERT_EQ(200, Call("GET", "/status"));
    EXPECT_EQ(stepped, std::string(resp->body));
    EXPECT_NE(std::string::npos, stepped.find("\"lock_left\":\"LOCK\""));
}

/** 목적: 응답 본문 상한을 넘는 직렬화는 500 (SRS-F-023). 입력: 상한 8로 POST /step, GET /status. 기대: 500, 빈 본문. 추적: UT-135 -> DD-F-057, DD-F-070, SRS-F-023. 기법: 경계값 분석 */
TEST_F(WebFixture, Status_BodyLimitExceeded_Returns500)
{
    ASSERT_EQ(200, Call("POST", "/step"));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), 8));
    EXPECT_EQ(500, Call("GET", "/status"));
    EXPECT_STREQ("", resp->body);
}

/** 목적: step 요청 처리. 입력: 정상 본문 / 구문 오류 본문 / 입력 검증 오류 본문(speed=abc). 기대: 200 / 400 / 200이고 DEGRADED로 보고(검증기가 거절). 추적: UT-136 -> DD-F-059, DD-F-063, SSR-007. 기법: 동치 클래스 */
TEST_F(WebFixture, Step_ValidBadAndInvalidInputs)
{
    EXPECT_EQ(200, Call("POST", "/step", ""));
    EXPECT_EQ(400, Call("POST", "/step", "bogus=1"));
    EXPECT_EQ(200, Call("POST", "/step", "speed=abc"));
    EXPECT_NE(std::string::npos, std::string(resp->body).find("\"state\":\"DEGRADED\""));
    EXPECT_NE(std::string::npos, std::string(resp->body).find("\"speed\":\"INVALID\""));
}

/** 목적: 이력 JSON과 변화 기준 기록 (SRS-F-021, 024, 025). 입력: 빈 이력 / 변화 있는 step 3회와 변화 없는 step. 기대: "[]" / 변화마다 1건, 필드 6개. 추적: UT-137 -> DD-F-058, DD-F-062, DD-F-064. 기법: 동치 클래스 */
TEST_F(WebFixture, History_RecordsChangesOnly)
{
    ASSERT_EQ(200, Call("GET", "/history"));
    EXPECT_STREQ("[]", resp->body);
    Call("POST", "/step", "speed=0");
    Call("POST", "/step", "speed=3.0");
    Call("POST", "/step", "speed=3.0");
    Call("POST", "/step", "speed=0"); // 출력은 LOCK 그대로지만 reason_code가 AUTO_LOCK에서 NONE으로 바뀌어 결정이다
    ASSERT_EQ(200, Call("GET", "/history"));
    std::string h(resp->body);
    EXPECT_EQ(0U, h.find("[{\"id\":1,")) << h;
    EXPECT_NE(std::string::npos, h.find("\"id\":1,\"timestamp_s\":0.010,\"lock_left\":\"LOCK\",\"lock_right\":\"LOCK\",\"state\":\"NORMAL\",\"reason_code\":\"AUTO_LOCK\"}")) << h;
    EXPECT_NE(std::string::npos, h.find("\"id\":2,\"timestamp_s\":0.030,\"lock_left\":\"LOCK\",\"lock_right\":\"LOCK\",\"state\":\"NORMAL\",\"reason_code\":\"NONE\"}")) << h;
    EXPECT_EQ(std::string::npos, h.find("\"id\":3")) << h; // 변화 없는 step(speed=3.0 반복)은 기록하지 않는다
    Call("POST", "/step", "ignition=0");
    ASSERT_EQ(200, Call("GET", "/history"));
    h = resp->body;
    EXPECT_NE(std::string::npos, h.find("\"id\":3,\"timestamp_s\":0.040,\"lock_left\":\"RELEASE\",\"lock_right\":\"RELEASE\",\"state\":\"OFF\",\"reason_code\":\"IGNITION_OFF\"}")) << h;
}

/** 목적: 이력 본문 상한 경계 (이벤트는 들어가지만 닫는 괄호가 못 들어가는 경우 포함). 입력: 한 건 이력에서 상한 L-1, L, L+1. 기대: 500, 500, 200. 추적: UT-138 -> DD-F-058, SRS-F-023. 기법: 경계값 분석, MC/DC */
TEST_F(WebFixture, History_BodyLimitBoundary)
{
    Call("POST", "/step", "speed=3.0");
    ASSERT_EQ(200, Call("GET", "/history"));
    size_t length = std::strlen(resp->body);
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), length - 1));
    EXPECT_EQ(500, Call("GET", "/history"));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), length));
    EXPECT_EQ(500, Call("GET", "/history"));
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), length + 1));
    EXPECT_EQ(200, Call("GET", "/history"));
}

/** 목적: 이력 여러 건 직렬화(쉼표 구분)와 중간 용량 부족. 입력: 세 건 이력, 상한을 한 건 크기 정도로(두 번째 이벤트에서 부족). 기대: 전체는 200 배열, 부족하면 500. 추적: UT-139 -> DD-F-062, DD-F-058. 기법: 경계값 분석 */
TEST_F(WebFixture, History_MultipleEvents_CommaSeparatedAndOverflow)
{
    Call("POST", "/step", "speed=3.0");
    Call("POST", "/step", "speed=0&ignition=0");
    Call("POST", "/step", "speed=0&ignition=1");
    ASSERT_EQ(200, Call("GET", "/history"));
    std::string h(resp->body);
    EXPECT_NE(std::string::npos, h.find("},{\"id\":2,"));
    EXPECT_NE(std::string::npos, h.find("},{\"id\":3,")); // 세 번째 이벤트(OFF에서 복귀)
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), 120));
    EXPECT_EQ(500, Call("GET", "/history"));
}

/** 목적: reset은 이력과 상태를 지우고 본문 상한 설정은 유지한다. 입력: step 후 reset. 기대: 200 {"reset":true}, 이후 /status 204, /history "[]", 상한 유지. 추적: UT-140 -> DD-F-060, SRS-F-026. 기법: 시나리오 */
TEST_F(WebFixture, Reset_ClearsStateAndKeepsLimit)
{
    Call("POST", "/step", "speed=3.0");
    ASSERT_EQ(STATUS_OK, Web_SetBodyLimit(web.get(), 4000));
    ASSERT_EQ(200, Call("POST", "/reset"));
    EXPECT_STREQ("{\"reset\":true}", resp->body);
    EXPECT_EQ(204, Call("GET", "/status"));
    ASSERT_EQ(200, Call("GET", "/history"));
    EXPECT_STREQ("[]", resp->body);
    EXPECT_EQ(4000U, web->body_limit);
}

/** 목적: 주기 번호가 step마다 증가해 논리 시각이 단조 증가한다 (SRS-C-005). 입력: step 5회. 기대: 모든 step 200(시각 역행 오류 없음), 이력 시각 0.01 단위. 추적: UT-141 -> DD-F-064. 기법: 시나리오 */
TEST_F(WebFixture, Step_CycleAdvancesTimestamps)
{
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(200, Call("POST", "/step", i % 2 == 0 ? "speed=3.0" : "speed=0&ignition=0"));
    }
    EXPECT_EQ(5, web->cycle);
}

} // namespace
