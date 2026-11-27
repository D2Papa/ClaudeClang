# 리뷰 보고서: Iter-2 설계 (아키텍처·상세 설계, 표시·이력·Web)

- 대상 (커밋 177bdf5, `feature/iter-0-requirements`):
  - `docs/architecture/ARCH-iter2.md` (DOC-ARC-002 v0.1)
  - `docs/detailed-design/DDS-iter2.md` (DOC-DDS-002 v0.1)
- 참고 입력: `docs/requirements/SRS.md` (DOC-SRS-001 v1.0), `docs/architecture/ARCH.md` (Iter-1, 리뷰 생략), `docs/detailed-design/DDS.md` (Iter-1, 리뷰 생략), `docs/safety/SAFETY-REQ.md` (DOC-SAF-001 v1.0)
- 담당 요구: SRS-F-011, 012, 013, 018~026, SRS-Q-002, Q-003, SRS-L-006, SRS-C-004 (지시 목록). 실제 SRS 1.0에서 SRS-F-011~013은 존재하지 않는다 (2장 D-04 참조).
- 산출물 종류: 아키텍처 설계(ARC-01~13), 상세 설계(DDS-01~12), 공통(COM-01~04)
- 일자: 2026-11-27
- 종합 판정: **반려**

## 1. 판정 근거 확인 사항

- 표시 필드 정합(SRS 6.4 OEM-IF-006): **불일치** (D-03)
- DD-F-042의 오류·HTTP 500 처리: 용량 부족과 널 처리는 ARCH-I-008과 일치 (확인). 단 기록 실패와 초기 응답의 처리는 미정의 (D-12)
- 이벤트 기록 조건(SRS-F-025 "결정" 정의)과 ARCH-D-08: **불일치** (D-02)
- 100건 순환 버퍼 인덱스: DD-F-047(head = (head+1) % 용량, count 상한 용량)과 DD-F-048(oldest = (head + 용량 − count) % 용량)이 올바름 (확인). 단 next_id 오버플로 미처리 (D-10)
- Web 입력 파싱과 SSR-007: DD-F-058은 값을 변환만 하고, 집합 검증은 Iter-1 DD-F-002(boolean 0/1), DD-F-003(유한한 실수 범위), DD-F-004(enum 범위), DD-F-005(명령 필드)가 수행한다 (확인). NaN과 Inf는 DD-F-003이 거절하므로 SSR-007을 해치지 않는다. 단 필드 표가 없어 파싱 규칙을 검증할 수 없음 (D-07)
- SRS-C-004(Web 시뮬레이터): `GET /` HTML 경로에 설계 함수가 없음 (D-05)

## 2. 체크리스트 결과

### 2.1 아키텍처 (ARC-01~13)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| ARC-01 | 컴포넌트와 한 문장 책임 | Pass | ARCH-iter2 §2의 C-014, C-015, C-016이 각각 한 문장 책임 |
| ARC-02 | 제공·요구 인터페이스 | Pass | C-014 제공 I-008, C-015 제공 I-009, C-016 제공 I-010과 요구 I-007·008·009 (§2) |
| ARC-03 | 인터페이스 정의 완전성 | **Fail** (D-01, D-02) | I-009 Event_Record의 입력 CycleResult_t가 도어별 reason을 제공하지 않음 (Iter-1 ARCH §4 CycleResult_t: priority_reason, reason_code가 단일 값). ARCH-D-08과 DD-F-046이 필요한 도어별 reason 정보가 없음. I-008 Build 입력도 같은 이유로 완전하지 않음 |
| ARC-04 | 인터페이스로만 통신 | Pass | C-016만 C-013, C-014, C-015의 공개 인터페이스를 쓰고 세 컴포넌트는 서로 호출하지 않음 |
| ARC-05 | 순환 의존 없음 | Pass | §2 조합 구조: C-016 → {C-013, C-014, C-015}. 역방향 없음 |
| ARC-06 | 응집도 | Pass | 표시(C-014), 이력(C-015), 조합·HTTP(C-016) 분리 |
| ARC-07 | 결합도 | Pass | 공유 가변 데이터 없음. Event_t, Web_t는 소유자만 접근 (ARCH-iter2 §6) |
| ARC-08 | 캡슐화 | Pass | 내부 필드 접근은 소유 컴포넌트 소스에 한정 |
| ARC-09 | SOLID | Pass | S, D를 근거로 명시. 위반 징후 없음 |
| ARC-10 | 정적·동적 구조 | **Fail** (D-14, 경미) | Iter-2 문서에 정적 구조 그림이 없고 동적 시나리오는 POST /step 하나뿐. GET /status, GET /history, POST /reset, GET / 흐름이 없음 |
| ARC-11 | 비기능 고려 근거 | Pass | Q-002(§6 필드 한정), Q-003(이름·값 집합 일치), L-006(HISTORY_CAPACITY) |
| ARC-12 | 요구 배정과 추적 | **Fail** (D-04) | §5 배정이 SRS-F-011, F-012, F-013에 연결됨. 이 ID는 SRS 1.0에 없음. 실제 항목은 F-018(상태 조회), F-019(표시 항목), F-022(priority_reason 매핑)이며 F-018~026 배정 자체는 맞음 |
| ARC-13 | 설계 결정 근거 | Pass | ARCH-D-06, D-07, D-08에 대안과 이유 기록 |

### 2.2 상세 설계 (DDS-01~12)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| DDS-01 | 컴포넌트의 모듈 분해와 책임 | **Fail** (D-05) | DD-M-016 데이터에 HTML(상수)이 있으나 HTML을 만드는 함수가 없음. 라우트 `/`가 DD-F-051에 있으나 DD-F-052~056 어디에도 연결되지 않음 |
| DDS-02 | 인터페이스 정의 기준 | Pass | §0의 기준(호출되는 함수가 있는 모듈은 인터페이스 정의)과 DD-M-017 제외가 명시됨 |
| DDS-03 | 기준 이상 모듈의 인터페이스 | Pass | DD-M-014~016에 DD-I-008~010 정의. DD-M-017은 기준 미만 |
| DDS-04 | 모듈 인터페이스와 ARCH-I 일치 | Pass | DD-I-008(Presenter_Build, Presenter_Serialize), DD-I-009(Event_Init, Record, Query), DD-I-010(Web_Init, Handle) 시그니처가 ARCH-I-008~010과 일치. 단 입력 데이터 형식 문제는 D-01 |
| DDS-05 | 공개 인터페이스로만 의존 | Pass | web_handler가 DD-M-013, 014, 015 공개 헤더만 사용 (§5) |
| DDS-06 | 반환값, void 금지 | **Fail** (D-08) | DD-F-041이 `const char *`를 반환. 설계 공통 규칙(§0: Status_t 또는 정수 반환)에 어긋남 |
| DDS-07 | 단일 반환 | Pass | §0 공통 규칙에 반환 1회가 명시됨. 코드 검증은 구현 단계 |
| DDS-08 | 단일 책임 | **Fail** (D-09) | DD-F-054 Web_RunStep이 본문 파싱, 제어 주기 실행, 이벤트 기록, 표시 직렬화를 함께 수행하며 변경 이유가 네 가지임 |
| DDS-09 | 이름·매직 넘버·상수 | **Fail** (D-13, 경미) | WEB_BODY_CAPACITY가 params.h 정의(DDS.md §0 표)와 DDS-iter2 어디에도 없음. `validity[13]`, 라우트 번호 0~4, "13개" 문구가 상수가 아닌 리터럴 |
| DDS-10 | 설계 함수의 크기·복잡도 달성 가능성 | **Fail** (D-11) | DD-F-043(13개 validity와 필드 직렬화), DD-F-058(3종 분기), DD-F-054(순차 호출 4단계)에 대해 CCN 10, 80라인을 지킬 수 있다는 근거(분할 방식이나 추정)가 없음 |
| DDS-11 | 입력 검증, 오류 처리, 경계 조건 | **Fail** (D-06, D-07, D-10, D-12) | 본문 필드 표 부재(D-07), GET에서 body NULL을 ERR_PARAM으로 처리하는 모순(D-06), next_id 오버플로(D-10), Event_Record 반환값 미처리와 초기 응답 미정의(D-12) |
| DDS-12 | 상위 추적(ARCH, SRS)과 하위 추적 | **Fail** (D-04) | §4 SRS 배정의 F-011~013이 존재하지 않는 ID. 하위(소스, 테스트) 추적은 구현 단계 대상으로 N/A |

### 2.3 공통 (COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| COM-01 | ID·버전·상태 | Pass | DOC-ARC-002 v0.1, DOC-DDS-002 v0.1, 상태 "초안 (리뷰 대기)" |
| COM-02 | 형상 관리 | Pass | 커밋 177bdf5에 포함. git status에 docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 | N/A | 사유: Iter-2 설계의 최초 리뷰. ARCH.md, DDS.md(Iter-1)는 리뷰 생략 |
| COM-04 | 상·하위 ID·용어 일관성 | **Fail** (D-04, D-15) | 존재하지 않는 SRS ID 참조. ARCH.md Iter-1 §2 그림과 ARCH-iter2의 구조 변경 기재가 어긋남 |

요약: ARC Pass 10 / Fail 3. DDS Pass 5 / Fail 7. COM Pass 2 / Fail 1 / N/A 1.

## 3. 결함 목록

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| D-01 | 중대 | ARCH-iter2 §2 (C-014 요구 IF I-007), §3 ARCH-I-008·009, DDS-iter2 §0·§2(DD-F-040, DD-F-046, DD-F-047) | Iter-2 인터페이스가 쓰는 도어별 priority_reason과 도어별 reason_code는 Iter-1 CycleResult_t(ARCH §4: output[2], state, priority_reason, reason_code, warning_code, input_validity)에 없다. ControlCycle이 이 값을 공급한다는 근거가 Iter-2 문서에 없다. Presenter와 EventRecorder가 쓰는 입력이 I-007 출력으로 정의되지 않았다. | 두 가지 중 하나를 택해 명시한다. (a) CycleResult_t에 도어별 priority_reason, reason_code를 추가하는 Iter-1 개정안을 I-007에 반영하고 DDS-iter2의 타입을 맞춘다. (b) Iter-2 입력을 단일 값으로 제한한다. 어느 쪽이든 ARCH-I-007의 사후 조건을 갱신한다. |
| D-02 | 중대 | ARCH-iter2 §1 ARCH-D-08, DDS-iter2 DD-F-046·DD-F-047, SRS 6.1 "결정", SRS-F-024·025, ASSUMPTIONS A-15 | SRS 6.1과 SRS-F-025는 결정을 "출력, state, reason_code 중 하나가 직전 주기와 달라진 주기"로 정의한다. reason_code는 단일 값이다. ARCH-D-08과 DD-F-046은 "도어별 reason_code 전체"로 판정하고 A-15를 근거로 인용하지만, A-15 문장은 단일 reason_code이다. 도어별 reason만 바뀌고 SRS reason_code가 같으면 설계는 이벤트를 만들고 SRS는 만들지 않는다. SRS-F-025 시험(변화 있는 주기마다 1건)을 위반할 수 있다. 또한 이벤트 reason_code 선택 규칙(좌 우선)은 SRS의 단일 reason_code가 무엇인지 정의하지 않는다. | 이벤트 기록 판정을 SRS 정의(출력, state, 단일 reason_code)로 맞추거나, 요구 변경(SRS 6.1, SRS-F-025, A-15 편차)을 먼저 승인받는다. 이벤트 reason_code를 CycleResult_t의 reason_code로 직접 쓰고, 좌 우선 규칙을 제거하거나 근거를 명시한다. |
| D-03 | 중대 | ARCH-iter2 §3 ARCH-I-008, DDS-iter2 §1 PresentedStatus_t, DD-F-040·043, SRS 6.4 OEM-IF-006, SRS-F-019·022, SRS-Q-003 | SRS 6.4의 OEM-IF-006은 state, priority_reason, reason_code, input_validity 네 항목을 단일 값으로 정의한다. 설계는 priority_reason[2], reason_code[2](도어별)와 warning_code(int)를 쓴다. (a) 형태가 SRS 정의와 다르다. (b) SRS-F-022는 reason_code가 priority_reason과 같은 값이어야 한다고 하지만 설계는 reason_code를 어떻게 정하는지 쓰지 않는다. (c) SRS 6.2는 경고코드 값을 "W-001"로 정의하나 설계는 int(0/1)로 쓴다. | 표시 항목 정의를 SRS 6.4와 일치시킨다. 도어별 값을 추가한다면 SRS 6.4 개정을 먼저 요청한다. priority_reason과 reason_code의 값 관계(같은 값)를 DD-F-040에 명시하고, warning_code를 "W-001" 또는 없음으로 표현한다. |
| D-04 | 중대 | ARCH-iter2 §2 표 (C-014 담당 요구), §5 배정 표 (SRS-F-011, 012, 013), DDS-iter2 §4 배정 표, 지시 목록 | SRS 1.0에 SRS-F-011, F-012, F-013이 없다. 이 ID들은 이전 번호 체계에서 안전 요구로 분리되어 폐기되었다(SRS §8). 실제 표시 기능은 F-018(상태 조회), F-019(표시 항목), F-022(priority_reason 매핑)다. 추적이 존재하지 않는 요구를 가리킨다. | ARCH-iter2 §2·§5와 DDS-iter2 §4에서 F-011, F-012, F-013을 F-018, F-019, F-022로 바꾼다. 담당 목록도 같이 고친다. |
| D-05 | 중대 | ARCH-iter2 §3 ARCH-I-010 (GET / HTML 200), DDS-iter2 DD-M-016, DD-F-050, DD-F-051 | ARCH-I-010은 `GET /`에 HTML 화면 200을 요구한다. DDS는 DD-F-051에서 `/`를 라우트 0으로 찾지만 DD-F-052~056 어디에도 HTML을 쓰는 함수가 없다. DD-M-016 데이터의 "HTML(상수)"을 쓰는 함수가 없다. 시험 화면(SRS-C-004)의 핵심 경로가 설계되지 않았다. | `GET /` 처리 함수(예: DD-F-060 Web_RespondPage)를 추가하고 HTML 상수의 소유와 용량을 정의한다. DD-F-050 경로 표에 연결한다. |
| D-06 | 중대 | DDS-iter2 DD-F-050 (널 검사) | DD-F-050은 req의 method, path, body 중 하나라도 널이면 ERR_PARAM이라고 한다. GET 요청은 본문이 없으므로 body가 널인 것이 정상이다. 그러면 GET /, /status, /history가 모두 ERR_PARAM이 되어 ARCH-I-010 응답(200)을 낼 수 없다. 또한 허용 메서드 표(어느 경로가 GET, 어느 경로가 POST인지)가 없어서 405 조건을 구현할 수 없다. | body 널은 허용하고(빈 본문으로 처리) 널 검사를 method와 path에만 적용한다. 경로별 허용 메서드 표를 DD-F-051에 추가한다. |
| D-07 | 중대 | DDS-iter2 DD-F-055, DD-F-056 (필드 표 참조), DD-M-016 데이터("필드 표(상수)") | DD-F-056은 "필드 표(이름, 종류 NUM/BOOL/ENUM, RawValue 위치)"를 쓰지만 그 표가 DDS 어디에도 없다. 입력 이름 13개의 목록만 있고, 각 이름이 RawInput_t의 어느 필드에 가는지, 명령 필드(present, side, action, source)를 어떤 이름으로 받는지 정의되지 않았다. 본문 인코딩(URL 디코딩), 같은 이름이 두 번 나올 때의 처리도 없다. 파싱 규칙이 정의되지 않으면 SSR-007 검증 근거를 확인할 수 없다. | 필드 표를 DDS-iter2에 추가한다(이름, 종류, RawInput_t 필드, 허용 형식). 명령 필드 이름을 정의한다. 중복 이름과 URL 디코딩 처리를 DD-F-055에 명시한다. |
| D-08 | 중대 | DDS-iter2 DD-F-041 | DD-F-041 `static const char *Presenter_Name(...)`은 포인터를 반환한다. DDS-iter2 §0 공통 규칙("함수는 Status_t(또는 정수) 반환")과 CLAUDE.md의 상태 코드 반환 원칙에 어긋난다. | 이름 조회를 `Status_t Presenter_Name(..., const char **name)`으로 바꾸거나, 공통 규칙에 예외를 명시하고 그 근거를 적는다. |
| D-09 | 중대 | DDS-iter2 DD-F-054 Web_RunStep, DD-M-016 | DD-F-054는 본문 파싱(DD-F-055), 제어 주기 실행(Control_Step), 이벤트 기록(Event_Record), 응답 생성(DD-F-052)을 모두 수행한다. 변경 이유가 입력 형식, 제어 인터페이스, 이벤트 정책, 표시 형식 네 가지다. DD-M-016도 라우팅, 파싱, HTML 응답, 조합을 함께 맡는다. | DD-F-054를 파싱 단계(입력 구성), 실행 단계(Control_Step과 Event_Record 호출), 응답 단계(DD-F-052)로 나눠 각 단계를 별도 함수로 정의한다. DD-M-016의 HTML과 라우팅 책임도 분리할 것을 검토한다. |
| D-10 | 중대 | DDS-iter2 DD-F-044(next_id=1), DD-F-047(id = next_id++), ARCH-I-009 사후(고유 ID) | SRS-F-020은 "이전에 부여한 적이 없는" 고유 ID를 요구한다. next_id는 uint32_t이므로 2^32번째 이벤트 이후 값이 0으로 되돌아가 이전 ID와 중복된다. 설계에 이 경계가 없다. | next_id를 uint64_t로 하거나, 2^32 − 1 도달 시 기록을 중단하고 오류를 반환하는 동작을 정의한다. 어느 쪽이든 ARCH-I-009 사후 조건을 갱신한다. |
| D-11 | 중대 | DDS-iter2 DD-F-043, DD-F-054, DD-F-058 | 세 함수의 크기와 복잡도(CCN 10, 80라인)를 지킬 수 있다는 근거가 없다. DD-F-043은 13개 validity와 4개 키 필드를 한 함수에서 쓰고, DD-F-058은 3종 분기, DD-F-054는 4단계를 순차 호출한다. | 각 함수의 분할 계획(예: validity 배열 직렬화 별도 함수, 종류별 파서 분리)과 예상 CCN, 라인 수 근거를 적는다. |
| D-12 | 경미 | DDS-iter2 DD-F-054 (Event_Record 반환값), DD-F-052 (has_last=0 초기 응답) | Event_Record 실패 처리가 없다(실패해도 응답 생성 여부 불명확). 초기 응답은 RELEASE, NORMAL, NONE만 정해져 있고 input_validity 13개와 warning_code 값이 정의되지 않았다. | Event_Record 실패 시 응답(500 또는 200)을 정의한다. 초기 응답의 input_validity와 warning_code 값을 정한다. |
| D-13 | 경미 | DDS-iter2 §1 PresentedStatus_t (validity[13]), DD-F-051 (경로 0~4), DD-M-016 | WEB_BODY_CAPACITY 상수가 정의되지 않았다(params.h 정의 목록 밖). validity 13과 라우트 번호 0~4가 리터럴이다. | WEB_BODY_CAPACITY와 입력 이름 개수(INPUT_VALIDITY_COUNT)를 상수 표(params.h 추가 항목)에 정의하고 라우트 번호는 열거형으로 바꾼다. |
| D-14 | 경미 | ARCH-iter2 전체 | 정적 구조(컴포넌트 연결도)가 없고, 동적 시나리오는 POST /step 하나뿐이다. GET /status, GET /history, POST /reset, GET / 흐름이 빠져 있다. | Iter-2 범위의 정적 구조 그림을 추가하고 주요 조회·재설정 시나리오를 시퀀스로 추가한다. |
| D-15 | 경미 | ARCH-iter2 §0 (DOC-ARC-001 5장 인용), DOC-ARC-001 §2 그림(C-013 -.-> C-015, C-014) | ARCH-iter2는 "DOC-ARC-001 5장의 '사용자 호출' 구조를 바꾼다"고 쓰지만, 그 내용은 ARCH.md §2 정적 구조 그림과 §5 동적 구조 문서에 있다(해당 대목의 위치 오류). ARCH.md §2 그림은 여전히 ControlCycle에서 EventRecorder, StatusPresenter로 점선 화살표를 그린다. | ARCH-iter2 §0의 인용을 "ARCH.md §2"로 고치고, ARCH.md 그림을 Iter-2 결정(WebSimulator 조합)에 맞게 수정하거나 "Iter-2에서 변경됨"을 그림 옆에 표기한다. |

## 4. 확인 사항 (결함 없음)

- DD-F-047 링 버퍼 쓰기: entries[head] 기록 후 head = (head+1) % 용량, count는 용량까지 증가 (정상).
- DD-F-048 오래된 위치 계산 (head + 용량 − count) % 용량: count = 용량일 때 head, count = 0일 때 head로 정상. 용량 부족 cap은 ERR_PARAM.
- DD-F-042 직렬화: 널이면 ERR_PARAM과 500, 용량 부족이면 buf[0]=0과 500 (ARCH-I-008과 일치). 직렬화 실패가 오류 코드가 아니라 HTTP 500으로 표현됨 (SRS-F-023 일치).
- DD-F-040: 널이면 ERR_PARAM. validity 13개는 res->input_validity 비트에서 채움 (SRS 6.2 "6.1 필수 입력과 gear" 13개와 목록 일치).
- DD-F-056·058, SSR-007 경계: 파싱은 "absent", 정수, 실수만 구분하고 값 집합 검사는 Iter-1 검증기(DD-F-002: boolean 0/1, DD-F-003: 유한 실수 범위, DD-F-004: enum 범위, DD-F-005: 명령 3필드)가 한다. 집합 밖 값과 NaN, Inf는 INVALID가 된다. 무효 입력이 평가 전에 거절됨 (확인).
- 개인정보(SRS-Q-002): 이벤트 필드(DD-F-047)와 이력 JSON(DD-F-053)에 영상·음성·식별자 없음 (확인).
- 호환성(SRS-Q-003): 입출력 이름은 SRS 6.4 이름과 대응 (형식 차이는 D-03).
- SRS-L-006: HISTORY_CAPACITY = 100 (params.h, DDS.md §0) 사용 (확인).

## 5. 요약

- 종합 판정: **반려**
- 검사: ARC 13항목 (Pass 10, Fail 3), DDS 12항목 (Pass 5, Fail 7), COM 4항목 (Pass 2, Fail 1, N/A 1)
- 결함: 중대 11건 (D-01~D-11), 경미 4건 (D-12~D-15). 합계 15건
- 중대 결함 요지: 인터페이스가 쓰는 도어별 reason 데이터가 I-007에 없음(D-01), 이벤트 결정 판정이 SRS 정의와 다름(D-02), 표시 항목 형태와 값이 SRS 6.4·6.2와 다름(D-03), 존재하지 않는 SRS ID 추적(D-04), GET / HTML 경로 설계 누락(D-05), GET 본문 널 처리 모순과 메서드 표 부재(D-06), 파싱 필드 표 부재(D-07), 포인터 반환 함수(D-08), 다중 책임 함수(D-09), next_id 오버플로(D-10), 크기 근거 부재(D-11)

## 6. 판정 메모

- D-01과 D-02는 Iter-1 설계(리뷰 생략)와 SRS 정의가 맞물린 문제다. Iter-1 ARCH/DDS를 함께 개정해야 하므로 범위를 정하는 사용자 판단이 필요하다.
- D-03은 SRS 6.4 개정 여부를 결정해야 한다. 설계만 고칠 수 없는 항목이다.
- D-04는 추적 ID 정정만 하면 된다.
- 재리뷰 시 D-01~D-15의 조치 상태를 표로 확인하고, 수정으로 생기는 Iter-1 ARCH·DDS 변경과 SRS 6.1·6.4 변경의 영향을 함께 점검한다.
