/**
 * @file    approach_risk_rule.c
 * @brief   접근위험 규칙 구현.
 * @trace   SSR-004, SSR-005, SSR-006
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
#include "control/approach_risk_rule.h"
#include <stddef.h>

/**
 * @brief 한 도어의 접근위험을 평가한다. override가 지속 중이면 평가하지 않는다.
 * @param[in]  in   규칙 입력
 * @param[in]  d    도어 인덱스
 * @param[out] out  규칙 결과
 * @return STATUS_OK
 * @trace SSR-004, SSR-005, SSR-006
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
static Status_t ApproachRisk_EvaluateDoor(const RuleInput_t *in, int d, RuleResult_t *out)
{
    const Snapshot_t *snap = in->snapshot;

    if ((snap->approach[d].validity == VALIDITY_VALID) && (snap->approach[d].value == 1) &&
        (in->override_status.active[d] == 0))
    {
        out->active[d] = 1;
        out->effect[d] = OUTPUT_LOCK;
        out->reason[d] = REASON_APPROACH_RISK_LOCK;
        if ((snap->cmd.validity == VALIDITY_VALID) && (snap->cmd.action == ACTION_UNLOCK) && (snap->cmd.target[d] == 1))
        {
            out->suppressed[d] = 1;
            out->reason[d] = REASON_APPROACH_RISK_SUPPRESSED;
        }
    }
    return STATUS_OK;
}

Status_t ApproachRisk_Evaluate(const RuleInput_t *in, RuleResult_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((in != NULL) && (out != NULL))
    {
        RuleResult_t cleared = {0};

        *out = cleared;
        int d;

        for (d = 0; d < DOOR_COUNT; d++)
        {
            ApproachRisk_EvaluateDoor(in, d, out);
        }
        result = STATUS_OK;
    }
    return result;
}
