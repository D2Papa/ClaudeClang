/**
 * @file    status_presenter.h
 * @brief   상태 표시 항목 구성과 JSON 직렬화 (ARCH-C-014, ARCH-I-008).
 * @trace   SRS-F-018, SRS-F-019, SRS-F-022, SRS-F-023, SRS-Q-003
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
#ifndef PRESENT_STATUS_PRESENTER_H
#define PRESENT_STATUS_PRESENTER_H

#include "common/types.h"
#include <stddef.h>

/** 입력 유효성 표시 항목 수 */
#define VALIDITY_ITEM_COUNT 13

/** 표시용 상태 (문자열은 정적 상수를 가리킨다) */
typedef struct
{
    const char *lock_left;       /**< 좌 출력 */
    const char *lock_right;      /**< 우 출력 */
    const char *state;           /**< state */
    const char *priority_reason; /**< 요약 priority_reason */
    const char *reason_code;     /**< 요약 reason_code */
    const char *warning_code;    /**< "NONE" 또는 "W-001" */
    const char *validity[VALIDITY_ITEM_COUNT]; /**< 입력 유효성("VALID"/"INVALID") */
} PresentedStatus_t;

/**
 * @brief 제어 결과를 표시 항목으로 만든다.
 * @param[in]  res  제어 결과 (NULL 불가)
 * @param[out] out  표시 항목 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-040
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
Status_t Presenter_Build(const CycleResult_t *res, PresentedStatus_t *out);

/**
 * @brief 표시 항목을 JSON 문자열로 직렬화한다. 용량이 모자라면 buf를 빈 문자열로 만들고 http_status를 500으로 한다.
 * @param[in]  st           표시 항목
 * @param[out] buf          출력 버퍼
 * @param[in]  cap          버퍼 용량 (1 이상)
 * @param[out] http_status  200 또는 500 (NULL 불가)
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 인자 오류(http_status가 널이 아니면 500)
 * @trace DD-F-042, SRS-F-023
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
Status_t Presenter_Serialize(const PresentedStatus_t *st, char *buf, size_t cap, int *http_status);

#endif /* PRESENT_STATUS_PRESENTER_H */
