/**
 * @file    freshness_monitor.c
 * @brief   시각 단조성과 차속 신선도 검사 구현.
 * @trace   SSR-008
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
#include "control/freshness_monitor.h"
#include "common/params.h"
#include <stddef.h>

/**
 * @brief 차속과 원천 시각이 유효하고 시간 차이가 한계를 넘거나 음수이면 오래된 것으로 판정한다.
 * @param[in] snap  검증된 입력
 * @return 오래되었으면 1, 아니면 0
 * @trace SSR-008
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
static int Freshness_IsSpeedStale(const Snapshot_t *snap)
{
    double age = snap->timestamp_s - snap->source_timestamp.value;

    return ((snap->speed.validity == VALIDITY_VALID) && (snap->source_timestamp.validity == VALIDITY_VALID) &&
            ((age > FRESHNESS_LIMIT_S) || (age < 0.0)))
               ? 1
               : 0;
}

Status_t Freshness_Init(Freshness_t *ctx)
{
    Status_t result = STATUS_ERR_PARAM;

    if (ctx != NULL)
    {
        ctx->initialized = 1;
        ctx->previous_timestamp_s = -1.0;
        result = STATUS_OK;
    }
    return result;
}

Status_t Freshness_Apply(Freshness_t *ctx, Snapshot_t *snap)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((ctx != NULL) && (snap != NULL))
    {
        if ((ctx->initialized == 0) || (snap->timestamp_s < ctx->previous_timestamp_s))
        {
            result = STATUS_ERR_STATE;
        }
        else
        {
            if (Freshness_IsSpeedStale(snap) == 1)
            {
                snap->speed.validity = VALIDITY_INVALID;
                snap->source_timestamp.validity = VALIDITY_INVALID;
            }
            ctx->previous_timestamp_s = snap->timestamp_s;
            result = STATUS_OK;
        }
    }
    return result;
}
