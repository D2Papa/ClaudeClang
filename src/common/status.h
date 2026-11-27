/**
 * @file    status.h
 * @brief   프로젝트 공통 함수 실행 결과 코드.
 * @note    파이프라인 검증용 샘플이다. 실제 개발 시작 시 요구사항·설계에 맞춰 교체하거나 삭제한다.
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
#ifndef COMMON_STATUS_H
#define COMMON_STATUS_H

/**
 * @brief 함수 실행 결과 코드.
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
typedef enum
{
    STATUS_OK = 0,        /**< 정상 실행 */
    STATUS_ERR_PARAM = 1  /**< 잘못된 입력 */
} Status_t;

#endif /* COMMON_STATUS_H */
