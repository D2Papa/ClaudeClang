/**
 * @file    input_validator.c
 * @brief   원시 입력 검증과 정규화 구현.
 * @trace   SSR-007, SRS-F-001, SRS-L-007, SRS-C-003
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
#include "control/input_validator.h"
#include "common/params.h"
#include <stddef.h>

/** 시각 입력 상한 (초) */
#define TIMESTAMP_MAX_S 1.0e12

/**
 * @brief 불리언 필드를 검증한다.
 * @param[in]  in   원시 값
 * @param[out] out  검증 결과
 * @return STATUS_OK
 * @trace DD-F-002
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
static Status_t Validate_Flag(const RawValue_t *in, Flag_t *out)
{
    out->value = 0;
    out->validity = VALIDITY_INVALID;
    if ((in->kind == RAW_BOOLEAN) && ((in->boolean == 0) || (in->boolean == 1)))
    {
        out->value = in->boolean;
        out->validity = VALIDITY_VALID;
    }
    return STATUS_OK;
}

/**
 * @brief 실수 필드를 범위 검사한다 (NaN은 비교가 거짓이라 거절된다).
 * @param[in]  in   원시 값
 * @param[in]  min  하한
 * @param[in]  max  상한
 * @param[out] out  검증 결과
 * @return STATUS_OK
 * @trace DD-F-003
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
static Status_t Validate_Number(const RawValue_t *in, double min, double max, Num_t *out)
{
    out->value = 0.0;
    out->validity = VALIDITY_INVALID;
    if ((in->kind == RAW_NUMBER) && (in->number >= min) && (in->number <= max))
    {
        out->value = in->number;
        out->validity = VALIDITY_VALID;
    }
    return STATUS_OK;
}

/**
 * @brief 열거 필드를 값 집합으로 검사한다.
 * @param[in]  in      원시 값
 * @param[in]  max_id  최대 식별자
 * @param[out] out     검증 결과
 * @return STATUS_OK
 * @trace DD-F-004
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
static Status_t Validate_Enum(const RawValue_t *in, int max_id, Enum_t *out)
{
    out->value = 0;
    out->validity = VALIDITY_INVALID;
    if ((in->kind == RAW_ENUM) && (in->enum_id >= 0) && (in->enum_id <= max_id))
    {
        out->value = in->enum_id;
        out->validity = VALIDITY_VALID;
    }
    return STATUS_OK;
}

/**
 * @brief 명령 필드가 하나라도 있으면 1을 돌려준다.
 * @param[in] raw  원시 입력
 * @return 있으면 1
 * @trace SRS-F-001
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
static int Command_AnyPresent(const RawInput_t *raw)
{
    int result = 0;

    if ((raw->cmd_side.kind != RAW_ABSENT) || (raw->cmd_action.kind != RAW_ABSENT) ||
        (raw->cmd_source.kind != RAW_ABSENT))
    {
        result = 1;
    }
    return result;
}

/**
 * @brief 명령 side가 지정한 도어에 해당하면 1을 돌려준다.
 * @param[in] side  명령 side
 * @param[in] door  도어에 대응하는 side 값
 * @return 해당하면 1
 * @trace SRS-F-002
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
static int Command_Targets(int side, int door_side)
{
    int result = 0;

    if ((side == SIDE_ALL) || (side == door_side))
    {
        result = 1;
    }
    return result;
}

/**
 * @brief 운전자 명령 필드 세 개를 검증하고 지정 도어를 계산한다.
 * @param[in]  raw  원시 입력
 * @param[out] out  검증된 명령
 * @return STATUS_OK
 * @trace DD-F-005
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
static Status_t Validate_Command(const RawInput_t *raw, Command_t *out)
{
    Enum_t side;
    Enum_t action;
    Enum_t source;
    Command_t cleared = {0};

    *out = cleared;
    out->present = Command_AnyPresent(raw);
    /* 반환값을 쓰지 않는 호출: 인자가 널이 아님이 이 함수에서 보장되어 오류를 반환하지 않는다 (SRC-11). */
    Validate_Enum(&raw->cmd_side, SIDE_MAX, &side);
    Validate_Enum(&raw->cmd_action, ACTION_MAX, &action);
    Validate_Enum(&raw->cmd_source, SOURCE_MAX, &source);
    if ((side.validity == VALIDITY_VALID) && (action.validity == VALIDITY_VALID) && (source.validity == VALIDITY_VALID))
    {
        out->validity = VALIDITY_VALID;
        out->side = side.value;
        out->action = action.value;
        out->source = source.value;
        out->target[DOOR_LEFT] = Command_Targets(side.value, SIDE_LEFT);
        out->target[DOOR_RIGHT] = Command_Targets(side.value, SIDE_RIGHT);
    }
    return STATUS_OK;
}

Status_t Validator_Normalize(const RawInput_t *raw, Snapshot_t *snap)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((raw != NULL) && (snap != NULL))
    {
        snap->timestamp_s = raw->timestamp_s;
        /* 반환값을 쓰지 않는 호출: 인자가 널이 아님이 이 함수에서 보장되어 오류를 반환하지 않는다 (SRC-11). */
        Validate_Number(&raw->speed, SPEED_MIN_KPH, SPEED_MAX_KPH, &snap->speed);
        Validate_Enum(&raw->gear, GEAR_MAX, &snap->gear);
        Validate_Number(&raw->source_timestamp, 0.0, TIMESTAMP_MAX_S, &snap->source_timestamp);
        Validate_Enum(&raw->crash, CRASH_MAX, &snap->crash);
        Validate_Flag(&raw->approach[DOOR_LEFT], &snap->approach[DOOR_LEFT]);
        Validate_Flag(&raw->approach[DOOR_RIGHT], &snap->approach[DOOR_RIGHT]);
        Validate_Flag(&raw->fire, &snap->fire);
        Validate_Flag(&raw->overtemp, &snap->overtemp);
        Validate_Flag(&raw->adult, &snap->adult);
        Validate_Flag(&raw->isofix[DOOR_LEFT], &snap->isofix[DOOR_LEFT]);
        Validate_Flag(&raw->isofix[DOOR_RIGHT], &snap->isofix[DOOR_RIGHT]);
        Validate_Flag(&raw->ignition, &snap->ignition);
        Validate_Flag(&raw->sensor_fault, &snap->sensor_fault);
        Validate_Command(raw, &snap->cmd);
        result = STATUS_OK;
    }
    return result;
}
