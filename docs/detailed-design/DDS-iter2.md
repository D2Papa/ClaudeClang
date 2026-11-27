# DDS-iter2 — 표시·이력·Web 상세 설계서 (Iter-2)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-DDS-002 |
| 버전 | 0.1 |
| 상태 | 초안 (리뷰 대기) |
| 작성 | Claude Sonnet 5.5, 2026-11-27, 요청자 KYLee |
| 입력 | DOC-ARC-002 v0.1, DOC-DDS-001 (공통 규칙), DOC-SRS-001 |

- **인터페이스 정의 기준 수준**: 다른 모듈이 호출하는 함수가 하나라도 있는 모듈은 인터페이스(DD-I)를 정의한다. DD-M-014~016은 모두 대상이고 ARCH-I-008~010과 1:1이다. `web_main.c`는 호출되는 함수가 없는 어댑터(진입점)라 기준 미만이다.
- 공통 규칙은 DOC-DDS-001과 같다: 함수는 `Status_t`(또는 정수) 반환, `void` 없음, 반환 1회, CCN 10 이하, 80라인 이하, 전역 가변 데이터 없음. 모든 소스는 ARCH 컴포넌트 외부에 의존하지 않고 공개 헤더만 사용한다.

## 1. 타입

| 타입 | 정의 |
|------|------|
| PresentedStatus_t | lock_left, lock_right(문자열 "LOCK"/"RELEASE"), state(문자열), priority_reason[2], reason_code[2](문자열), warning_code(int), validity[13](문자열 "VALID"/"INVALID") |
| EventEntry_t | id(uint32_t), timestamp_s(double), lock_left, lock_right(Output_t), state(State_t), reason_code(ReasonCode_t) |
| Event_t | entries[HISTORY_CAPACITY], head(다음 기록 위치), count(보존 건수), next_id, initialized, prev_output[2], prev_state, prev_reason[2] |
| WebRequest_t | method(const char*), path(const char*), body(const char*) |
| WebResponse_t | status(int), content_type(const char*), body(char[WEB_BODY_CAPACITY]) |
| Web_t | initialized, control(Control_t), events(Event_t), last(CycleResult_t), has_last, cycle(int) |

입력 유효성 이름 13개(순서 고정): speed, gear, source_timestamp, crash, approach_left, approach_right, fire, overtemp, adult, isofix_left, isofix_right, ignition, sensor_fault.

## 2. 모듈

| ID | 모듈(`src/`) | 컴포넌트 | 책임 | 인터페이스 | 의존 | 데이터 |
|----|-------------|----------|------|-----------|------|--------|
| DD-M-014 | present/status_presenter.c/.h | ARCH-C-014 | 결과를 표시 항목과 JSON으로 만든다 | DD-I-008 Presenter_Build, Presenter_Serialize | common | 열거형 이름 표(상수) |
| DD-M-015 | history/event_recorder.c/.h | ARCH-C-015 | 이벤트를 기록하고 조회한다 | DD-I-009 Event_Init, Event_Record, Event_Query | common | Event_t |
| DD-M-016 | web/web_handler.c/.h | ARCH-C-016 | HTTP 요청을 처리해 제어 주기, 이력, 표시를 조합한다 | DD-I-010 Web_Init, Web_Handle | DD-M-013, 014, 015 공개 헤더 | Web_t, 필드 표(상수), HTML(상수) |
| DD-M-017 | web/web_main.c | ARCH-C-016 | 소켓으로 요청을 받아 Web_Handle에 넘기고 응답을 보낸다 | 기준 미만(진입점) | DD-M-016 공개 헤더, POSIX 소켓 | 없음 |

## 3. 함수

### DD-M-014 status_presenter

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-040 | `Status_t Presenter_Build(const CycleResult_t *res, PresentedStatus_t *out)` | 널이면 ERR_PARAM. 출력, state, 도어별 reason 이름은 DD-F-041로 변환하고 validity 13개는 res->input_validity 비트로 채운다 | SRS-F-011, 012, 013, 018, 019, 022 |
| DD-F-041 | `static const char *Presenter_Name(const char *const *table, int count, int index)` | index가 0 미만이거나 count 이상이면 "UNKNOWN", 아니면 table[index] | SRS-F-019 |
| DD-F-042 | `Status_t Presenter_Serialize(const PresentedStatus_t *st, char *buf, size_t cap, int *http_status)` | http_status가 널이면 ERR_PARAM. st 또는 buf가 널이거나 cap이 0이면 ERR_PARAM이고 http_status=500. 아니면 DD-F-043으로 JSON을 쓰고 길이가 cap 이상이면 buf[0]=0, http_status=500, 정상이면 200 | SRS-F-023, SRS-F-018 |
| DD-F-043 | `static int Presenter_Write(const PresentedStatus_t *st, char *buf, size_t cap)` | 고정 순서의 JSON을 snprintf로 쓰고 길이를 반환한다. 필드: lock_left, lock_right, state, priority_reason{left,right}, reason_code{left,right}, warning_code, input_validity{13개}. reason_code 이름은 priority_reason과 같은 열거형 집합 | SRS-F-018, 019, 022 |

### DD-M-015 event_recorder

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-044 | `Status_t Event_Init(Event_t *ctx)` | 널이면 ERR_PARAM. 0으로 채우고 next_id=1, prev_output=RELEASE, prev_state=NORMAL, prev_reason=NONE, initialized=1 | SRS-F-020, 025, 026 |
| DD-F-045 | `Status_t Event_Record(Event_t *ctx, const CycleResult_t *res, double timestamp_s)` | 널이면 ERR_PARAM, 미초기화면 ERR_STATE. DD-F-046으로 변경 여부를 판정해 변경되었으면 DD-F-047로 한 건 추가하고 prev_*를 갱신한다 | SRS-F-020, 024, 025, 021, SRS-L-006 |
| DD-F-046 | `static int Event_HasChanged(const Event_t *ctx, const CycleResult_t *res)` | 도어별 출력, state, 도어별 reason_code 중 하나라도 prev와 다르면 1 | SRS-F-025 |
| DD-F-047 | `static Status_t Event_Append(Event_t *ctx, const CycleResult_t *res, double ts)` | entries[head]에 id(next_id++), ts, 출력, state, reason_code(좌 reason이 NONE이 아니면 좌, 아니면 우)를 쓰고 head를 (head+1) % 용량, count를 용량까지 증가 | SRS-F-020, 024, 021 |
| DD-F-048 | `Status_t Event_Query(const Event_t *ctx, EventEntry_t *out, size_t cap, size_t *count)` | 널이면 ERR_PARAM, 미초기화면 ERR_STATE, cap이 count보다 작으면 ERR_PARAM. 가장 오래된 위치((head + 용량 − count) % 용량)부터 count건을 복사하고 *count 설정 | SRS-F-021, 026 |

### DD-M-016 web_handler

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-049 | `Status_t Web_Init(Web_t *ctx)` | 널이면 ERR_PARAM. Control_Init, Event_Init, has_last=0, cycle=0 | SRS-C-004 |
| DD-F-050 | `Status_t Web_Handle(Web_t *ctx, const WebRequest_t *req, WebResponse_t *resp)` | 널(ctx, req, resp, req의 method/path/body)이면 ERR_PARAM. 라우팅: 경로 `/` `/status` `/history` `/step` `/reset` 을 DD-F-051로 찾고 허용 메서드가 아니면 405, 없으면 404. 경로별 DD-F-052~056 호출 | SRS-C-004, SRS-F-018 |
| DD-F-051 | `static int Web_FindRoute(const char *path)` | 경로 문자열을 경로 번호(0~4)로, 없으면 -1 | SRS-C-004 |
| DD-F-052 | `static Status_t Web_RespondStatus(Web_t *ctx, WebResponse_t *resp)` | has_last가 0이면 정상 초기 결과(RELEASE, NORMAL, NONE)로, 아니면 last로 Presenter_Build/Serialize 호출. http_status 500이면 resp->status=500 | SRS-F-018, 023 |
| DD-F-053 | `static Status_t Web_RespondHistory(Web_t *ctx, WebResponse_t *resp)` | Event_Query 결과를 JSON 배열(id, timestamp_s, lock_left, lock_right, state, reason_code)로 쓴다. 버퍼 부족이면 500 | SRS-F-021, 024 |
| DD-F-054 | `static Status_t Web_RunStep(Web_t *ctx, const char *body, WebResponse_t *resp)` | DD-F-055로 본문을 RawInput_t로 만든다(오류면 400 `{"error":...}`). Control_Step 오류면 500. 성공하면 last 갱신, Event_Record, cycle++ 후 DD-F-052 | SRS-C-004, SRS-F-025 |
| DD-F-055 | `static int Web_ParseBody(const char *body, int cycle, RawInput_t *raw)` | 기본 정상 입력(DD-F-057)을 만든 뒤 `이름=값`을 `&`로 분리해 DD-F-056으로 적용. 이름이 필드 표에 없으면 0(오류), 모두 적용하면 1 | SRS-C-004, SRS-Q-003 |
| DD-F-056 | `static int Web_ApplyField(RawInput_t *raw, const char *name, const char *value)` | 필드 표(이름, 종류 NUM/BOOL/ENUM, RawValue 위치)에서 이름을 찾아 값을 DD-F-058로 변환해 대입. 없으면 0 | SRS-C-004 |
| DD-F-057 | `static Status_t Web_DefaultRaw(int cycle, RawInput_t *raw)` | cycle×CYCLE_PERIOD_S 시각의 정상 입력(속도 0, gear P, 충돌 NONE, 나머지 FALSE, ignition TRUE, source_timestamp는 시각과 같음) | SRS-C-004 |
| DD-F-058 | `static Status_t Web_ParseValue(int kind, const char *text, RawValue_t *out)` | text가 "absent"이면 RAW_ABSENT. NUM: 문자열 전체가 실수이면 RAW_NUMBER, 아니면 RAW_OTHER. BOOL과 ENUM: 문자열 전체가 정수이면 각각 RAW_BOOLEAN(boolean=값), RAW_ENUM(enum_id=값), 아니면 RAW_OTHER | SRS-C-004, SSR-007 |
| DD-F-059 | `static Status_t Web_Reset(Web_t *ctx, WebResponse_t *resp)` | Web_Init 후 `{"reset":true}` 200 | SRS-C-004 |

### DD-M-017 web_main

`main(int argc, char **argv)`: 포트(기본 8081)로 TCP 수신 소켓을 열고, 연결마다 요청 라인과 Content-Length 본문을 읽어 WebRequest_t를 만들고 Web_Handle 결과를 HTTP/1.1 응답으로 보낸다. 주기 처리는 요청마다 직렬이다. 호출되는 함수가 없는 진입점이라 인터페이스 정의 기준 미만이다 (단위 테스트 대상이 아니며 시연으로 검증한다).

## 4. 요구 → 설계 배정

| 요구 ID | 상세 설계 |
|---------|-----------|
| SRS-F-011 | DD-F-040 |
| SRS-F-012 | DD-F-040 |
| SRS-F-013 | DD-F-040, DD-F-043 |
| SRS-F-018 | DD-F-040, DD-F-042, DD-F-043, DD-F-052 |
| SRS-F-019 | DD-F-040, DD-F-041, DD-F-043 |
| SRS-F-020 | DD-F-044, DD-F-045, DD-F-047 |
| SRS-F-021 | DD-F-047, DD-F-048, DD-F-053 |
| SRS-F-022 | DD-F-040, DD-F-043 |
| SRS-F-023 | DD-F-042, DD-F-052 |
| SRS-F-024 | DD-F-045, DD-F-047, DD-F-053 |
| SRS-F-025 | DD-F-045, DD-F-046, DD-F-054 |
| SRS-F-026 | DD-F-044, DD-F-048 |
| SRS-Q-002 | DD-F-047, DD-F-053 |
| SRS-Q-003 | DD-F-043, DD-F-055 |
| SRS-L-006 | DD-F-047 |
| SRS-C-004 | DD-F-049, DD-F-050, DD-F-054, DD-F-055, DD-F-059 |

## 5. 점검표 결과

| 항목 | 결과 |
|------|------|
| 인터페이스 | DD-M-014~016 모두 정의, ARCH-I-008~010과 1:1. web_main은 기준 미만 |
| 의존 | web_handler만 다른 모듈(ControlCycle, Presenter, EventRecorder)의 공개 헤더를 호출한다 |
| 크기 | Web_Handle은 라우팅(DD-F-051)과 경로별 함수로 나눠 CCN 10 이하. 필드 표 방식으로 Web_ApplyField의 분기를 줄인다 |
| 반환·추적 | 모든 함수가 Status_t 또는 정수 반환, 단일 반환, 3장 추적 열 |
