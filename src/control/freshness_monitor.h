/**
 * @file    freshness_monitor.h
 * @brief   시각 단조성과 차속 신선도 검사 (ARCH-C-002, ARCH-I-002).
 * @trace   SSR-008
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
#ifndef CONTROL_FRESHNESS_MONITOR_H
#define CONTROL_FRESHNESS_MONITOR_H

#include "common/types.h"

/** 신선도 검사 컨텍스트 */
typedef struct
{
    int initialized;       /**< Freshness_Init 호출 여부 */
    double previous_timestamp_s; /**< 직전 주기의 논리 시각 */
} Freshness_t;

/**
 * @brief 컨텍스트를 초기화한다.
 * @param[out] ctx  컨텍스트 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-006
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
Status_t Freshness_Init(Freshness_t *ctx);

/**
 * @brief 시각 단조성을 확인하고 차속이 200 ms 넘게 오래되었으면 차속 입력을 INVALID로 표시한다.
 * @param[inout] ctx   컨텍스트 (NULL 불가)
 * @param[inout] snap  검증된 입력 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터 / STATUS_ERR_STATE 미초기화 또는 시각 역행
 * @trace DD-F-007, SSR-008
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
Status_t Freshness_Apply(Freshness_t *ctx, Snapshot_t *snap);

#endif /* CONTROL_FRESHNESS_MONITOR_H */
