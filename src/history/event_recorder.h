/**
 * @file    event_recorder.h
 * @brief   결정 이벤트 기록과 조회 (ARCH-C-015, ARCH-I-009).
 * @trace   SRS-F-020, SRS-F-021, SRS-F-024, SRS-F-025, SRS-F-026, SRS-L-006, SRS-Q-002
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
#ifndef HISTORY_EVENT_RECORDER_H
#define HISTORY_EVENT_RECORDER_H

#include "common/types.h"
#include "common/params.h"
#include <stddef.h>
#include <stdint.h>

/** 이벤트 한 건 (허용 필드 6개) */
typedef struct
{
    uint64_t id;          /**< 고유 이벤트 ID (1부터) */
    double timestamp_s;   /**< 논리 시각 */
    Output_t lock_left;   /**< 좌 출력 */
    Output_t lock_right;  /**< 우 출력 */
    State_t state;        /**< state */
    ReasonCode_t reason_code; /**< 요약 reason_code */
} EventEntry_t;

/** 이력 컨텍스트 (메모리 전용) */
typedef struct
{
    EventEntry_t entries[HISTORY_CAPACITY]; /**< 순환 버퍼 */
    size_t head;                 /**< 다음 기록 위치 */
    size_t count;                /**< 보존 건수 */
    uint64_t next_id;            /**< 다음 이벤트 ID */
    int initialized;             /**< Event_Init 호출 여부 */
    Output_t prev_output[DOOR_COUNT]; /**< 직전 기록 기준 출력 */
    State_t prev_state;          /**< 직전 기록 기준 state */
    ReasonCode_t prev_reason;    /**< 직전 기록 기준 요약 reason_code */
} Event_t;

/**
 * @brief 이력 컨텍스트를 초기화한다.
 * @param[out] ctx  컨텍스트 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-046
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
Status_t Event_Init(Event_t *ctx);

/**
 * @brief 직전 기록과 (출력, state, 요약 reason_code)가 달라졌으면 이벤트 한 건을 기록한다.
 * @param[inout] ctx          컨텍스트
 * @param[in]    res          제어 결과
 * @param[in]    timestamp_s  논리 시각
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터 / STATUS_ERR_STATE 미초기화
 * @trace DD-F-047
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
Status_t Event_Record(Event_t *ctx, const CycleResult_t *res, double timestamp_s);

/**
 * @brief 보존된 이벤트를 오래된 순서로 복사한다.
 * @param[in]  ctx    컨텍스트
 * @param[out] out    복사할 배열
 * @param[in]  cap    배열 크기 (보존 건수 이상)
 * @param[out] count  복사한 건수
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터 또는 cap 부족 / STATUS_ERR_STATE 미초기화
 * @trace DD-F-050
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
Status_t Event_Query(const Event_t *ctx, EventEntry_t *out, size_t cap, size_t *count);

#endif /* HISTORY_EVENT_RECORDER_H */
