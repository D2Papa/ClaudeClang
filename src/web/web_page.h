/**
 * @file    web_page.h
 * @brief   시험 화면(HTML) 제공 (ARCH-C-016 모듈, ARCH-I-010 내부).
 * @trace   SRS-C-004
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
#ifndef WEB_WEB_PAGE_H
#define WEB_WEB_PAGE_H

#include "common/types.h"
#include <stddef.h>

/**
 * @brief 시험 화면 HTML과 길이를 돌려준다.
 * @param[out] html    HTML 문자열 포인터
 * @param[out] length  문자열 길이
 * @return STATUS_OK 정상 / STATUS_ERR_PARAM 널 포인터
 * @trace DD-F-061
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
Status_t WebPage_Get(const char **html, size_t *length);

#endif /* WEB_WEB_PAGE_H */
