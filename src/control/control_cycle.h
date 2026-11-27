/**
 * @file    control_cycle.h
 * @brief   평가주기 실행 (ARCH-C-013, ARCH-I-007).
 * @trace   SRS-Q-001, SRS-C-002, SRS-C-005, SSR-002
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
#ifndef CONTROL_CONTROL_CYCLE_H
#define CONTROL_CONTROL_CYCLE_H

#include "common/types.h"
#include "control/freshness_monitor.h"
#include "control/override_manager.h"

/** 제어 컨텍스트 */
typedef struct
{
    int initialized;                    /**< Control_Init 호출 여부 */
    Freshness_t freshness;              /**< 신선도 컨텍스트 */
    Override_t override_ctx;            /**< override 컨텍스트 */
    Output_t previous[DOOR_COUNT];      /**< 직전 출력 */
    int prev_suppressed[DOOR_COUNT];    /**< 직전 주기 unlock 억제 */
    uint32_t cycle;                     /**< 주기 번호 */
} Control_t;

/**
 * @brief 제어 컨텍스트를 초기화한다. 직전 출력은 RELEASE이다.
 * @param[out] ctx  컨텍스트 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-025
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
Status_t Control_Init(Control_t *ctx);

/**
 * @brief 한 평가주기를 실행해 최종 결과를 만든다. 오류가 나면 컨텍스트의 직전 출력은 바뀌지 않는다.
 * @param[inout] ctx  컨텍스트 (NULL 불가, 초기화됨)
 * @param[in]    raw  원시 입력 (NULL 불가)
 * @param[out]   out  최종 결과 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터 / STATUS_ERR_STATE 미초기화 또는 시각 역행
 * @trace DD-F-026
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
Status_t Control_Step(Control_t *ctx, const RawInput_t *raw, CycleResult_t *out);

#endif /* CONTROL_CONTROL_CYCLE_H */
