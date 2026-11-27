/**
 * @file    web_main.c
 * @brief   시험용 HTTP 어댑터 진입점: 소켓으로 요청을 받아 Web_Handle에 넘긴다 (시연으로 검증).
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
#include "web/web_handler.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/** 요청 버퍼 크기 */
#define REQUEST_CAPACITY 8192

/**
 * @brief 소켓에서 바이트를 더 읽어 버퍼 끝에 붙인다.
 * @param[in]    fd     소켓
 * @param[inout] buf    버퍼
 * @param[inout] total  읽은 길이
 * @param[in]    cap    버퍼 크기
 * @return 읽은 바이트 수 (0 이하이면 끝)
 * @trace SRS-C-004
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
static long Adapter_Fill(int fd, char *buf, size_t *total, size_t cap)
{
    ssize_t n = recv(fd, buf + *total, cap - 1U - *total, 0);

    if (n > 0)
    {
        *total += (size_t)n;
        buf[*total] = '\0';
    }
    return (long)n;
}

/**
 * @brief 헤더를 모두 읽은 뒤 Content-Length만큼 본문을 더 읽는다.
 * @param[in]    fd     소켓
 * @param[inout] buf    버퍼 (헤더가 이미 들어 있음)
 * @param[inout] total  읽은 길이
 * @param[in]    cap    버퍼 크기
 * @return 본문까지 읽은 전체 길이
 * @trace SRS-C-004
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
static size_t Adapter_ReadBody(int fd, char *buf, size_t *total, size_t cap)
{
    const char *header_end = strstr(buf, "\r\n\r\n");
    const char *length_field = strstr(buf, "Content-Length:");
    size_t need = (length_field != NULL) ? (size_t)atoi(length_field + 15) : 0U;
    size_t have = *total - (size_t)(header_end + 4 - buf);
    long n = 1;

    while ((have < need) && (n > 0) && (*total < (cap - 1U)))
    {
        n = Adapter_Fill(fd, buf, total, cap);
        have += (n > 0) ? (size_t)n : 0U;
    }
    return *total;
}

/**
 * @brief 한 요청을 읽는다 (헤더 끝까지 읽고 Content-Length만큼 본문을 더 읽는다).
 * @param[in]  fd   소켓
 * @param[out] buf  요청 버퍼
 * @param[in]  cap  버퍼 크기
 * @return 헤더를 읽었으면 1
 * @trace SRS-C-004
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
static int Adapter_ReadRequest(int fd, char *buf, size_t cap)
{
    size_t total = 0U;
    const char *header_end = NULL;
    long n = 1;

    buf[0] = '\0';
    while ((header_end == NULL) && (n > 0) && (total < (cap - 1U)))
    {
        n = Adapter_Fill(fd, buf, &total, cap);
        header_end = strstr(buf, "\r\n\r\n");
    }
    if (header_end != NULL)
    {
        Adapter_ReadBody(fd, buf, &total, cap);
    }
    return (header_end != NULL) ? 1 : 0;
}

/**
 * @brief 요청 문자열을 해석해 Web_Handle로 처리하고 HTTP 응답을 보낸다.
 * @param[in]    fd   소켓
 * @param[inout] web  Web 컨텍스트
 * @param[in]    buf  요청 문자열
 * @return 요청 라인을 해석했으면 1
 * @trace SRS-C-004
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
static int Adapter_Serve(int fd, Web_t *web, const char *buf)
{
    char method[8] = {0};
    char path[64] = {0};
    const char *body = strstr(buf, "\r\n\r\n");
    WebResponse_t resp;
    WebRequest_t req;
    /* 경로가 63자를 넘으면 잘린 경로가 되지만 모든 경로 표 항목이 짧아 어떤 항목과도 같지 않으므로 404가 된다. */
    int parsed = (sscanf(buf, "%7s %63s", method, path) == 2) ? 1 : 0;

    if (parsed == 1)
    {
        char head[256];
        int head_len;

        req.method = method;
        req.path = path;
        req.body = (body != NULL) ? body + 4 : "";
        if (Web_Handle(web, &req, &resp) != STATUS_OK)
        {
            resp.status = 500;
            resp.content_type = "text/plain";
            resp.body[0] = '\0';
        }
        head_len = snprintf(head, sizeof(head),
                            "HTTP/1.1 %d X\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                            resp.status, resp.content_type, strlen(resp.body));
        /* 반환값을 쓰지 않는 호출: 인자가 널이 아님이 이 함수에서 보장되어 오류를 반환하지 않는다 (SRC-11). */
        send(fd, head, (size_t)head_len, 0);
        send(fd, resp.body, strlen(resp.body), 0);
    }
    return parsed;
}

/**
 * @brief 수신 소켓을 만들어 포트에 묶는다.
 * @param[in] port  포트
 * @return 소켓, 실패하면 -1
 * @trace SRS-C-004
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
static int Adapter_Listen(int port)
{
    int server = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);
    /* 반환값을 쓰지 않는 호출: 인자가 널이 아님이 이 함수에서 보장되어 오류를 반환하지 않는다 (SRC-11). */
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if ((server < 0) || (bind(server, (struct sockaddr *)&addr, sizeof(addr)) != 0) || (listen(server, 8) != 0))
    {
        server = -1;
    }
    return server;
}

/**
 * @brief 진입점: 인자로 포트를 받고(기본 WEB_DEFAULT_PORT) 요청을 한 건씩 직렬로 처리한다.
 * @param[in] argc  인자 수
 * @param[in] argv  인자 (argv[1]이 포트)
 * @return 소켓을 열지 못하면 1 (정상 동작 중에는 반환하지 않는다)
 * @trace SRS-C-004
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
int main(int argc, char **argv)
{
    Web_t web;
    int port = (argc > 1) ? atoi(argv[1]) : WEB_DEFAULT_PORT;
    int server = Adapter_Listen(port);

    /* 반환값을 쓰지 않는 호출: 인자가 널이 아님이 이 함수에서 보장되어 오류를 반환하지 않는다 (SRC-11). */
    Web_Init(&web);
    if (server >= 0)
    {
        char buf[REQUEST_CAPACITY];

        printf("web_simulator: http://localhost:%d/\n", port);
        fflush(stdout);
        for (;;)
        {
            int fd = accept(server, NULL, NULL);

            if ((fd >= 0) && (Adapter_ReadRequest(fd, buf, sizeof(buf)) == 1))
            {
                Adapter_Serve(fd, &web, buf);
            }
            if (fd >= 0)
            {
                close(fd);
            }
        }
    }
    perror("web_simulator");
    return 1;
}
