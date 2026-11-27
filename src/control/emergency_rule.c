/**
 * @file    emergency_rule.c
 * @brief   긴급 강제 해제 규칙 구현.
 * @trace   SRS-F-014
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
#include "control/emergency_rule.h"
#include <stddef.h>

/**
 * @brief 화재, 과온, 성인 탑승 순서로 유효한 TRUE 입력의 이유 코드를 고른다.
 * @param[in] snap  검증된 입력
 * @return 이유 코드 (없으면 REASON_NONE)
 * @trace SRS-F-014
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
static ReasonCode_t Emergency_SelectReason(const Snapshot_t *snap)
{
    ReasonCode_t reason = REASON_NONE;

    if ((snap->fire.validity == VALIDITY_VALID) && (snap->fire.value == 1))
    {
        reason = REASON_FIRE;
    }
    else if ((snap->overtemp.validity == VALIDITY_VALID) && (snap->overtemp.value == 1))
    {
        reason = REASON_OVERTEMP;
    }
    else if ((snap->adult.validity == VALIDITY_VALID) && (snap->adult.value == 1))
    {
        reason = REASON_ADULT_PRESENT;
    }
    else
    {
        reason = REASON_NONE;
    }
    return reason;
}

Status_t Emergency_Evaluate(const RuleInput_t *in, RuleResult_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((in != NULL) && (out != NULL))
    {
        RuleResult_t cleared = {0};

        *out = cleared;
        ReasonCode_t reason = Emergency_SelectReason(in->snapshot);

        if (reason != REASON_NONE)
        {
            int d;

            for (d = 0; d < DOOR_COUNT; d++)
            {
                out->active[d] = 1;
                out->effect[d] = OUTPUT_RELEASE;
                out->reason[d] = reason;
            }
        }
        result = STATUS_OK;
    }
    return result;
}
