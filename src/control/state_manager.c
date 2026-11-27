/**
 * @file    state_manager.c
 * @brief   state와 경고코드 결정 구현.
 * @trace   SSR-009, SSR-012, SSR-014, SSR-015, SRS-F-016, SRS-F-027
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
#include "control/state_manager.h"
#include <stddef.h>

/**
 * @brief 무효이면 1, 유효하면 0을 돌려준다.
 * @param[in] validity  유효성
 * @return 무효 개수(0 또는 1)
 * @trace SSR-009
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
static int State_InvalidCount(Validity_t validity)
{
    int count = 0;

    if (validity == VALIDITY_INVALID)
    {
        count = 1;
    }
    return count;
}

/**
 * @brief 플래그가 유효하고 지정한 값이면 1을 돌려준다.
 * @param[in] flag   검증된 불리언
 * @param[in] value  비교할 값
 * @return 일치하면 1
 * @trace SSR-015
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
static int State_FlagIs(const Flag_t *flag, int value)
{
    int result = 0;

    if ((flag->validity == VALIDITY_VALID) && (flag->value == value))
    {
        result = 1;
    }
    return result;
}

/**
 * @brief 필수 입력 중 INVALID인 것의 개수를 센다 (gear는 제외).
 * @param[in] snap  검증된 입력
 * @return INVALID 개수
 * @trace SSR-008, SSR-009
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
static int State_CountInvalidRequired(const Snapshot_t *snap)
{
    int count = 0;

    count += State_InvalidCount(snap->speed.validity);
    count += State_InvalidCount(snap->source_timestamp.validity);
    count += State_InvalidCount(snap->crash.validity);
    count += State_InvalidCount(snap->approach[DOOR_LEFT].validity);
    count += State_InvalidCount(snap->approach[DOOR_RIGHT].validity);
    count += State_InvalidCount(snap->fire.validity);
    count += State_InvalidCount(snap->overtemp.validity);
    count += State_InvalidCount(snap->adult.validity);
    count += State_InvalidCount(snap->isofix[DOOR_LEFT].validity);
    count += State_InvalidCount(snap->isofix[DOOR_RIGHT].validity);
    count += State_InvalidCount(snap->ignition.validity);
    count += State_InvalidCount(snap->sensor_fault.validity);
    return count;
}

Status_t State_Determine(const Snapshot_t *snap, State_t *state, int *warning_code)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((snap != NULL) && (state != NULL) && (warning_code != NULL))
    {
        int off = State_FlagIs(&snap->ignition, 0);
        int fault = State_FlagIs(&snap->sensor_fault, 1);

        *state = STATE_NORMAL;
        if (off == 1)
        {
            *state = STATE_OFF;
        }
        else if (fault == 1)
        {
            *state = STATE_FAULT;
        }
        else if (State_CountInvalidRequired(snap) > 0)
        {
            *state = STATE_DEGRADED;
        }
        *warning_code = fault;
        result = STATUS_OK;
    }
    return result;
}
