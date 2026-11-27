/**
 * @file    input_validator.h
 * @brief   원시 입력 검증과 정규화 (ARCH-C-001, ARCH-I-001).
 * @trace   SSR-007, SRS-F-001, SRS-Q-003, SRS-L-007, SRS-C-003
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
#ifndef CONTROL_INPUT_VALIDATOR_H
#define CONTROL_INPUT_VALIDATOR_H

#include "common/types.h"

/**
 * @brief 원시 입력을 검증해 VehicleSnapshot을 만든다. 무효 필드는 INVALID로 표시한다.
 * @param[in]  raw   원시 입력 (NULL 불가)
 * @param[out] snap  정규화된 입력 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-001
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
Status_t Validator_Normalize(const RawInput_t *raw, Snapshot_t *snap);

#endif /* CONTROL_INPUT_VALIDATOR_H */
