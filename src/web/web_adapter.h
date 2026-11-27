/**
 * @file    web_adapter.h
 * @brief   HTTP 소켓 어댑터 (ARCH-C-016 모듈): 소켓에서 요청을 읽어 Web_Handle에 넘기고 응답을 보낸다.
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
#ifndef WEB_WEB_ADAPTER_H
#define WEB_WEB_ADAPTER_H

#include <stddef.h>

#include "web/web_handler.h"

/** 요청 버퍼 크기 (바이트) */
#define WEB_REQUEST_CAPACITY 8192

/**
 * @brief 한 요청을 읽는다 (헤더 끝까지 읽고 Content-Length만큼 본문을 더 읽는다).
 * @param[in]  fd   소켓
 * @param[out] buf  요청 버퍼 (종료 문자 포함)
 * @param[in]  cap  버퍼 크기 (2 이상)
 * @return 헤더를 끝까지 읽었으면 1, 아니면 0
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
int WebAdapter_ReadRequest(int fd, char *buf, size_t cap);

/**
 * @brief 요청 문자열을 해석해 Web_Handle로 처리하고 HTTP 응답을 보낸다. 경로가 63자를 넘으면 잘린 경로로 처리되어 404가 된다.
 * @param[in]    fd   소켓
 * @param[inout] web  Web 컨텍스트
 * @param[in]    buf  요청 문자열 (요청 라인으로 시작)
 * @return 요청 라인을 해석했으면 1, 아니면 0
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
int WebAdapter_Serve(int fd, Web_t *web, const char *buf);

/**
 * @brief 수신 소켓을 만들어 포트에 묶고 대기 상태로 한다 (포트 0이면 임의 포트).
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
int WebAdapter_Listen(int port);

/**
 * @brief 연결을 최대 max_requests건 받아 한 건씩 직렬로 처리한다. accept가 실패하면 멈춘다.
 * @param[in]    server        수신 소켓
 * @param[inout] web           Web 컨텍스트
 * @param[in]    max_requests  처리할 최대 연결 수
 * @return 처리한 연결 수
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
int WebAdapter_Run(int server, Web_t *web, int max_requests);

#endif /* WEB_WEB_ADAPTER_H */
