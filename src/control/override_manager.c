/**
 * @file    override_manager.c
 * @brief   override 성립, 지속, 종료 추적 구현.
 * @trace   SRS-F-007, SRS-F-029, SRS-F-030, SRS-L-005
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
#include "control/override_manager.h"
#include "common/params.h"
#include <stddef.h>

/**
 * @brief override 지속 종료 조건을 판정한다: 접근위험 FALSE, 3.0 km/h 이상, 적용되는 lock 요청.
 * @param[in] in  갱신 입력
 * @param[in] d   도어 인덱스
 * @return 종료 조건이 성립하면 1
 * @trace SRS-F-030
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
static int Override_EndConditionMet(const OverrideInput_t *in, int d)
{
    return ((in->risk_true[d] == 0) || (in->speed_ge[d] == 1) ||
            ((in->lock_requested[d] == 1) && (in->state == STATE_NORMAL)))
               ? 1
               : 0;
}

/**
 * @brief 억제 기록이 필요한지 판정한다 (직전 주기에 억제되었고 기록이 없거나 만료됨).
 * @param[in] ctx  컨텍스트
 * @param[in] in   갱신 입력
 * @param[in] d    도어 인덱스
 * @return 기록이 필요하면 1
 * @trace SRS-F-007
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
static int Override_NeedsRecord(const Override_t *ctx, const OverrideInput_t *in, int d)
{
    int expired = ((ctx->suppressed_valid[d] == 1) && ((in->cycle - ctx->suppressed_cycle[d]) > OVERRIDE_WINDOW_CYCLES)) ? 1 : 0;

    return ((in->prev_suppressed[d] == 1) && (in->state == STATE_NORMAL) &&
            ((ctx->suppressed_valid[d] == 0) || (expired == 1)))
               ? 1
               : 0;
}

/**
 * @brief override를 성립시킬 수 있는지 판정한다 (유효한 억제 기록이 허용 주기 안에 있고 허용된 unlock 요청이 있음).
 * @param[in] ctx  컨텍스트
 * @param[in] in   갱신 입력
 * @param[in] d    도어 인덱스
 * @return 성립 가능하면 1
 * @trace SRS-F-007, SRS-L-005
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
static int Override_CanEstablish(const Override_t *ctx, const OverrideInput_t *in, int d)
{
    return ((in->state == STATE_NORMAL) && (in->unlock_allowed[d] == 1) && (in->risk_true[d] == 1) &&
            (ctx->suppressed_valid[d] == 1) && ((in->cycle - ctx->suppressed_cycle[d]) <= OVERRIDE_WINDOW_CYCLES))
               ? 1
               : 0;
}

/**
 * @brief 한 도어의 override 상태를 갱신한다.
 * @param[inout] ctx  컨텍스트
 * @param[in]    in   갱신 입력
 * @param[in]    d    도어 인덱스
 * @return STATUS_OK
 * @trace DD-F-020
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
static Status_t Override_UpdateDoor(Override_t *ctx, const OverrideInput_t *in, int d)
{
    if (ctx->active[d] == 1)
    {
        if (Override_EndConditionMet(in, d) == 1)
        {
            ctx->active[d] = 0;
            ctx->suppressed_valid[d] = 0;
        }
    }
    else
    {
        if (Override_NeedsRecord(ctx, in, d) == 1)
        {
            ctx->suppressed_cycle[d] = in->cycle - 1U;
            ctx->suppressed_valid[d] = 1;
        }
        if (Override_CanEstablish(ctx, in, d) == 1)
        {
            ctx->active[d] = 1;
            ctx->suppressed_valid[d] = 0;
        }
        if (in->risk_true[d] == 0)
        {
            ctx->suppressed_valid[d] = 0;
        }
    }
    return STATUS_OK;
}

Status_t Override_Init(Override_t *ctx)
{
    Status_t result = STATUS_ERR_PARAM;

    if (ctx != NULL)
    {
        Override_t cleared = {{0}, {0}, {0}};

        *ctx = cleared;
        result = STATUS_OK;
    }
    return result;
}

Status_t Override_Update(Override_t *ctx, const OverrideInput_t *in, OverrideStatus_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((ctx != NULL) && (in != NULL) && (out != NULL))
    {
        int d;

        for (d = 0; d < DOOR_COUNT; d++)
        {
            Override_UpdateDoor(ctx, in, d);
            out->active[d] = ctx->active[d];
        }
        result = STATUS_OK;
    }
    return result;
}
