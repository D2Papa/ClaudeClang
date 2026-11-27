/**
 * @file    driver_command_rule.h
 * @brief   운전자 명령 규칙 평가 (ARCH-C-009, ARCH-I-004).
 * @trace   SRS-F-001, SRS-F-002, SRS-F-003, SRS-L-004
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
#ifndef CONTROL_DRIVER_COMMAND_RULE_H
#define CONTROL_DRIVER_COMMAND_RULE_H

#include "common/types.h"

/**
 * @brief 운전자 명령을 속도 조건으로 허용 여부를 정해 지정 도어의 출력으로 요구한다.
 * @param[in]  in   규칙 입력 (NULL 불가)
 * @param[out] out  규칙 결과 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-015
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
Status_t DriverCommand_Evaluate(const RuleInput_t *in, RuleResult_t *out);

#endif /* CONTROL_DRIVER_COMMAND_RULE_H */
