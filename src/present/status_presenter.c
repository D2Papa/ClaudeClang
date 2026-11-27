/**
 * @file    status_presenter.c
 * @brief   상태 표시 항목 구성과 JSON 직렬화 구현.
 * @trace   SRS-F-018, SRS-F-019, SRS-F-022, SRS-F-023
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
#include "present/status_presenter.h"
#include <stddef.h>
#include <string.h>

/** 출력 이름 표 (Output_t 순서) */
static const char *const OUTPUT_NAMES[2] = {"RELEASE", "LOCK"};

/** state 이름 표 (State_t 순서) */
static const char *const STATE_NAMES[4] = {"OFF", "NORMAL", "DEGRADED", "FAULT"};

/** 이유 이름 표 (ReasonCode_t 순서) */
static const char *const REASON_NAMES[17] = {
    "NONE",       "CMD_APPLIED",    "CMD_REJECTED_SPEED", "INPUT_INVALID",       "CRASH_CONFIRMED",
    "CRASH_PENDING", "FIRE",        "OVERTEMP",           "ADULT_PRESENT",       "IGNITION_OFF",
    "SENSOR_FAULT", "DEGRADED_STALE", "APPROACH_RISK_LOCK", "APPROACH_RISK_SUPPRESSED", "OVERRIDE",
    "ISOFIX_LOCK", "AUTO_LOCK"};

/** 입력 유효성 항목 이름 (DOC-DDS-001 비트 순서) */
static const char *const VALIDITY_NAMES[VALIDITY_ITEM_COUNT] = {
    "speed", "gear", "source_timestamp", "crash", "approach_left", "approach_right", "fire",
    "overtemp", "adult", "isofix_left", "isofix_right", "ignition", "sensor_fault"};

/**
 * @brief 이름 표에서 index의 이름을 찾는다. 범위 밖이면 "UNKNOWN".
 * @param[in]  table  이름 표
 * @param[in]  count  표 크기
 * @param[in]  index  찾을 위치
 * @param[out] name   이름
 * @return STATUS_OK
 * @trace DD-F-041
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
static Status_t Presenter_Name(const char *const *table, int count, int index, const char **name)
{
    *name = "UNKNOWN";
    if ((index >= 0) && (index < count))
    {
        *name = table[index];
    }
    return STATUS_OK;
}

/**
 * @brief 텍스트를 버퍼 끝에 덧붙인다.
 * @param[inout] buf  버퍼
 * @param[in]    cap  용량
 * @param[inout] pos  쓴 길이
 * @param[in]    text 덧붙일 문자열
 * @return 성공이면 1, 용량 부족이면 0
 * @trace DD-F-044
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
static int Presenter_AppendText(char *buf, size_t cap, size_t *pos, const char *text)
{
    size_t length = strlen(text);
    int ok = 0;

    if ((*pos + length + 1U) <= cap)
    {
        memcpy(buf + *pos, text, length + 1U);
        *pos += length;
        ok = 1;
    }
    return ok;
}

/**
 * @brief JSON 키와 문자열 값 한 쌍을 덧붙인다.
 * @param[inout] buf    버퍼
 * @param[in]    cap    용량
 * @param[inout] pos    쓴 길이
 * @param[in]    key    키
 * @param[in]    value  값
 * @return 성공이면 1
 * @trace DD-F-043
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
static int Presenter_AppendField(char *buf, size_t cap, size_t *pos, const char *key, const char *value)
{
    int ok = Presenter_AppendText(buf, cap, pos, "\"");

    ok &= Presenter_AppendText(buf, cap, pos, key);
    ok &= Presenter_AppendText(buf, cap, pos, "\":\"");
    ok &= Presenter_AppendText(buf, cap, pos, value);
    ok &= Presenter_AppendText(buf, cap, pos, "\"");
    return ok;
}

/**
 * @brief 입력 유효성 13항목을 JSON 객체 내용으로 덧붙인다.
 * @param[in]    st   표시 항목
 * @param[inout] buf  버퍼
 * @param[in]    cap  용량
 * @param[inout] pos  쓴 길이
 * @return 성공이면 1
 * @trace DD-F-045
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
static int Presenter_WriteValidity(const PresentedStatus_t *st, char *buf, size_t cap, size_t *pos)
{
    int ok = 1;
    int i;

    for (i = 0; i < VALIDITY_ITEM_COUNT; i++)
    {
        if (i > 0)
        {
            ok &= Presenter_AppendText(buf, cap, pos, ",");
        }
        ok &= Presenter_AppendField(buf, cap, pos, VALIDITY_NAMES[i], st->validity[i]);
    }
    return ok;
}

/**
 * @brief 표시 항목 전체를 고정 순서의 JSON으로 쓴다.
 * @param[in]  st   표시 항목
 * @param[out] buf  버퍼
 * @param[in]  cap  용량
 * @return 성공이면 1, 용량 부족이면 0
 * @trace DD-F-043
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
static int Presenter_Write(const PresentedStatus_t *st, char *buf, size_t cap)
{
    size_t pos = 0U;
    int ok = Presenter_AppendText(buf, cap, &pos, "{");

    ok &= Presenter_AppendField(buf, cap, &pos, "lock_left", st->lock_left);
    ok &= Presenter_AppendText(buf, cap, &pos, ",");
    ok &= Presenter_AppendField(buf, cap, &pos, "lock_right", st->lock_right);
    ok &= Presenter_AppendText(buf, cap, &pos, ",");
    ok &= Presenter_AppendField(buf, cap, &pos, "state", st->state);
    ok &= Presenter_AppendText(buf, cap, &pos, ",");
    ok &= Presenter_AppendField(buf, cap, &pos, "priority_reason", st->priority_reason);
    ok &= Presenter_AppendText(buf, cap, &pos, ",");
    ok &= Presenter_AppendField(buf, cap, &pos, "reason_code", st->reason_code);
    ok &= Presenter_AppendText(buf, cap, &pos, ",");
    ok &= Presenter_AppendField(buf, cap, &pos, "warning_code", st->warning_code);
    ok &= Presenter_AppendText(buf, cap, &pos, ",\"input_validity\":{");
    ok &= Presenter_WriteValidity(st, buf, cap, &pos);
    ok &= Presenter_AppendText(buf, cap, &pos, "}}");
    return ok;
}

Status_t Presenter_Build(const CycleResult_t *res, PresentedStatus_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((res != NULL) && (out != NULL))
    {
        int i;

        Presenter_Name(OUTPUT_NAMES, 2, (int)res->output[DOOR_LEFT], &out->lock_left);
        Presenter_Name(OUTPUT_NAMES, 2, (int)res->output[DOOR_RIGHT], &out->lock_right);
        Presenter_Name(STATE_NAMES, 4, (int)res->state, &out->state);
        Presenter_Name(REASON_NAMES, 17, (int)res->priority_reason_summary, &out->priority_reason);
        Presenter_Name(REASON_NAMES, 17, (int)res->reason_code_summary, &out->reason_code);
        out->warning_code = "NONE";
        if (res->warning_code == 1)
        {
            out->warning_code = "W-001";
        }
        for (i = 0; i < VALIDITY_ITEM_COUNT; i++)
        {
            out->validity[i] = "INVALID";
            if (((res->input_validity >> i) & 1U) == 1U)
            {
                out->validity[i] = "VALID";
            }
        }
        result = STATUS_OK;
    }
    return result;
}

Status_t Presenter_Serialize(const PresentedStatus_t *st, char *buf, size_t cap, int *http_status)
{
    Status_t result = STATUS_ERR_PARAM;

    if (http_status != NULL)
    {
        *http_status = 500;
        if ((st != NULL) && (buf != NULL) && (cap > 0U))
        {
            if (Presenter_Write(st, buf, cap) == 1)
            {
                *http_status = 200;
            }
            else
            {
                buf[0] = '\0';
            }
            result = STATUS_OK;
        }
    }
    return result;
}
