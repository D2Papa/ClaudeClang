/**
 * @file    state_manager.h
 * @brief   state와 경고코드 결정 (ARCH-C-003, ARCH-I-003).
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
#ifndef CONTROL_STATE_MANAGER_H
#define CONTROL_STATE_MANAGER_H

#include "common/types.h"

/**
 * @brief 입력 유효성으로 state(OFF, FAULT, DEGRADED, NORMAL)와 경고코드를 결정한다.
 * @param[in]  snap          검증된 입력 (NULL 불가)
 * @param[out] state         결정된 state (NULL 불가)
 * @param[out] warning_code  0 또는 1(W-001) (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-008
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
Status_t State_Determine(const Snapshot_t *snap, State_t *state, int *warning_code);

#endif /* CONTROL_STATE_MANAGER_H */
