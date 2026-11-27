/**
 * @file    web_adapter.c
 * @brief   HTTP 소켓 어댑터 구현.
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
#include "web/web_adapter.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

int WebAdapter_ReadRequest(int fd, char *buf, size_t cap)
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
        /* 반환값을 쓰지 않는 호출: 읽은 길이는 버퍼 안의 문자열 끝으로 이미 반영된다. */
        Adapter_ReadBody(fd, buf, &total, cap);
    }
    return (header_end != NULL) ? 1 : 0;
}

int WebAdapter_Serve(int fd, Web_t *web, const char *buf)
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
        /* 반환값을 쓰지 않는 호출: 상대가 연결을 끊어도 서버는 다음 연결을 계속 처리한다. */
        send(fd, head, (size_t)head_len, 0);
        send(fd, resp.body, strlen(resp.body), 0);
    }
    return parsed;
}

int WebAdapter_Listen(int port)
{
    int server = socket(AF_INET, SOCK_STREAM, 0);

    if (server >= 0)
    {
        int yes = 1;
        struct sockaddr_in addr;
        int rc;

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons((unsigned short)port);
        /* 반환값을 쓰지 않는 호출: 재사용 옵션 실패는 bind 결과로 드러난다. */
        setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        rc = bind(server, (struct sockaddr *)&addr, sizeof(addr));
        if (rc == 0)
        {
            rc = listen(server, 8);
        }
        if (rc != 0)
        {
            close(server);
            server = -1;
        }
    }
    return server;
}

int WebAdapter_Run(int server, Web_t *web, int max_requests)
{
    char buf[WEB_REQUEST_CAPACITY];
    int served = 0;
    int fd = 0;

    while ((served < max_requests) && (fd >= 0))
    {
        fd = accept(server, NULL, NULL);
        if (fd >= 0)
        {
            if (WebAdapter_ReadRequest(fd, buf, sizeof(buf)) == 1)
            {
                /* 반환값을 쓰지 않는 호출: 요청 라인을 해석하지 못해도 연결만 닫고 다음으로 넘어간다. */
                WebAdapter_Serve(fd, web, buf);
            }
            close(fd);
            served += 1;
        }
    }
    return served;
}
