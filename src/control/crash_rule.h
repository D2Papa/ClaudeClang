/**
 * @file    crash_rule.h
 * @brief   충돌 규칙 평가 (ARCH-C-004, ARCH-I-004).
 * @trace   SSR-001, SSR-002, SSR-003
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
#ifndef CONTROL_CRASH_RULE_H
#define CONTROL_CRASH_RULE_H

#include "common/types.h"

/**
 * @brief 충돌 CONFIRMED이면 좌, 우 RELEASE를 요구하고 PENDING이면 이유 코드만 알린다.
 * @param[in]  in   규칙 입력 (NULL 불가)
 * @param[out] out  규칙 결과 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-010
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
Status_t Crash_Evaluate(const RuleInput_t *in, RuleResult_t *out);

#endif /* CONTROL_CRASH_RULE_H */
