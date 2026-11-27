/**
 * @file    ignition_rule.c
 * @brief   ignition off 규칙 구현.
 * @trace   SRS-F-016
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
#include "control/ignition_rule.h"
#include <stddef.h>

Status_t Ignition_Evaluate(const RuleInput_t *in, RuleResult_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((in != NULL) && (out != NULL))
    {
        RuleResult_t cleared = {0};

        *out = cleared;
        if ((in->snapshot->ignition.validity == VALIDITY_VALID) && (in->snapshot->ignition.value == 0))
        {
            int d;

            for (d = 0; d < DOOR_COUNT; d++)
            {
                out->active[d] = 1;
                out->effect[d] = OUTPUT_RELEASE;
                out->reason[d] = REASON_IGNITION_OFF;
            }
        }
        result = STATUS_OK;
    }
    return result;
}
