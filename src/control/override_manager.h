/**
 * @file    override_manager.h
 * @brief   override 성립, 지속, 종료 추적 (ARCH-C-011, ARCH-I-005).
 * @trace   SRS-F-007, SRS-F-029, SRS-F-030, SRS-L-005
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
#ifndef CONTROL_OVERRIDE_MANAGER_H
#define CONTROL_OVERRIDE_MANAGER_H

#include "common/types.h"

/** override 갱신 입력 */
typedef struct
{
    uint32_t cycle;                  /**< 현재 주기 번호 */
    State_t state;                   /**< 현재 state */
    int risk_true[DOOR_COUNT];       /**< 접근위험이 유효한 TRUE */
    int speed_ge[DOOR_COUNT];        /**< 유효 차속이 3.0 km/h 이상 */
    int unlock_allowed[DOOR_COUNT];  /**< 허용된 unlock 요청 */
    int lock_requested[DOOR_COUNT];  /**< lock 요청 */
    int prev_suppressed[DOOR_COUNT]; /**< 직전 주기에 unlock이 억제됨 */
} OverrideInput_t;

/** override 컨텍스트 */
typedef struct
{
    uint32_t suppressed_cycle[DOOR_COUNT]; /**< 억제 기록 주기 */
    int suppressed_valid[DOOR_COUNT];      /**< 억제 기록 유효 여부 */
    int active[DOOR_COUNT];                /**< override 지속 여부 */
} Override_t;

/**
 * @brief 컨텍스트를 초기화한다.
 * @param[out] ctx  컨텍스트 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-018
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
Status_t Override_Init(Override_t *ctx);

/**
 * @brief 도어별 override 성립, 지속, 종료를 갱신한다.
 * @param[inout] ctx  컨텍스트 (NULL 불가)
 * @param[in]    in   갱신 입력 (NULL 불가)
 * @param[out]   out  도어별 override 상태 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-019
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
Status_t Override_Update(Override_t *ctx, const OverrideInput_t *in, OverrideStatus_t *out);

#endif /* CONTROL_OVERRIDE_MANAGER_H */
