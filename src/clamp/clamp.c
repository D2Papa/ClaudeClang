/**
 * @file    clamp.c
 * @brief   정수 범위 제한(clamp) 모듈 구현.
 * @note    파이프라인 검증용 샘플이다. 실제 개발 시작 시 삭제한다.
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
#include "clamp/clamp.h"

#include <stddef.h>

/**
 * @brief 값을 [min, max] 범위로 제한하여 out에 저장한다.
 * @param[in]  value  제한할 값
 * @param[in]  min    하한 (max 이하여야 한다)
 * @param[in]  max    상한
 * @param[out] out    결과 저장 위치 (NULL 불가)
 * @return STATUS_OK 성공 / STATUS_ERR_PARAM out이 NULL이거나 min이 max보다 큼
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
Status_t Clamp_Int(int value, int min, int max, int *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((out != NULL) && (min <= max))
    {
        if (value < min)
        {
            *out = min;
        }
        else if (value > max)
        {
            *out = max;
        }
        else
        {
            *out = value;
        }
        result = STATUS_OK;
    }

    return result;
}
