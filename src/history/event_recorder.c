/**
 * @file    event_recorder.c
 * @brief   결정 이벤트 기록과 조회 구현.
 * @trace   SRS-F-020, SRS-F-021, SRS-F-024, SRS-F-025, SRS-F-026, SRS-L-006
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
#include "history/event_recorder.h"
#include <stddef.h>

/**
 * @brief 직전 기록과 달라졌는지 판정한다 (SRS 6.1 결정).
 * @param[in] ctx  컨텍스트
 * @param[in] res  제어 결과
 * @return 달라졌으면 1
 * @trace DD-F-048
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
static int Event_HasChanged(const Event_t *ctx, const CycleResult_t *res)
{
    int changed = 0;

    if ((res->output[DOOR_LEFT] != ctx->prev_output[DOOR_LEFT]) || (res->output[DOOR_RIGHT] != ctx->prev_output[DOOR_RIGHT]) ||
        (res->state != ctx->prev_state) || (res->reason_code_summary != ctx->prev_reason))
    {
        changed = 1;
    }
    return changed;
}

/**
 * @brief 이벤트 한 건을 순환 버퍼에 추가하고 기준값을 갱신한다.
 * @param[inout] ctx  컨텍스트
 * @param[in]    res  제어 결과
 * @param[in]    ts   논리 시각
 * @return STATUS_OK
 * @trace DD-F-049
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
static Status_t Event_Append(Event_t *ctx, const CycleResult_t *res, double ts)
{
    EventEntry_t *entry = &ctx->entries[ctx->head];

    entry->id = ctx->next_id;
    entry->timestamp_s = ts;
    entry->lock_left = res->output[DOOR_LEFT];
    entry->lock_right = res->output[DOOR_RIGHT];
    entry->state = res->state;
    entry->reason_code = res->reason_code_summary;
    ctx->next_id += 1U;
    ctx->head = (ctx->head + 1U) % HISTORY_CAPACITY;
    if (ctx->count < HISTORY_CAPACITY)
    {
        ctx->count += 1U;
    }
    ctx->prev_output[DOOR_LEFT] = res->output[DOOR_LEFT];
    ctx->prev_output[DOOR_RIGHT] = res->output[DOOR_RIGHT];
    ctx->prev_state = res->state;
    ctx->prev_reason = res->reason_code_summary;
    return STATUS_OK;
}

Status_t Event_Init(Event_t *ctx)
{
    Status_t result = STATUS_ERR_PARAM;

    if (ctx != NULL)
    {
        Event_t cleared = {{{0, 0.0, OUTPUT_RELEASE, OUTPUT_RELEASE, STATE_OFF, REASON_NONE}}, 0U, 0U, 0U, 0, {OUTPUT_RELEASE, OUTPUT_RELEASE}, STATE_NORMAL, REASON_NONE};

        *ctx = cleared;
        ctx->next_id = 1U;
        ctx->prev_state = STATE_NORMAL;
        ctx->initialized = 1;
        result = STATUS_OK;
    }
    return result;
}

Status_t Event_Record(Event_t *ctx, const CycleResult_t *res, double timestamp_s)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((ctx != NULL) && (res != NULL))
    {
        result = STATUS_ERR_STATE;
        if (ctx->initialized == 1)
        {
            if (Event_HasChanged(ctx, res) == 1)
            {
                /* 반환값을 쓰지 않는 호출: 인자가 널이 아님이 이 함수에서 보장되어 오류를 반환하지 않는다 (SRC-11). */
                Event_Append(ctx, res, timestamp_s);
            }
            result = STATUS_OK;
        }
    }
    return result;
}

Status_t Event_Query(const Event_t *ctx, EventEntry_t *out, size_t cap, size_t *count)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((ctx != NULL) && (out != NULL) && (count != NULL))
    {
        result = STATUS_ERR_STATE;
        if (ctx->initialized == 1)
        {
            result = STATUS_ERR_PARAM;
            if (cap >= ctx->count)
            {
                size_t oldest = (ctx->head + HISTORY_CAPACITY - ctx->count) % HISTORY_CAPACITY;
                size_t i;

                for (i = 0U; i < ctx->count; i++)
                {
                    out[i] = ctx->entries[(oldest + i) % HISTORY_CAPACITY];
                }
                *count = ctx->count;
                result = STATUS_OK;
            }
        }
    }
    return result;
}
