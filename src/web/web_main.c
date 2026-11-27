/**
 * @file    web_main.c
 * @brief   시험용 HTTP 서버 진입점 (커버리지 측정 대상이 아닌 유일한 예외: main 한 함수, 사용자 승인 2026-11-27).
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "web/web_adapter.h"

/**
 * @brief 진입점: 인자로 포트를 받고(기본 WEB_DEFAULT_PORT) 연결을 사실상 무한히 처리한다. 모든 동작은 web_adapter가 한다.
 * @param[in] argc  인자 수
 * @param[in] argv  인자 (argv[1]이 포트)
 * @return 소켓을 열지 못했거나 연결 처리가 끝나면 1
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
    int server = WebAdapter_Listen(port);

    /* 반환값을 쓰지 않는 호출: 초기화는 널이 아닌 컨텍스트에서 실패하지 않는다. */
    Web_Init(&web);
    if (server >= 0)
    {
        printf("web_simulator: http://localhost:%d/\n", port);
        fflush(stdout);
        WebAdapter_Run(server, &web, INT_MAX);
    }
    perror("web_simulator");
    return 1;
}
