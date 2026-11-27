# DDS-iter2 — 표시·이력·Web 상세 설계서 (Iter-2)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-DDS-002 |
| 버전 | 0.5 (4차 설계 리뷰 반려 조치본) |
| 상태 | 승인 (aspice-reviewer 10차, 2026-11-27). 입력 SWE.1은 12차 재리뷰에서 승인 |
| 작성 | Claude Sonnet 5.5, 2026-11-27, 요청자 KYLee |
| 입력 | DOC-ARC-002 v0.2, DOC-DDS-001 v0.2 (공통 규칙), DOC-SRS-001 v1.0 |

- **인터페이스 정의 기준 수준**: 다른 모듈이 호출하는 함수가 하나라도 있는 모듈은 인터페이스(DD-I)를 정의한다. DD-M-014~016은 ARCH-I-008~010과 1:1이고, web_handler가 호출하는 DD-M-018, 019는 같은 컴포넌트(ARCH-C-016) 내부 인터페이스 DD-I-011, 012를 가진다. `web_main.c`(DD-M-017)는 호출되는 함수가 없는 진입점이라 기준 미만이다.
- 공통 규칙은 DOC-DDS-001과 같다: 모든 함수는 `Status_t` 또는 정수를 반환하고 `void`가 없으며 반환 1회, CCN 10 이하, 80라인 이하, 전역 가변 데이터 없음. 문자열 이름은 반환값이 아니라 출력 인자(`const char **`)로 돌려준다.
- 전제: DOC-ARC-002 ARCH-D-08에 따라 Iter-1의 `CycleResult_t`에 `priority_reason_summary`, `reason_code_summary`를 추가한다 (DOC-DDS-001 DD-F-028 개정).

## 1. 타입과 상수

| 이름 | 정의 |
|------|------|
| VALIDITY_ITEM_COUNT | 13 (입력 유효성 항목 수, `status_presenter.h`) |
| WEB_BODY_CAPACITY | 16384 (응답 본문 버퍼 바이트, `web_handler.h`. 이력 100건 JSON도 담을 수 있는 크기) |
| WEB_DEFAULT_PORT | 8081 (`web_handler.h`, 어댑터의 기본 포트) |
| WebRoute_t | ROUTE_PAGE=0, ROUTE_STATUS=1, ROUTE_HISTORY=2, ROUTE_STEP=3, ROUTE_RESET=4, ROUTE_COUNT=5 |
| PresentedStatus_t | lock_left, lock_right, state, priority_reason, reason_code, warning_code(문자열 포인터), validity[VALIDITY_ITEM_COUNT](문자열 포인터) |
| EventEntry_t | id(uint64_t), timestamp_s(double), lock_left, lock_right(Output_t), state(State_t), reason_code(ReasonCode_t) |
| Event_t | entries[HISTORY_CAPACITY], head, count, next_id(uint64_t), initialized, prev_output[2], prev_state, prev_reason(ReasonCode_t, 요약 값) |
| WebRequest_t | method, path, body (모두 널이 아닌 `const char *`, 본문이 없으면 `""`) |
| WebResponse_t | status(int), content_type(`const char *`), body(`char[WEB_BODY_CAPACITY]`) |
| Web_t | initialized, control(Control_t), events(Event_t), last(CycleResult_t), has_last, cycle(int), body_limit(size_t, 응답 본문 상한. 기본 WEB_BODY_CAPACITY이며 시험이 줄일 수 있다) |
| WebField_t | name(const char *), kind(WEB_KIND_NUMBER/BOOLEAN/ENUM), offset(size_t, RawInput_t 안의 RawValue_t 위치 = `offsetof`) |

입력 유효성 항목 13개의 이름과 비트 순서는 DOC-DDS-001의 "input_validity 비트 순서" 표를 그대로 쓴다 (비트 i가 i번째 항목).

**필드 표 (`WEB_FIELDS`, 16개)**:

| 이름 | 종류 | RawInput_t 필드 |
|------|------|-----------------|
| speed | NUMBER | speed |
| gear | ENUM | gear |
| source_timestamp | NUMBER | source_timestamp |
| crash | ENUM | crash |
| approach_left, approach_right | BOOLEAN | approach[0], approach[1] |
| fire, overtemp, adult | BOOLEAN | fire, overtemp, adult |
| isofix_left, isofix_right | BOOLEAN | isofix[0], isofix[1] |
| ignition, sensor_fault | BOOLEAN | ignition, sensor_fault |
| cmd_side, cmd_action, cmd_source | ENUM | cmd_side, cmd_action, cmd_source |

본문 규칙: `이름=값` 쌍을 `&`로 잇는다. URL 디코딩은 하지 않는다(값은 숫자 또는 `absent`뿐이다). 같은 이름이 반복되면 마지막 값이 적용된다. 표에 없는 이름, `=`가 없는 쌍, 빈 이름은 400이다. 빈 본문은 모두 기본값(정상 입력)이다.

## 2. 모듈

| ID | 모듈(`src/`) | 컴포넌트 | 책임 | 인터페이스 | 의존 | 데이터 |
|----|-------------|----------|------|-----------|------|--------|
| DD-M-014 | present/status_presenter.c/.h | ARCH-C-014 | 결과를 표시 항목과 JSON으로 만든다 | DD-I-008 Presenter_Build, Presenter_Serialize | common | 열거형 이름 표(상수) |
| DD-M-015 | history/event_recorder.c/.h | ARCH-C-015 | 이벤트를 기록하고 조회한다 | DD-I-009 Event_Init, Event_Record, Event_Query | common | Event_t |
| DD-M-016 | web/web_handler.c/.h | ARCH-C-016 | 경로와 메서드를 검사해 제어 주기, 이력, 표시를 조합한 응답을 만든다 | DD-I-010 Web_Init, Web_Handle | DD-M-013, 014, 015, 018, 019 공개 헤더 | Web_t, 경로 표 |
| DD-M-018 | web/web_page.c/.h | ARCH-C-016 | 시험 화면(HTML)을 제공한다 | DD-I-011 WebPage_Get | 없음 | HTML 상수 |
| DD-M-019 | web/web_request_parser.c/.h | ARCH-C-016 | 요청 본문(`이름=값&...`)을 제어 입력으로 해석한다 | DD-I-012 WebParser_ParseBody | common | 필드 표 `WEB_FIELDS` |
| DD-M-017 | web/web_main.c | ARCH-C-016 | 소켓으로 요청을 받아 Web_Handle에 넘기고 응답을 보낸다 | 기준 미만(진입점) | DD-M-016 공개 헤더, POSIX 소켓 | 없음 |

## 3. 함수

### DD-M-014 status_presenter

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-040 | `Status_t Presenter_Build(const CycleResult_t *res, PresentedStatus_t *out)` | 널이면 ERR_PARAM. 출력 이름, state 이름, 요약 priority_reason/reason_code 이름은 DD-F-041로 변환하고 경고코드는 res->warning_code가 1이면 "W-001", 아니면 "NONE", validity는 res->input_validity의 비트 i(DOC-DDS-001 비트 순서 표)가 1이면 "VALID", 아니면 "INVALID" | SRS-F-018, 019, 022 |
| DD-F-041 | `static Status_t Presenter_Name(const char *const *table, int count, int index, const char **name)` | index가 0 미만이거나 count 이상이면 *name="UNKNOWN", 아니면 table[index]. 항상 STATUS_OK | SRS-F-019 |
| DD-F-042 | `Status_t Presenter_Serialize(const PresentedStatus_t *st, char *buf, size_t cap, int *http_status)` | http_status가 널이면 ERR_PARAM. st/buf가 널이거나 cap이 0이면 *http_status=500 후 ERR_PARAM. 아니면 DD-F-043으로 JSON을 쓰고 실패하면 buf[0]=0, 500, 성공이면 200 (반환 STATUS_OK) | SRS-F-023, 018 |
| DD-F-043 | `static int Presenter_Write(const PresentedStatus_t *st, char *buf, size_t cap)` | `pos=0`에서 DD-F-044(텍스트 덧붙이기)로 다음을 이어 쓴다: `{"lock_left":"..","lock_right":"..","state":"..","priority_reason":"..","reason_code":"..","warning_code":"..","input_validity":{` + DD-F-045 + `}}`. 성공이면 1, 용량 부족이면 0 | SRS-F-018, 019, 022 |
| DD-F-044 | `static int Presenter_AppendText(char *buf, size_t cap, size_t *pos, const char *text)` | 텍스트 길이를 구해 `*pos + 길이 + 1 > cap`이면 0, 아니면 복사하고 *pos 증가 후 1 | SRS-F-023 |
| DD-F-045 | `static int Presenter_WriteValidity(const PresentedStatus_t *st, char *buf, size_t cap, size_t *pos)` | 이름 13개 표를 순회하며 `"이름":"VALID|INVALID"`를 쉼표로 이어 쓴다. 하나라도 실패하면 0 | SRS-F-018, 019 |

### DD-M-015 event_recorder

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-046 | `Status_t Event_Init(Event_t *ctx)` | 널이면 ERR_PARAM. 0으로 채우고 next_id=1, prev_output=RELEASE, prev_state=NORMAL, prev_reason=NONE, initialized=1 | SRS-F-020, 025, 026 |
| DD-F-047 | `Status_t Event_Record(Event_t *ctx, const CycleResult_t *res, double timestamp_s)` | 널이면 ERR_PARAM, 미초기화면 ERR_STATE. DD-F-048이 1이면 DD-F-049로 한 건 추가하고 prev_*를 갱신 | SRS-F-020, 024, 025 |
| DD-F-048 | `static int Event_HasChanged(const Event_t *ctx, const CycleResult_t *res)` | (좌 출력, 우 출력, state, reason_code_summary) 중 하나라도 prev와 다르면 1 (SRS 6.1 "결정") | SRS-F-025 |
| DD-F-049 | `static Status_t Event_Append(Event_t *ctx, const CycleResult_t *res, double ts)` | entries[head]에 id(next_id), ts, 출력, state, reason_code_summary를 쓰고 next_id 증가, head를 (head+1)%HISTORY_CAPACITY, count를 HISTORY_CAPACITY까지 증가 | SRS-F-020, 021, 024, SRS-L-006 |
| DD-F-050 | `Status_t Event_Query(const Event_t *ctx, EventEntry_t *out, size_t cap, size_t *count)` | 널이면 ERR_PARAM, 미초기화면 ERR_STATE, cap<count이면 ERR_PARAM. 가장 오래된 위치 (head + HISTORY_CAPACITY − count) % HISTORY_CAPACITY부터 count건을 순서대로 복사하고 *count 설정 | SRS-F-021, 026 |

### DD-M-016 web_handler

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-051 | `Status_t Web_Init(Web_t *ctx)` | 널이면 ERR_PARAM. Control_Init, Event_Init, has_last=0, cycle=0, body_limit=WEB_BODY_CAPACITY, initialized=1 | SRS-C-004 |
| DD-F-070 | `Status_t Web_SetBodyLimit(Web_t *ctx, size_t limit)` | 사전 조건: Web_Init 완료. ctx가 널이거나 미초기화이거나 limit이 0이거나 WEB_BODY_CAPACITY보다 크면 ERR_PARAM. 아니면 body_limit=limit (ARCH-D-13: 버퍼 넘침 방지) | SRS-C-004, SRS-F-023 |
| DD-F-052 | `Status_t Web_Handle(Web_t *ctx, const WebRequest_t *req, WebResponse_t *resp)` | ctx/req/resp 또는 req의 method/path/body가 널이거나 미초기화면 ERR_PARAM(미초기화는 ERR_STATE). 아니면 DD-F-053으로 경로 번호를 찾아 없으면 404, DD-F-054로 메서드를 검사해 다르면 405, 아니면 DD-F-055로 경로별 처리 | SRS-C-004, SRS-F-018 |
| DD-F-053 | `static int Web_FindRoute(const char *path)` | 경로 표 `ROUTES[ROUTE_COUNT] = {{"/","GET"},{"/status","GET"},{"/history","GET"},{"/step","POST"},{"/reset","POST"}}`에서 경로가 같은 항목의 번호, 없으면 -1 | SRS-C-004 |
| DD-F-054 | `static int Web_MethodAllowed(int route, const char *method)` | ROUTES[route].method와 같으면 1 | SRS-C-004 |
| DD-F-055 | `static Status_t Web_Dispatch(Web_t *ctx, int route, const char *body, WebResponse_t *resp)` | route에 따라 DD-F-056(화면), 057(상태), 058(이력), 059(step), 060(reset) 호출 | SRS-C-004 |
| DD-F-056 | `static Status_t Web_RespondPage(const Web_t *ctx, WebResponse_t *resp)` | DD-F-061으로 HTML을 얻어 본문 상한(ctx->body_limit)보다 길면 500, 아니면 복사하고 200, `text/html` | SRS-C-004 |
| DD-F-057 | `static Status_t Web_RespondStatus(const Web_t *ctx, WebResponse_t *resp)` | has_last가 0이면 status=204, 본문 빈 문자열, content_type `text/plain`(ARCH-D-12). 아니면 last로 Presenter_Build한 뒤 ctx->body_limit를 용량으로 Presenter_Serialize. http_status 500이면 resp->status=500 | SRS-F-018, 023 |
| DD-F-058 | `static Status_t Web_RespondHistory(const Web_t *ctx, WebResponse_t *resp)` | HISTORY_CAPACITY 크기의 지역 배열로 Event_Query를 호출(오류가 없다, ARCH-D-11)하고 JSON 배열 `[{"id":..,"timestamp_s":..,"lock_left":..,"lock_right":..,"state":..,"reason_code":..},...]`을 DD-F-062로 쓴다. 쓰기가 본문 상한(ctx->body_limit)을 넘으면 500 | SRS-F-021, 024 |
| DD-F-059 | `static Status_t Web_RespondStep(Web_t *ctx, const char *body, WebResponse_t *resp)` | WebParser_ParseBody(DD-F-063)로 본문을 입력으로 변환(실패 시 400 `{"error":"bad request"}`). 성공하면 DD-F-064로 한 주기를 실행하고 DD-F-057로 응답 | SRS-C-004, SRS-F-025 |
| DD-F-060 | `static Status_t Web_RespondReset(Web_t *ctx, WebResponse_t *resp)` | Web_Init 후 `{"reset":true}` 200 | SRS-C-004 |
| DD-F-062 | `static int Web_AppendEvent(char *buf, size_t cap, size_t *pos, const EventEntry_t *e, int first)` | 이벤트 하나를 JSON 객체로 이어 쓴다 (snprintf, 용량 부족이면 0) | SRS-F-024 |
| DD-F-063 | `int WebParser_ParseBody(const char *body, int cycle, RawInput_t *raw)` (DD-M-019, 인자가 널이면 0) | DD-F-065로 기본 입력을 만든 뒤 본문을 `&`로 쪼개 쌍마다 DD-F-066으로 적용. 구문·이름 오류면 0, 모두 적용하면 1 | SRS-C-004, SRS-Q-003 |
| DD-F-064 | `static Status_t Web_ExecuteCycle(Web_t *ctx, const RawInput_t *raw)` | Control_Step으로 last를 갱신하고(has_last=1), Event_Record로 이력을 기록하고, cycle++ 한다. Web_Init과 단조 시각이 보장되어(ARCH-D-11) 하위 호출은 오류를 반환하지 않으므로 반환값으로 분기하지 않고 항상 STATUS_OK | SRS-C-004, SRS-F-025 |
| DD-F-061 | `Status_t WebPage_Get(const char **html, size_t *length)` (DD-M-018) | 널이면 ERR_PARAM. 아니면 HTML 상수(`WEB_PAGE`: 입력 폼 16개 필드, 전송과 결과 표시 fetch 스크립트)와 그 길이를 돌려준다 | SRS-C-004 |
| DD-F-065 | `static Status_t Web_DefaultRaw(int cycle, RawInput_t *raw)` | cycle×CYCLE_PERIOD_S 시각의 정상 입력(속도 0, gear P, 충돌 NONE, 접근위험·화재·과온·성인·ISOFIX·sensor_fault FALSE, ignition TRUE, source_timestamp는 시각과 같음, 명령 없음) | SRS-C-004 |
| DD-F-066 | `static int Web_ApplyField(RawInput_t *raw, const char *name, size_t name_len, const char *value)` | `WEB_FIELDS`에서 이름이 같은 항목을 찾아(없으면 0) DD-F-067로 값을 변환해 `(RawValue_t *)((char *)raw + offset)`에 대입하고 1 | SRS-C-004 |
| DD-F-067 | `static Status_t Web_ParseValue(int kind, const char *text, RawValue_t *out)` | text가 "absent"이면 RAW_ABSENT. NUMBER는 DD-F-068, BOOLEAN과 ENUM은 DD-F-069로 구문 분석해 성공하면 각각 RAW_NUMBER(number), RAW_BOOLEAN(boolean), RAW_ENUM(enum_id), 실패하면 RAW_OTHER | SRS-C-004, SSR-007 |
| DD-F-068 | `static int Web_ParseNumber(const char *text, double *value)` | 문자열 전체가 실수(strtod가 끝까지 소비, 빈 문자열 제외)이면 1 | SSR-007 |
| DD-F-069 | `static int Web_ParseInteger(const char *text, int *value)` | 문자열 전체가 정수(strtol이 끝까지 소비, 빈 문자열 제외, int 범위)이면 1 | SSR-007 |

값 검증(범위, 값 집합, 유한성)은 Web이 하지 않는다. 파싱된 값은 Iter-1의 InputValidator(DD-F-002~005, SSR-007)가 평가 전에 검증하므로 무효 입력이 규칙에 쓰이지 않는다.

### DD-M-017 web_main

`main(int argc, char **argv)`: 포트(기본 `WEB_DEFAULT_PORT`)로 TCP 수신 소켓을 열고, 연결마다 요청 라인과 Content-Length 본문을 읽어 WebRequest_t를 만든다(본문이 없으면 `""`). Web_Handle 결과를 HTTP/1.1 응답으로 보낸다. 요청 1건씩 직렬 처리한다. 호출되는 함수가 없는 진입점이라 인터페이스 정의 기준 미만이고, 시연으로 검증한다.

## 4. 크기와 복잡도 계획 (CCN 10 이하, 80라인 이하)

| 함수 | 계획 |
|------|------|
| Presenter_Write | 텍스트 덧붙이기(DD-F-044)와 유효성 13항목 쓰기(DD-F-045)를 분리. 본 함수는 고정 순서의 덧붙이기 호출을 `ok &=`로 누적(분기 없음) |
| Web_Handle | 경로 검색, 메서드 검사, 경로별 처리를 DD-F-053~055로 분리 |
| Web_RespondStep | 파싱(063), 실행(064), 응답(057)으로 분리해 본 함수는 두 번의 상태 확인만 한다 |
| Web_ParseValue | 숫자(068)와 정수(069) 분석을 분리 |
| Web_ApplyField | 표 순회는 for 루프 하나, 변환은 DD-F-067 호출 |

## 5. 요구 → 설계 배정

| 요구 ID | 상세 설계 |
|---------|-----------|
| SRS-F-018 | DD-F-040, DD-F-042, DD-F-043, DD-F-045, DD-F-052, DD-F-057 |
| SRS-F-019 | DD-F-040, DD-F-041, DD-F-043, DD-F-045 |
| SRS-F-020 | DD-F-046, DD-F-047, DD-F-049 |
| SRS-F-021 | DD-F-049, DD-F-050, DD-F-058 |
| SRS-F-022 | DD-F-040, DD-F-043 |
| SRS-F-023 | DD-F-042, DD-F-044, DD-F-057, DD-F-070 |
| SRS-F-024 | DD-F-047, DD-F-049, DD-F-058, DD-F-062 |
| SRS-F-025 | DD-F-046, DD-F-047, DD-F-048, DD-F-059, DD-F-064 |
| SRS-F-026 | DD-F-046, DD-F-050 |
| SRS-Q-002 | DD-F-049, DD-F-058 |
| SRS-Q-003 | DD-F-043, DD-F-063 |
| SRS-L-006 | DD-F-049 |
| SRS-C-004 | DD-F-051, DD-F-070, DD-F-052, DD-F-053, DD-F-054, DD-F-055, DD-F-056, DD-F-059, DD-F-060, DD-F-063, DD-F-064, DD-F-065, DD-F-066, DD-F-067, DD-F-061 |

## 6. 점검표 결과

| 항목 | 결과 |
|------|------|
| 인터페이스 | DD-M-014~016 ARCH-I-008~010과 1:1, DD-M-018, 019는 컴포넌트 내부 인터페이스(DD-I-011, 012). web_main은 기준 미만 |
| 의존 | web_handler만 다른 모듈(ControlCycle, Presenter, EventRecorder)의 공개 헤더를 호출한다 |
| 단일 책임 | 모듈: 요청 처리(web_handler), 화면(web_page), 입력 해석(web_request_parser) 분리. 요청 처리 안에서도 라우팅, 상태, 이력, step, reset 분리 |
| 반환·추적 | 모든 함수가 Status_t 또는 정수 반환, 단일 반환 |
| 상수 | VALIDITY_ITEM_COUNT, WEB_BODY_CAPACITY, ROUTE_COUNT, 필드 수는 이름 있는 상수 |
