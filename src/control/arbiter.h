/**
 * @file    arbiter.h
 * @brief   우선순위 중재 (ARCH-C-012, ARCH-I-006).
 * @trace   SRS-F-017, SRS-F-028, SRS-F-031, SSR-010, SSR-011, SSR-013, SSR-016
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
#ifndef CONTROL_ARBITER_H
#define CONTROL_ARBITER_H

#include "common/types.h"

/**
 * @brief 규칙 결과를 순위로 합쳐 도어별 출력과 이유를 결정한다. state에 따라 순위 4~7(OFF)과 순위 6(DEGRADED, FAULT)을 제외한다.
 * @param[in]  in   중재 입력 (NULL 불가)
 * @param[out] out  중재 결과 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-021
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
Status_t Arbiter_Resolve(const ArbiterInput_t *in, Decision_t *out);

#endif /* CONTROL_ARBITER_H */
