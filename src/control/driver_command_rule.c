/**
 * @file    driver_command_rule.c
 * @brief   운전자 명령 규칙 구현.
 * @trace   SRS-F-001, SRS-F-002, SRS-F-003, SRS-L-004
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
#include "control/driver_command_rule.h"
#include "common/params.h"
#include <stddef.h>

/**
 * @brief 지정된 도어에 대해 lock 요청 또는 속도 조건을 만족하는 unlock 요청을 반영한다.
 * @param[in]  snap  검증된 입력
 * @param[in]  d     도어 인덱스
 * @param[out] out   규칙 결과
 * @return STATUS_OK
 * @trace SRS-F-002, SRS-F-003
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
static Status_t DriverCommand_EvaluateDoor(const Snapshot_t *snap, int d, RuleResult_t *out)
{
    if (snap->cmd.target[d] == 1)
    {
        if (snap->cmd.action == ACTION_LOCK)
        {
            out->active[d] = 1;
            out->effect[d] = OUTPUT_LOCK;
            out->reason[d] = REASON_CMD_APPLIED;
            out->lock_requested[d] = 1;
        }
        else if ((snap->speed.validity == VALIDITY_VALID) && (snap->speed.value < SPEED_THRESHOLD_KPH))
        {
            out->active[d] = 1;
            out->effect[d] = OUTPUT_RELEASE;
            out->reason[d] = REASON_CMD_APPLIED;
            out->unlock_allowed[d] = 1;
        }
        else
        {
            out->info_reason[d] = REASON_CMD_REJECTED_SPEED;
        }
    }
    return STATUS_OK;
}

Status_t DriverCommand_Evaluate(const RuleInput_t *in, RuleResult_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((in != NULL) && (out != NULL))
    {
        RuleResult_t cleared = {0};

        *out = cleared;
        const Command_t *cmd = &in->snapshot->cmd;

        if ((cmd->present == 1) && (cmd->validity == VALIDITY_INVALID))
        {
            out->info_reason[DOOR_LEFT] = REASON_INPUT_INVALID;
            out->info_reason[DOOR_RIGHT] = REASON_INPUT_INVALID;
        }
        else if (cmd->validity == VALIDITY_VALID)
        {
            int d;

            for (d = 0; d < DOOR_COUNT; d++)
            {
                DriverCommand_EvaluateDoor(in->snapshot, d, out);
            }
        }
        result = STATUS_OK;
    }
    return result;
}
