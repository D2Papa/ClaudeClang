/**
 * @file    crash_rule.c
 * @brief   충돌 규칙 구현.
 * @trace   SSR-001, SSR-002, SSR-003
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
#include "control/crash_rule.h"
#include <stddef.h>

Status_t Crash_Evaluate(const RuleInput_t *in, RuleResult_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((in != NULL) && (out != NULL))
    {
        RuleResult_t cleared = {0};

        *out = cleared;
        if (in->snapshot->crash.validity == VALIDITY_VALID)
        {
            if (in->snapshot->crash.value == CRASH_CONFIRMED)
            {
                int d;

                for (d = 0; d < DOOR_COUNT; d++)
                {
                    out->active[d] = 1;
                    out->effect[d] = OUTPUT_RELEASE;
                    out->reason[d] = REASON_CRASH_CONFIRMED;
                }
            }
            else if (in->snapshot->crash.value == CRASH_PENDING)
            {
                out->info_reason[DOOR_LEFT] = REASON_CRASH_PENDING;
                out->info_reason[DOOR_RIGHT] = REASON_CRASH_PENDING;
            }
        }
        result = STATUS_OK;
    }
    return result;
}
