/**
 * @file    web_handler.h
 * @brief   HTTP 요청 처리: 제어 주기, 이력, 표시 조합 (ARCH-C-016, ARCH-I-010).
 * @trace   SRS-C-004, SRS-F-018, SRS-F-023, SRS-F-025
 *
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
#ifndef WEB_WEB_HANDLER_H
#define WEB_WEB_HANDLER_H

#include "common/types.h"
#include "control/control_cycle.h"
#include "history/event_recorder.h"
#include <stddef.h>

/** 응답 본문 버퍼 크기 (바이트) */
#define WEB_BODY_CAPACITY 16384

/** 어댑터의 기본 포트 */
#define WEB_DEFAULT_PORT 8081

/** 요청 (모든 포인터는 널이 아니다, 본문이 없으면 빈 문자열) */
typedef struct
{
    const char *method; /**< GET 또는 POST */
    const char *path;   /**< 경로 */
    const char *body;   /**< 본문 */
} WebRequest_t;

/** 응답 */
typedef struct
{
    int status;                       /**< HTTP 상태 코드 */
    const char *content_type;        /**< 본문 종류 */
    char body[WEB_BODY_CAPACITY];    /**< 본문 */
} WebResponse_t;

/** Web 컨텍스트 */
typedef struct
{
    int initialized;       /**< Web_Init 호출 여부 */
    Control_t control;     /**< 제어 컨텍스트 */
    Event_t events;        /**< 이력 컨텍스트 */
    CycleResult_t last;    /**< 마지막 평가 결과 */
    int has_last;          /**< 평가된 주기가 있으면 1 */
    int cycle;             /**< 다음 주기 번호 */
    size_t body_limit;     /**< 응답 본문 상한 (1 이상 WEB_BODY_CAPACITY 이하) */
} Web_t;

/**
 * @brief Web 컨텍스트를 초기화한다.
 * @param[out] ctx  컨텍스트 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-051
 *
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
Status_t Web_Init(Web_t *ctx);

/**
 * @brief 응답 본문 상한을 바꾼다 (시험용). 범위는 1 이상 WEB_BODY_CAPACITY 이하.
 * @param[inout] ctx    컨텍스트 (초기화됨)
 * @param[in]    limit  본문 상한
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널, 미초기화 또는 범위 밖
 * @trace DD-F-070
 *
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
Status_t Web_SetBodyLimit(Web_t *ctx, size_t limit);

/**
 * @brief HTTP 요청 하나를 처리해 응답을 만든다. HTTP 오류(404, 405, 400, 500)는 resp->status로 표현하고 반환값은 STATUS_OK이다.
 * @param[inout] ctx   컨텍스트
 * @param[in]    req   요청
 * @param[out]   resp  응답
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 인자 / STATUS_ERR_STATE 미초기화
 * @trace DD-F-052
 *
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
Status_t Web_Handle(Web_t *ctx, const WebRequest_t *req, WebResponse_t *resp);

#endif /* WEB_WEB_HANDLER_H */
