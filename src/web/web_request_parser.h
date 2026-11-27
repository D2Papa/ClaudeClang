/**
 * @file    web_request_parser.h
 * @brief   요청 본문(이름=값&...)을 제어 입력으로 해석 (ARCH-C-016 모듈).
 * @trace   SRS-C-004, SRS-Q-003, SSR-007
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
#ifndef WEB_WEB_REQUEST_PARSER_H
#define WEB_WEB_REQUEST_PARSER_H

#include "common/types.h"

/**
 * @brief 본문을 해석해 입력을 만든다. 기본은 정상 입력이며 본문의 이름=값이 해당 필드를 바꾼다. 이름이 표에 없거나 구문이 틀리면 0을 돌려준다. 값의 범위 검증은 하지 않는다(InputValidator가 한다).
 * @param[in]  body   본문 (NULL 불가, 비어 있으면 기본 입력)
 * @param[in]  cycle  주기 번호 (시각 = cycle x 10 ms)
 * @param[out] raw    제어 입력
 * @return 성공이면 1, 인자가 널이거나 구문·이름 오류이면 0
 * @trace DD-F-063
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
int WebParser_ParseBody(const char *body, int cycle, RawInput_t *raw);

#endif /* WEB_WEB_REQUEST_PARSER_H */
