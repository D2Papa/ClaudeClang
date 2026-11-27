/**
 * @file    arbiter.c
 * @brief   우선순위 중재 구현.
 * @trace   SRS-F-017, SRS-F-028, SRS-F-031, SSR-010, SSR-011, SSR-013, SSR-016
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
#include "control/arbiter.h"
#include <stddef.h>

/**
 * @brief state에 따라 해당 순위를 중재에서 제외할지 판정한다 (OFF: 접근위험 이하 순위, DEGRADED/FAULT: 운전자 명령).
 * @param[in] state  state
 * @param[in] rank   순위 인덱스
 * @return 제외하면 1
 * @trace SRS-F-031, SSR-010, SSR-011
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
static int Arbiter_IsGated(State_t state, int rank)
{
    return (((state == STATE_OFF) && (rank >= (int)RANK_APPROACH)) ||
            (((state == STATE_DEGRADED) || (state == STATE_FAULT)) && (rank == (int)RANK_DRIVER)))
               ? 1
               : 0;
}

/**
 * @brief 한 순위가 도어의 출력을 정할 후보인지 판정한다 (규칙이 활성이거나 운전자 순위에서 override가 지속 중).
 * @param[in] in    중재 입력
 * @param[in] d     도어 인덱스
 * @param[in] rank  순위 인덱스
 * @return 후보이면 1
 * @trace SRS-F-017, SRS-F-030
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
static int Arbiter_IsCandidate(const ArbiterInput_t *in, int d, int rank)
{
    return ((Arbiter_IsGated(in->state, rank) == 0) &&
            ((in->rules[rank].active[d] == 1) || ((rank == (int)RANK_DRIVER) && (in->override_status.active[d] == 1))))
               ? 1
               : 0;
}

/**
 * @brief 도어의 출력을 정하는 가장 높은 순위를 찾는다.
 * @param[in] in  중재 입력
 * @param[in] d   도어 인덱스
 * @return 순위 인덱스, 없으면 -1
 * @trace SRS-F-017
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
static int Arbiter_FindRank(const ArbiterInput_t *in, int d)
{
    int found = -1;
    int rank;

    for (rank = 0; (rank < RULE_COUNT) && (found < 0); rank++)
    {
        if (Arbiter_IsCandidate(in, d, rank) == 1)
        {
            found = rank;
        }
    }
    return found;
}

/**
 * @brief 선택된 규칙이 없을 때의 reason_code를 정한다 (FAULT, DEGRADED, 규칙의 정보성 이유, NONE 순서).
 * @param[in] in  중재 입력
 * @param[in] d   도어 인덱스
 * @return 이유 코드
 * @trace SSR-009, SSR-012, SRS-F-003
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
static ReasonCode_t Arbiter_FallbackReason(const ArbiterInput_t *in, int d)
{
    ReasonCode_t reason = REASON_NONE;
    int rank;

    if (in->state == STATE_FAULT)
    {
        reason = REASON_SENSOR_FAULT;
    }
    else if (in->state == STATE_DEGRADED)
    {
        reason = REASON_DEGRADED_STALE;
    }
    else
    {
        for (rank = 0; (rank < RULE_COUNT) && (reason == REASON_NONE); rank++)
        {
            if (Arbiter_IsGated(in->state, rank) == 0)
            {
                reason = in->rules[rank].info_reason[d];
            }
        }
    }
    return reason;
}

/**
 * @brief 한 도어의 중재 결과를 정한다.
 * @param[in]  in   중재 입력
 * @param[in]  d    도어 인덱스
 * @param[out] out  중재 결과
 * @return STATUS_OK
 * @trace SRS-F-017, SRS-F-028, SRS-F-029
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
static Status_t Arbiter_ResolveDoor(const ArbiterInput_t *in, int d, Decision_t *out)
{
    int rank = Arbiter_FindRank(in, d);

    if (rank < 0)
    {
        out->output[d] = in->previous[d];
        out->priority_reason[d] = REASON_NONE;
        out->reason_code[d] = Arbiter_FallbackReason(in, d);
    }
    else if ((rank == (int)RANK_DRIVER) && (in->override_status.active[d] == 1))
    {
        out->output[d] = OUTPUT_RELEASE;
        out->priority_reason[d] = REASON_OVERRIDE;
        out->reason_code[d] = REASON_OVERRIDE;
    }
    else
    {
        out->output[d] = in->rules[rank].effect[d];
        out->priority_reason[d] = in->rules[rank].reason[d];
        out->reason_code[d] = in->rules[rank].reason[d];
    }
    return STATUS_OK;
}

Status_t Arbiter_Resolve(const ArbiterInput_t *in, Decision_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((in != NULL) && (out != NULL))
    {
        int d;

        for (d = 0; d < DOOR_COUNT; d++)
        {
            Arbiter_ResolveDoor(in, d, out);
        }
        result = STATUS_OK;
    }
    return result;
}
