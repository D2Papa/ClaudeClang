/**
 * @file    approach_risk_rule.h
 * @brief   접근위험 규칙 평가 (ARCH-C-007, ARCH-I-004).
 * @trace   SSR-004, SSR-005, SSR-006
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
#ifndef CONTROL_APPROACH_RISK_RULE_H
#define CONTROL_APPROACH_RISK_RULE_H

#include "common/types.h"

/**
 * @brief 접근위험이 유효한 TRUE인 도어에 LOCK을 요구하고 해제 요청 억제 여부를 알린다.
 * @param[in]  in   규칙 입력 (NULL 불가)
 * @param[out] out  규칙 결과 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-013
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
Status_t ApproachRisk_Evaluate(const RuleInput_t *in, RuleResult_t *out);

#endif /* CONTROL_APPROACH_RISK_RULE_H */
