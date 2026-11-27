/**
 * @file    auto_lock_rule.h
 * @brief   자동 잠금 규칙 평가 (ARCH-C-010, ARCH-I-004).
 * @trace   SRS-F-004, SRS-L-004
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
#ifndef CONTROL_AUTO_LOCK_RULE_H
#define CONTROL_AUTO_LOCK_RULE_H

#include "common/types.h"

/**
 * @brief 유효 차속이 3.0 km/h 이상이면 좌, 우 LOCK을 요구한다.
 * @param[in]  in   규칙 입력 (NULL 불가)
 * @param[out] out  규칙 결과 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-017
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
Status_t AutoLock_Evaluate(const RuleInput_t *in, RuleResult_t *out);

#endif /* CONTROL_AUTO_LOCK_RULE_H */
