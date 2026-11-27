/**
 * @file    web_adapter_unit_test.cpp
 * @brief   HTTP 소켓 어댑터(web_adapter) 단위 테스트: socketpair와 로컬 수신 소켓으로 실제 소켓 입출력을 검증한다.
 * @trace   SRS-C-004
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

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <memory>
#include <string>

extern "C" {
#include "web/web_adapter.h"
}

namespace
{

struct Pair
{
    int a = -1;
    int b = -1;

    Pair()
    {
        int sv[2];
        EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sv));
        a = sv[0];
        b = sv[1];
    }
    ~Pair()
    {
        close(a);
        close(b);
    }
    void Send(const std::string &text) { EXPECT_EQ(static_cast<ssize_t>(text.size()), send(b, text.data(), text.size(), 0)); }
    std::string Received()
    {
        std::string out;
        char chunk[4096];
        ssize_t n;
        while ((n = recv(b, chunk, sizeof(chunk), MSG_DONTWAIT)) > 0)
        {
            out.append(chunk, static_cast<size_t>(n));
        }
        return out;
    }
};

/** 파일 디스크립터 한도를 바꾸고 소멸할 때(단정 실패 시에도) 원래 값으로 되돌리는 도우미 */
struct NofileGuard
{
    rlimit old{};
    explicit NofileGuard(rlim_t soft)
    {
        EXPECT_EQ(0, getrlimit(RLIMIT_NOFILE, &old));
        rlimit changed = old;
        changed.rlim_cur = soft;
        EXPECT_EQ(0, setrlimit(RLIMIT_NOFILE, &changed));
    }
    ~NofileGuard() { setrlimit(RLIMIT_NOFILE, &old); }
};

int PortOf(int server)
{
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    getsockname(server, reinterpret_cast<sockaddr *>(&addr), &len);
    return ntohs(addr.sin_port);
}

int Connect(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<unsigned short>(port));
    EXPECT_EQ(0, connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)));
    return fd;
}

std::string ReadAll(int fd)
{
    std::string out;
    char chunk[4096];
    ssize_t n;
    while ((n = recv(fd, chunk, sizeof(chunk), 0)) > 0)
    {
        out.append(chunk, static_cast<size_t>(n));
    }
    return out;
}

std::unique_ptr<Web_t> NewWeb()
{
    auto web = std::make_unique<Web_t>();
    EXPECT_EQ(STATUS_OK, Web_Init(web.get()));
    return web;
}

/** 목적: 본문 없는 요청 읽기. 입력: "GET /status" 헤더만. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 1과 요청 문자열 그대로. 추적: UT-150 -> WebAdapter_ReadRequest. 기법: 동치 클래스 */
TEST(WebAdapterUnit, ReadRequest_HeaderOnly_ReturnsOne)
{
    Pair p;
    p.Send("GET /status HTTP/1.1\r\nHost: x\r\n\r\n");
    char buf[WEB_REQUEST_CAPACITY];
    EXPECT_EQ(1, WebAdapter_ReadRequest(p.a, buf, sizeof(buf)));
    EXPECT_STREQ("GET /status HTTP/1.1\r\nHost: x\r\n\r\n", buf);
}

/** 목적: Content-Length만큼 본문까지 읽는다. 입력: POST 헤더와 본문 "speed=3.0". 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 1, 버퍼에 본문 포함. 추적: UT-151 -> WebAdapter_ReadRequest. 기법: 동치 클래스 */
TEST(WebAdapterUnit, ReadRequest_WithBody_ReadsBody)
{
    Pair p;
    p.Send("POST /step HTTP/1.1\r\nContent-Length: 9\r\n\r\nspeed=3.0");
    char buf[WEB_REQUEST_CAPACITY];
    EXPECT_EQ(1, WebAdapter_ReadRequest(p.a, buf, sizeof(buf)));
    EXPECT_NE(nullptr, std::strstr(buf, "\r\n\r\nspeed=3.0"));
}

/** 목적: 본문이 선언보다 짧고 상대가 연결을 닫으면 읽은 만큼만 두고 끝낸다. 입력: Content-Length 100에 본문 5바이트 후 종료. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 1(헤더는 읽음), 본문 5바이트. 추적: UT-152 -> WebAdapter_ReadRequest. 기법: 경계값, MC/DC */
TEST(WebAdapterUnit, ReadRequest_ShortBodyThenClose_ReturnsPartial)
{
    Pair p;
    p.Send("POST /step HTTP/1.1\r\nContent-Length: 100\r\n\r\nabcde");
    shutdown(p.b, SHUT_WR);
    char buf[WEB_REQUEST_CAPACITY];
    EXPECT_EQ(1, WebAdapter_ReadRequest(p.a, buf, sizeof(buf)));
    EXPECT_NE(nullptr, std::strstr(buf, "\r\n\r\nabcde"));
}

/** 목적: 헤더가 끝나기 전에 연결이 닫히면 실패. 입력: "GET /" 후 종료. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 0. 추적: UT-153 -> WebAdapter_ReadRequest. 기법: 오류 추정, MC/DC */
TEST(WebAdapterUnit, ReadRequest_ClosedBeforeHeaderEnd_ReturnsZero)
{
    Pair p;
    p.Send("GET /");
    shutdown(p.b, SHUT_WR);
    char buf[WEB_REQUEST_CAPACITY];
    EXPECT_EQ(0, WebAdapter_ReadRequest(p.a, buf, sizeof(buf)));
}

/** 목적: 버퍼가 차도록 헤더가 끝나지 않으면 실패. 입력: 용량 16 버퍼에 헤더 끝 없는 40바이트. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 0. 추적: UT-154 -> WebAdapter_ReadRequest. 기법: 경계값, MC/DC */
TEST(WebAdapterUnit, ReadRequest_BufferFullWithoutHeaderEnd_ReturnsZero)
{
    Pair p;
    p.Send(std::string(40, 'a'));
    char buf[16];
    EXPECT_EQ(0, WebAdapter_ReadRequest(p.a, buf, sizeof(buf)));
}

/** 목적: 본문이 버퍼보다 크면 버퍼가 찰 때까지만 읽는다. 입력: 용량 64 버퍼, Content-Length 200 본문 200바이트. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 1. 추적: UT-155 -> WebAdapter_ReadRequest. 기법: 경계값, MC/DC */
TEST(WebAdapterUnit, ReadRequest_BodyLargerThanBuffer_StopsAtCapacity)
{
    Pair p;
    p.Send("POST /step HTTP/1.1\r\nContent-Length: 200\r\n\r\n" + std::string(200, 'x'));
    char buf[64];
    EXPECT_EQ(1, WebAdapter_ReadRequest(p.a, buf, sizeof(buf)));
    EXPECT_EQ(63U, std::strlen(buf));
}

/** 목적: 요청 처리와 HTTP 응답 형식. 입력: GET /status(평가 전), POST /step 본문, 알 수 없는 경로, 63자 초과 경로. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 204, 200+JSON, 404, 404 (Content-Length와 Connection: close 포함). 추적: UT-156 -> WebAdapter_Serve, SRS-C-004. 기법: 동치 클래스 */
TEST(WebAdapterUnit, Serve_RoutesProduceHttpResponses)
{
    auto web = NewWeb();
    {
        Pair p;
        EXPECT_EQ(1, WebAdapter_Serve(p.a, web.get(), "GET /status HTTP/1.1\r\n\r\n"));
        std::string r = p.Received();
        EXPECT_EQ(0U, r.find("HTTP/1.1 204 X\r\n"));
        EXPECT_NE(std::string::npos, r.find("Content-Length: 0\r\nConnection: close\r\n\r\n"));
    }
    {
        Pair p;
        EXPECT_EQ(1, WebAdapter_Serve(p.a, web.get(), "POST /step HTTP/1.1\r\nContent-Length: 9\r\n\r\nspeed=3.0"));
        std::string r = p.Received();
        EXPECT_EQ(0U, r.find("HTTP/1.1 200 X\r\n"));
        EXPECT_NE(std::string::npos, r.find("Content-Type: application/json"));
        EXPECT_NE(std::string::npos, r.find("\"reason_code\":\"AUTO_LOCK\""));
    }
    {
        Pair p;
        WebAdapter_Serve(p.a, web.get(), "GET /nope HTTP/1.1\r\n\r\n");
        EXPECT_EQ(0U, p.Received().find("HTTP/1.1 404 X\r\n"));
    }
    {
        Pair p;
        std::string longpath = "GET /" + std::string(70, 'a') + " HTTP/1.1\r\n\r\n";
        WebAdapter_Serve(p.a, web.get(), longpath.c_str());
        EXPECT_EQ(0U, p.Received().find("HTTP/1.1 404 X\r\n"));
    }
}

/** 목적: 요청 라인이 없으면 응답하지 않고, 헤더 끝이 없어도 요청 라인만으로 처리하며, 미초기화 Web은 500. 입력: "x", "GET /status HTTP/1.1"(CRLF 없음), 0으로 채운 Web_t. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 0과 무응답 / 1과 204 / 1과 500. 추적: UT-157 -> WebAdapter_Serve. 기법: 오류 추정, MC/DC */
TEST(WebAdapterUnit, Serve_MalformedAndUninitialized)
{
    auto web = NewWeb();
    {
        Pair p;
        EXPECT_EQ(0, WebAdapter_Serve(p.a, web.get(), "x"));
        EXPECT_EQ("", p.Received());
    }
    {
        Pair p;
        EXPECT_EQ(1, WebAdapter_Serve(p.a, web.get(), "GET /status HTTP/1.1"));
        EXPECT_EQ(0U, p.Received().find("HTTP/1.1 204 X\r\n"));
    }
    {
        auto raw = std::make_unique<Web_t>();
        std::memset(raw.get(), 0, sizeof(Web_t));
        Pair p;
        EXPECT_EQ(1, WebAdapter_Serve(p.a, raw.get(), "GET /status HTTP/1.1\r\n\r\n"));
        EXPECT_EQ(0U, p.Received().find("HTTP/1.1 500 X\r\n"));
    }
}

/** 목적: 수신 소켓 생성. 입력: 포트 0, 이미 쓰는 포트, 파일 디스크립터 한도 0. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 성공(0 이상) / 실패(-1) / 실패(-1). 추적: UT-158 -> WebAdapter_Listen. 기법: 오류 추정, MC/DC */
TEST(WebAdapterUnit, Listen_SuccessAndFailures)
{
    int first = WebAdapter_Listen(0);
    ASSERT_GE(first, 0);
    int port = PortOf(first);
    EXPECT_EQ(-1, WebAdapter_Listen(port));
    {
        NofileGuard guard(0);
        EXPECT_EQ(-1, WebAdapter_Listen(0));
    }
    close(first);
}

/** 목적: 연결 처리 루프. 입력: 연결 2건(정상 요청, 요청 없이 종료), 최대 2건 / 최대 0건 / 잘못된 수신 소켓. 절차: 입력값을 구성하고 대상을 호출한 뒤 반환값과 출력을 기대값과 비교한다(입력이 여러 개이면 입력별로 반복하거나 순서대로 넣는다). 기대: 2 / 0 / 0, 정상 요청에는 HTTP 응답. 추적: UT-159 -> WebAdapter_Run. 기법: 경계값, MC/DC */
TEST(WebAdapterUnit, Run_ServesConnectionsAndStops)
{
    auto web = NewWeb();
    int server = WebAdapter_Listen(0);
    ASSERT_GE(server, 0);
    int port = PortOf(server);
    int good = Connect(port);
    int empty = Connect(port);
    const char *request = "GET /status HTTP/1.1\r\n\r\n";
    ASSERT_EQ(static_cast<ssize_t>(std::strlen(request)), send(good, request, std::strlen(request), 0));
    shutdown(empty, SHUT_WR);
    EXPECT_EQ(0, WebAdapter_Run(server, web.get(), 0));
    EXPECT_EQ(2, WebAdapter_Run(server, web.get(), 2));
    EXPECT_EQ(0U, ReadAll(good).find("HTTP/1.1 204 X\r\n"));
    EXPECT_EQ("", ReadAll(empty));
    close(good);
    close(empty);
    close(server);
    EXPECT_EQ(0, WebAdapter_Run(-1, web.get(), 1));
}

} // namespace
