/**
 * @file    web_handler.c
 * @brief   HTTP 요청 처리 구현.
 * @trace   SRS-C-004, SRS-F-018, SRS-F-021, SRS-F-023, SRS-F-024, SRS-F-025
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
#include "web/web_handler.h"
#include "common/params.h"
#include "present/status_presenter.h"
#include "web/web_page.h"
#include "web/web_request_parser.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>

/** 경로 번호 */
typedef enum
{
    ROUTE_PAGE = 0,    /**< GET / */
    ROUTE_STATUS = 1,  /**< GET /status */
    ROUTE_HISTORY = 2, /**< GET /history */
    ROUTE_STEP = 3,    /**< POST /step */
    ROUTE_RESET = 4,   /**< POST /reset */
    ROUTE_COUNT = 5    /**< 경로 수 */
} WebRoute_t;

/** 경로 표 항목 */
typedef struct
{
    const char *path;   /**< 경로 */
    const char *method; /**< 허용 메서드 */
} WebRouteEntry_t;

/** 경로와 허용 메서드 표 (ROUTE_* 순서) */
static const WebRouteEntry_t ROUTES[ROUTE_COUNT] = {
    {"/", "GET"}, {"/status", "GET"}, {"/history", "GET"}, {"/step", "POST"}, {"/reset", "POST"}};

/**
 * @brief 상태 코드와 본문을 한 번에 설정한다.
 * @param[out] resp          응답
 * @param[in]  status        HTTP 상태
 * @param[in]  content_type  본문 종류
 * @param[in]  text          본문 문자열
 * @return STATUS_OK
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
static Status_t Web_SetText(WebResponse_t *resp, int status, const char *content_type, const char *text)
{
    resp->status = status;
    resp->content_type = content_type;
    snprintf(resp->body, WEB_BODY_CAPACITY, "%s", text);
    return STATUS_OK;
}

/**
 * @brief 경로 문자열에 해당하는 경로 번호를 찾는다.
 * @param[in] path  경로
 * @return 경로 번호, 없으면 -1
 * @trace DD-F-053
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
static int Web_FindRoute(const char *path)
{
    int found = -1;
    int i;

    for (i = 0; (i < (int)ROUTE_COUNT) && (found < 0); i++)
    {
        if (strcmp(ROUTES[i].path, path) == 0)
        {
            found = i;
        }
    }
    return found;
}

/**
 * @brief 경로에서 허용하는 메서드인지 판정한다.
 * @param[in] route   경로 번호
 * @param[in] method  메서드
 * @return 허용이면 1
 * @trace DD-F-054
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
static int Web_MethodAllowed(int route, const char *method)
{
    return (strcmp(ROUTES[route].method, method) == 0) ? 1 : 0;
}

/**
 * @brief 시험 화면을 응답한다. 본문 상한보다 길면 500.
 * @param[in]  ctx   컨텍스트
 * @param[out] resp  응답
 * @return STATUS_OK
 * @trace DD-F-056
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
static Status_t Web_RespondPage(const Web_t *ctx, WebResponse_t *resp)
{
    const char *html = NULL;
    size_t length = 0U;

    WebPage_Get(&html, &length);
    if (length < ctx->body_limit)
    {
        Web_SetText(resp, 200, "text/html; charset=utf-8", html);
    }
    else
    {
        Web_SetText(resp, 500, "text/plain", "");
    }
    return STATUS_OK;
}

/**
 * @brief 마지막 평가 결과를 JSON으로 응답한다. 평가된 주기가 없으면 204.
 * @param[in]  ctx   컨텍스트
 * @param[out] resp  응답
 * @return STATUS_OK
 * @trace DD-F-057
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
static Status_t Web_RespondStatus(const Web_t *ctx, WebResponse_t *resp)
{
    PresentedStatus_t presented;
    int http_status = 500;

    if (ctx->has_last == 0)
    {
        Web_SetText(resp, 204, "text/plain", "");
    }
    else
    {
        Presenter_Build(&ctx->last, &presented);
        Presenter_Serialize(&presented, resp->body, ctx->body_limit, &http_status);
        resp->status = http_status;
        resp->content_type = "application/json";
    }
    return STATUS_OK;
}

/**
 * @brief 이벤트 한 건을 JSON 객체로 덧붙인다.
 * @param[inout] buf    버퍼
 * @param[in]    cap    용량
 * @param[inout] pos    쓴 길이
 * @param[in]    e      이벤트
 * @param[in]    first  첫 건이면 1
 * @return 성공이면 1, 용량 부족이면 0
 * @trace DD-F-062
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
static int Web_AppendEvent(char *buf, size_t cap, size_t *pos, const EventEntry_t *e, int first)
{
    CycleResult_t shown = {{OUTPUT_RELEASE, OUTPUT_RELEASE}, STATE_NORMAL, {REASON_NONE, REASON_NONE}, {REASON_NONE, REASON_NONE}, REASON_NONE, REASON_NONE, 0, 0U};
    PresentedStatus_t presented;
    int written;
    int ok = 0;

    shown.output[DOOR_LEFT] = e->lock_left;
    shown.output[DOOR_RIGHT] = e->lock_right;
    shown.state = e->state;
    shown.reason_code_summary = e->reason_code;
    Presenter_Build(&shown, &presented);
    written = snprintf(buf + *pos, cap - *pos,
                       "%s{\"id\":%llu,\"timestamp_s\":%.3f,\"lock_left\":\"%s\",\"lock_right\":\"%s\",\"state\":\"%s\",\"reason_code\":\"%s\"}",
                       (first == 1) ? "" : ",", (unsigned long long)e->id, e->timestamp_s, presented.lock_left,
                       presented.lock_right, presented.state, presented.reason_code);
    if ((size_t)written < (cap - *pos))
    {
        *pos += (size_t)written;
        ok = 1;
    }
    return ok;
}

/**
 * @brief 이력을 JSON 배열로 응답한다. 본문 상한을 넘으면 500.
 * @param[in]  ctx   컨텍스트
 * @param[out] resp  응답
 * @return STATUS_OK
 * @trace DD-F-058
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
static Status_t Web_RespondHistory(const Web_t *ctx, WebResponse_t *resp)
{
    EventEntry_t entries[HISTORY_CAPACITY];
    size_t count = 0U;
    size_t pos = 1U;
    size_t i;
    int ok = 1;

    Event_Query(&ctx->events, entries, HISTORY_CAPACITY, &count);
    resp->body[0] = '[';
    for (i = 0U; (i < count) && (ok == 1); i++)
    {
        ok = Web_AppendEvent(resp->body, ctx->body_limit, &pos, &entries[i], (i == 0U) ? 1 : 0);
    }
    if ((ok == 1) && ((pos + 2U) <= ctx->body_limit))
    {
        resp->body[pos] = ']';
        resp->body[pos + 1U] = '\0';
        resp->status = 200;
        resp->content_type = "application/json";
    }
    else
    {
        Web_SetText(resp, 500, "text/plain", "");
    }
    return STATUS_OK;
}

/**
 * @brief 제어 주기를 한 번 실행하고 결과를 기록한다 (Web_Init과 단조 시각이 보장되어 하위 호출은 오류를 반환하지 않는다).
 * @param[inout] ctx  컨텍스트
 * @param[in]    raw  제어 입력
 * @return STATUS_OK
 * @trace DD-F-064
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
static Status_t Web_ExecuteCycle(Web_t *ctx, const RawInput_t *raw)
{
    Control_Step(&ctx->control, raw, &ctx->last);
    ctx->has_last = 1;
    Event_Record(&ctx->events, &ctx->last, raw->timestamp_s);
    ctx->cycle += 1;
    return STATUS_OK;
}

/**
 * @brief step 요청을 처리한다: 본문 해석(실패 시 400), 한 주기 실행, 상태 응답.
 * @param[inout] ctx   컨텍스트
 * @param[in]    body  본문
 * @param[out]   resp  응답
 * @return STATUS_OK
 * @trace DD-F-059
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
static Status_t Web_RespondStep(Web_t *ctx, const char *body, WebResponse_t *resp)
{
    RawInput_t raw;

    if (WebParser_ParseBody(body, ctx->cycle, &raw) == 1)
    {
        Web_ExecuteCycle(ctx, &raw);
        Web_RespondStatus(ctx, resp);
    }
    else
    {
        Web_SetText(resp, 400, "application/json", "{\"error\":\"bad request\"}");
    }
    return STATUS_OK;
}

/**
 * @brief 초기화 요청을 처리한다.
 * @param[inout] ctx   컨텍스트
 * @param[out]   resp  응답
 * @return STATUS_OK
 * @trace DD-F-060
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
static Status_t Web_RespondReset(Web_t *ctx, WebResponse_t *resp)
{
    size_t limit = ctx->body_limit;

    Web_Init(ctx);
    ctx->body_limit = limit;
    Web_SetText(resp, 200, "application/json", "{\"reset\":true}");
    return STATUS_OK;
}

/**
 * @brief 경로 번호에 따라 처리 함수를 호출한다.
 * @param[inout] ctx    컨텍스트
 * @param[in]    route  경로 번호
 * @param[in]    body   본문
 * @param[out]   resp   응답
 * @return STATUS_OK
 * @trace DD-F-055
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
static Status_t Web_Dispatch(Web_t *ctx, int route, const char *body, WebResponse_t *resp)
{
    if (route == (int)ROUTE_PAGE)
    {
        Web_RespondPage(ctx, resp);
    }
    else if (route == (int)ROUTE_STATUS)
    {
        Web_RespondStatus(ctx, resp);
    }
    else if (route == (int)ROUTE_HISTORY)
    {
        Web_RespondHistory(ctx, resp);
    }
    else if (route == (int)ROUTE_STEP)
    {
        Web_RespondStep(ctx, body, resp);
    }
    else
    {
        Web_RespondReset(ctx, resp);
    }
    return STATUS_OK;
}

/**
 * @brief 요청의 method, path, body가 모두 있는지 판정한다.
 * @param[in] req  요청
 * @return 모두 있으면 1
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
static int Web_RequestComplete(const WebRequest_t *req)
{
    return ((req->method != NULL) && (req->path != NULL) && (req->body != NULL)) ? 1 : 0;
}

Status_t Web_Init(Web_t *ctx)
{
    Status_t result = STATUS_ERR_PARAM;

    if (ctx != NULL)
    {
        Control_Init(&ctx->control);
        Event_Init(&ctx->events);
        ctx->has_last = 0;
        ctx->cycle = 0;
        ctx->body_limit = WEB_BODY_CAPACITY;
        ctx->initialized = 1;
        result = STATUS_OK;
    }
    return result;
}

Status_t Web_SetBodyLimit(Web_t *ctx, size_t limit)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((ctx != NULL) && (ctx->initialized == 1) && (limit > 0U) && (limit <= (size_t)WEB_BODY_CAPACITY))
    {
        ctx->body_limit = limit;
        result = STATUS_OK;
    }
    return result;
}

Status_t Web_Handle(Web_t *ctx, const WebRequest_t *req, WebResponse_t *resp)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((ctx != NULL) && (req != NULL) && (resp != NULL) && (Web_RequestComplete(req) == 1))
    {
        int route = Web_FindRoute(req->path);

        result = STATUS_OK;
        if (ctx->initialized == 0)
        {
            result = STATUS_ERR_STATE;
        }
        else if (route < 0)
        {
            Web_SetText(resp, 404, "text/plain", "not found");
        }
        else if (Web_MethodAllowed(route, req->method) == 0)
        {
            Web_SetText(resp, 405, "text/plain", "method not allowed");
        }
        else
        {
            Web_Dispatch(ctx, route, req->body, resp);
        }
    }
    return result;
}
