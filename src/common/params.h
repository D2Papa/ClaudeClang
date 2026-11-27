/**
 * @file    params.h
 * @brief   제어 코어의 읽기 전용 수치 상수 (SRS-L 항목).
 * @trace   SRS-L-004, SRS-L-005, SRS-L-006, SRS-L-007, SSR-008, ARCH-D-04
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
#ifndef COMMON_PARAMS_H
#define COMMON_PARAMS_H

#define CYCLE_PERIOD_S 0.01        /**< 평가주기 10 ms (SRS-C-005) */
#define SPEED_THRESHOLD_KPH 3.0    /**< 자동 잠금과 해제 허용 임계값 (SRS-L-004) */
#define SPEED_MIN_KPH 0.0          /**< 차속 하한 (SRS-L-007) */
#define SPEED_MAX_KPH 300.0        /**< 차속 상한 (SRS-L-007) */
#define FRESHNESS_LIMIT_S 0.2      /**< 신선도 한계 200 ms (SSR-008) */
#define OVERRIDE_WINDOW_CYCLES 1000U /**< override 허용 주기 수 (SRS-L-005) */
#define HISTORY_CAPACITY 100U      /**< 이력 보존 건수 (SRS-L-006) */

#endif /* COMMON_PARAMS_H */
