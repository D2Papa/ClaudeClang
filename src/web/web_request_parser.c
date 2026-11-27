/**
 * @file    web_request_parser.c
 * @brief   요청 본문 해석 구현.
 * @trace   SRS-C-004, SRS-Q-003, SSR-007
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
#include "web/web_request_parser.h"
#include "common/params.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/** 필드 종류 */
typedef enum
{
    WEB_KIND_NUMBER = 0, /**< 실수 */
    WEB_KIND_BOOLEAN = 1, /**< 불리언(정수로 해석) */
    WEB_KIND_ENUM = 2    /**< 열거 식별자(정수로 해석) */
} WebKind_t;

/** 필드 표 항목 */
typedef struct
{
    const char *name; /**< 본문에서 쓰는 이름 */
    int kind;         /**< WebKind_t */
    size_t offset;    /**< RawInput_t 안의 RawValue_t 위치 */
} WebField_t;

/** 필드 수 */
#define WEB_FIELD_COUNT 16

/** 값 문자열의 최대 길이 */
#define WEB_VALUE_MAX 31

/** 본문 필드 표 (DOC-DDS-002 1장) */
static const WebField_t WEB_FIELDS[WEB_FIELD_COUNT] = {
    {"speed", WEB_KIND_NUMBER, offsetof(RawInput_t, speed)},
    {"gear", WEB_KIND_ENUM, offsetof(RawInput_t, gear)},
    {"source_timestamp", WEB_KIND_NUMBER, offsetof(RawInput_t, source_timestamp)},
    {"crash", WEB_KIND_ENUM, offsetof(RawInput_t, crash)},
    {"approach_left", WEB_KIND_BOOLEAN, offsetof(RawInput_t, approach[DOOR_LEFT])},
    {"approach_right", WEB_KIND_BOOLEAN, offsetof(RawInput_t, approach[DOOR_RIGHT])},
    {"fire", WEB_KIND_BOOLEAN, offsetof(RawInput_t, fire)},
    {"overtemp", WEB_KIND_BOOLEAN, offsetof(RawInput_t, overtemp)},
    {"adult", WEB_KIND_BOOLEAN, offsetof(RawInput_t, adult)},
    {"isofix_left", WEB_KIND_BOOLEAN, offsetof(RawInput_t, isofix[DOOR_LEFT])},
    {"isofix_right", WEB_KIND_BOOLEAN, offsetof(RawInput_t, isofix[DOOR_RIGHT])},
    {"ignition", WEB_KIND_BOOLEAN, offsetof(RawInput_t, ignition)},
    {"sensor_fault", WEB_KIND_BOOLEAN, offsetof(RawInput_t, sensor_fault)},
    {"cmd_side", WEB_KIND_ENUM, offsetof(RawInput_t, cmd_side)},
    {"cmd_action", WEB_KIND_ENUM, offsetof(RawInput_t, cmd_action)},
    {"cmd_source", WEB_KIND_ENUM, offsetof(RawInput_t, cmd_source)}};

/**
 * @brief 누락이 아닌 값의 RawValue_t를 만든다.
 * @param[in] kind  값 종류
 * @param[in] value  값
 * @return STATUS_OK
 * @trace DD-F-067
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
static RawValue_t Web_Number(double value)
{
    RawValue_t raw = {RAW_NUMBER, value, 0, 0};

    return raw;
}

/**
 * @brief 문자열 전체가 실수이면 1을 돌려준다.
 * @param[in]  text   문자열
 * @param[out] value  실수 값
 * @return 실수이면 1
 * @trace DD-F-068, SSR-007
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
static int Web_ParseNumber(const char *text, double *value)
{
    char *end = NULL;
    int ok = 0;

    *value = strtod(text, &end);
    if ((end != text) && (*end == '\0'))
    {
        ok = 1;
    }
    return ok;
}

/**
 * @brief 문자열 전체가 int 범위의 정수이면 1을 돌려준다.
 * @param[in]  text   문자열
 * @param[out] value  정수 값
 * @return 정수이면 1
 * @trace DD-F-069, SSR-007
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
static int Web_ParseInteger(const char *text, int *value)
{
    char *end = NULL;
    long parsed = strtol(text, &end, 10);
    int ok = 0;

    if ((end != text) && (*end == '\0') && (parsed >= INT_MIN) && (parsed <= INT_MAX))
    {
        *value = (int)parsed;
        ok = 1;
    }
    return ok;
}

/**
 * @brief 값 문자열을 종류에 맞게 RawValue_t로 만든다 (absent는 누락, 해석 실패는 형식 오류).
 * @param[in]  kind  WebKind_t
 * @param[in]  text  값 문자열
 * @param[out] out   원시 값
 * @return STATUS_OK
 * @trace DD-F-067
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
static Status_t Web_ParseValue(int kind, const char *text, RawValue_t *out)
{
    double number = 0.0;
    int integer = 0;
    RawValue_t raw = {RAW_OTHER, 0.0, 0, 0};

    if (strcmp(text, "absent") == 0)
    {
        raw.kind = RAW_ABSENT;
    }
    else if (kind == WEB_KIND_NUMBER)
    {
        if (Web_ParseNumber(text, &number) == 1)
        {
            raw = Web_Number(number);
        }
    }
    else if (Web_ParseInteger(text, &integer) == 1)
    {
        raw.kind = (kind == WEB_KIND_BOOLEAN) ? RAW_BOOLEAN : RAW_ENUM;
        raw.boolean = integer;
        raw.enum_id = integer;
    }
    *out = raw;
    return STATUS_OK;
}

/**
 * @brief 이름이 같은 필드를 표에서 찾아 값을 적용한다.
 * @param[inout] raw       제어 입력
 * @param[in]    name      이름 (종료 문자 없음)
 * @param[in]    name_len  이름 길이
 * @param[in]    value     값 문자열
 * @return 적용했으면 1, 이름이 없으면 0
 * @trace DD-F-066
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
static int Web_ApplyField(RawInput_t *raw, const char *name, size_t name_len, const char *value)
{
    int applied = 0;
    int i;

    for (i = 0; (i < WEB_FIELD_COUNT) && (applied == 0); i++)
    {
        if ((strlen(WEB_FIELDS[i].name) == name_len) && (strncmp(WEB_FIELDS[i].name, name, name_len) == 0))
        {
            RawValue_t *target = (RawValue_t *)((char *)raw + WEB_FIELDS[i].offset);

            Web_ParseValue(WEB_FIELDS[i].kind, value, target);
            applied = 1;
        }
    }
    return applied;
}

/**
 * @brief `이름=값` 한 쌍을 해석해 적용한다. `=`가 없거나 이름이 비었거나 값이 너무 길면 실패한다.
 * @param[inout] raw   제어 입력
 * @param[in]    pair  쌍의 시작
 * @param[in]    len   쌍의 길이
 * @return 성공이면 1
 * @trace DD-F-063
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
static int Web_ApplyPair(RawInput_t *raw, const char *pair, size_t len)
{
    const char *equals = memchr(pair, '=', len);
    int ok = 0;

    if ((equals != NULL) && (equals != pair))
    {
        size_t name_len = (size_t)(equals - pair);
        size_t value_len = len - name_len - 1U;

        if (value_len <= (size_t)WEB_VALUE_MAX)
        {
            char value[WEB_VALUE_MAX + 1];

            memcpy(value, equals + 1, value_len);
            value[value_len] = '\0';
            ok = Web_ApplyField(raw, pair, name_len, value);
        }
    }
    return ok;
}

/**
 * @brief 주기 번호 시각의 정상 입력을 만든다.
 * @param[in]  cycle  주기 번호
 * @param[out] raw    제어 입력
 * @return STATUS_OK
 * @trace DD-F-065
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
static Status_t Web_DefaultRaw(int cycle, RawInput_t *raw)
{
    RawInput_t clean = {0};
    RawValue_t zero = {RAW_BOOLEAN, 0.0, 0, 0};
    RawValue_t one = {RAW_BOOLEAN, 0.0, 1, 0};

    *raw = clean;
    raw->timestamp_s = (double)cycle * CYCLE_PERIOD_S;
    raw->speed = Web_Number(0.0);
    raw->gear.kind = RAW_ENUM;
    raw->gear.enum_id = 0;
    raw->source_timestamp = Web_Number(raw->timestamp_s);
    raw->crash.kind = RAW_ENUM;
    raw->crash.enum_id = 0;
    raw->approach[DOOR_LEFT] = zero;
    raw->approach[DOOR_RIGHT] = zero;
    raw->fire = zero;
    raw->overtemp = zero;
    raw->adult = zero;
    raw->isofix[DOOR_LEFT] = zero;
    raw->isofix[DOOR_RIGHT] = zero;
    raw->ignition = one;
    raw->sensor_fault = zero;
    return STATUS_OK;
}

int WebParser_ParseBody(const char *body, int cycle, RawInput_t *raw)
{
    int ok = 0;

    if ((body != NULL) && (raw != NULL))
    {
        const char *pos = body;

        ok = 1;
        Web_DefaultRaw(cycle, raw);
        while ((ok == 1) && (*pos != '\0'))
        {
            const char *amp = strchr(pos, '&');
            size_t len = (amp != NULL) ? (size_t)(amp - pos) : strlen(pos);

            ok = Web_ApplyPair(raw, pos, len);
            pos += len;
            if (*pos == '&')
            {
                pos += 1;
            }
        }
    }
    return ok;
}
