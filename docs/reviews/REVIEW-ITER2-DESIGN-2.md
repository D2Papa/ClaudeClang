# 리뷰 보고서: Iter-2 설계 2차 재리뷰 (표시·이력·Web)

- 대상 (커밋 5f1df0e, `feature/iter-0-requirements`):
  - `docs/architecture/ARCH-iter2.md` (DOC-ARC-002 v0.2)
  - `docs/detailed-design/DDS-iter2.md` (DOC-DDS-002 v0.2)
  - 개정: `docs/architecture/ARCH.md` (v0.2, §2 정적 구조, §4 타입, §8 배정), `docs/detailed-design/DDS.md` (v0.2, DD-F-028)
  - 가정: `docs/requirements/ASSUMPTIONS.md` (A-19, A-20 추가)
- 이전 리뷰: REVIEW-ITER2-DESIGN-1.md (D-01~D-15)
- 참고 입력: `docs/requirements/SRS.md` (DOC-SRS-001 v1.0), `docs/safety/SAFETY-REQ.md` (DOC-SAF-001 v1.0)
- 산출물 종류: 아키텍처 설계(ARC-01~13), 상세 설계(DDS-01~12), 공통(COM-01~04)
- 일자: 2026-11-27
- 종합 판정: **반려**

## 1. 이전 결함 조치 상태 (D-01~D-15)

| 번호 | 심각도 | 1차 내용 | 조치 상태 | 근거 / 잔여 |
|------|--------|----------|-----------|-------------|
| D-01 | 중대 | I-007 출력에 도어별 reason 없음 | 조치 완료 | ARCH.md §4 CycleResult_t에 priority_reason[2], reason_code[2], 요약 값 두 개 추가 확인. Presenter와 EventRecorder는 요약 값을 쓴다 |
| D-02 | 중대 | 이벤트 결정 판정이 SRS 6.1과 다름 | 조치 완료 | DD-F-048이 (좌/우 출력, state, reason_code_summary)로 판정, ARCH-D-08 및 A-20 정정. 단 A-19에 의한 reason_code 덮어쓰기 문제는 E-01 |
| D-03 | 중대 | 표시 항목 형태와 값이 SRS 6.4와 다름 | **부분 조치** | 표시가 단일 값으로 바뀌고 경고코드 int가 문자열로 바뀜 (확인). 그러나 A-19로 reason_code가 priority_reason과 달라질 수 있어 SRS-F-022(같은 값) 위반 (E-01). 경고코드 "NONE"의 근거 인용 오류 (E-06) |
| D-04 | 중대 | 폐기 ID F-011~013 배정 | 조치 완료 | ARCH-iter2 §6, DDS-iter2 §5에서 F-018, 019, 022로 정정. ARCH.md §8에 F-011~013 없음 (확인) |
| D-05 | 중대 | GET / HTML 설계 누락 | 조치 완료 | DD-F-056 Web_RespondPage, WEB_PAGE 상수, 길이 초과 시 500 (확인) |
| D-06 | 중대 | GET 본문 널 처리, 메서드 표 부재 | 조치 완료 | 본문은 널 불가("" 전달), DD-F-053 경로 표와 DD-F-054 메서드 검사 (확인) |
| D-07 | 중대 | 필드 표 부재, 본문 규칙 부재 | 조치 완료 (새 결함 E-03) | 16개 필드 표, 디코딩 없음, 마지막 값 우선, 400 규칙 확인. 표의 필드 이름이 ARCH의 RawInput_t 이름과 다름 (E-03) |
| D-08 | 중대 | 포인터 반환 함수 | 조치 완료 | DD-F-041을 Status_t와 출력 인자로 변경 (확인) |
| D-09 | 중대 | Web_RunStep 다중 책임 | 조치 완료 (모듈 수준 잔여 E-08) | 파싱(DD-F-063), 실행(DD-F-064), 응답(DD-F-057)으로 분리. DD-M-016 모듈 자체의 다중 책임은 E-08 |
| D-10 | 중대 | next_id 오버플로 | 조치 완료 | uint64 (ARCH-D-09, DD-F-046, DD-F-049, EventEntry_t) |
| D-11 | 중대 | 크기·복잡도 근거 없음 | 조치 완료 | §4 분할 계획 (Presenter_Write, Web_Handle, Web_RespondStep, Web_ParseValue, Web_ApplyField) |
| D-12 | 경미 | Event_Record 오류와 초기 응답 | 조치 완료 (세부 E-05, E-07) | DD-F-059가 Event_Record 오류를 500으로 처리. 초기 응답 정의(DD-F-057). 초기 validity 값과 부분 커밋 문제는 E-05, E-07 |
| D-13 | 경미 | 상수 미정의 | 조치 완료 (잔여 E-09) | VALIDITY_ITEM_COUNT, WEB_BODY_CAPACITY, ROUTE_COUNT 정의. 0x1FFF과 8081 리터럴은 잔여 |
| D-14 | 경미 | 정적 구조와 GET 시퀀스 부재 | 조치 완료 | ARCH-iter2 §2 정적 구조, §5 시퀀스 (POST /step, GET /status, GET /history, POST /reset) 확인 |
| D-15 | 경미 | ARCH.md 그림과 인용 오류 | 조치 완료 | ARCH.md §2 그림의 C-016 → C-013, C-014, C-015 구조와 §2 본문 개정 확인 |

이전 결함 15건 중 조치 완료 14, 부분 조치 1 (D-03). 미조치 0. 부분 조치가 있으므로 반려다.

## 2. 체크리스트 결과

### 2.1 아키텍처 (ARC-01~13)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| ARC-01 | 컴포넌트와 한 문장 책임 | Pass | ARCH-iter2 §3의 C-014, C-015, C-016 책임 한 문장 |
| ARC-02 | 제공·요구 인터페이스 | **Fail** (E-04, 경미) | C-014 요구 IF가 "—"이지만 Build 입력은 I-007 출력(CycleResult_t)이다. ARCH.md의 I-007 소비자 목록은 C-014를 직접 소비자로 쓰지만 ARCH-iter2 호출 구조는 C-016이 결과를 전달한다. 두 문서가 다르다 |
| ARC-03 | 인터페이스 정의 완전성 | Pass | I-008, I-009, I-010의 시그니처, 사전·사후, 오류 처리, 타이밍이 정의됨. 요약 값 입력이 I-007 출력으로 정의됨 |
| ARC-04 | 인터페이스로만 통신 | Pass | C-016만 C-013, C-014, C-015를 호출하고 세 컴포넌트는 서로 호출하지 않음 |
| ARC-05 | 순환 의존 없음 | Pass | §2 정적 구조 (C-016 → C-013, C-014, C-015). 역방향 없음 |
| ARC-06 | 응집도 | Pass | 표시, 이력, 조합·HTTP 분리 |
| ARC-07 | 결합도 | Pass | 공유 가변 데이터 없음 |
| ARC-08 | 캡슐화 | Pass | Event_t, Web_t 내부 접근은 소유 컴포넌트에 한정 |
| ARC-09 | SOLID | Pass | S, D 근거 명시. 위반 징후 없음 |
| ARC-10 | 정적·동적 구조 | Pass | §2 정적 구조, §5 동적 시퀀스 4개 |
| ARC-11 | 비기능 고려 | Pass | Q-002(§7 필드 한정), Q-003(이름·값 집합), L-006(HISTORY_CAPACITY) |
| ARC-12 | 요구 배정과 추적 | Pass | §6 배정 SRS-F-018~026, Q-002, Q-003, L-006, C-004. 폐기 ID 없음 |
| ARC-13 | 설계 결정 근거 | Pass | ARCH-D-06~10에 결정과 이유 기록 |

### 2.2 상세 설계 (DDS-01~12)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| DDS-01 | 컴포넌트의 모듈 분해와 책임 | Pass | DD-M-014~017. HTML은 DD-F-056이 생성 |
| DDS-02 | 인터페이스 정의 기준 | Pass | §0 기준과 DD-M-017 제외가 명시됨 |
| DDS-03 | 기준 이상 모듈의 인터페이스 | Pass | DD-M-014~016에 DD-I-008~010 정의 |
| DDS-04 | 모듈 인터페이스와 ARCH-I 일치 | Pass | 시그니처 일치 (Build, Serialize, Init, Record, Query, Web_Init, Web_Handle) |
| DDS-05 | 공개 인터페이스로만 의존 | Pass | web_handler는 DD-M-013~015 공개 헤더만 사용 |
| DDS-06 | 반환값과 void 금지 | Pass | DD-F-041 Status_t로 변경됨. 전 함수 Status_t 또는 정수 |
| DDS-07 | 단일 반환 | Pass | §0 공통 규칙 명시. 코드 검증은 구현 단계 |
| DDS-08 | 단일 책임 | **Fail** (E-08) | DD-M-016 한 모듈이 경로 라우팅, HTML 페이지 생성, 본문 파싱, step·reset·history 응답을 맡음. 변경 이유가 UI, 파싱, 라우팅, 제어 연동 등 네 가지 이상 |
| DDS-09 | 이름·매직 넘버 | **Fail** (E-09, 경미) | DD-F-057의 0x1FFF 리터럴, DD-M-017의 포트 8081 리터럴 |
| DDS-10 | 함수 크기·복잡도 달성 가능성 | Pass | §4 계획이 043, 054, 058(Web_ParseValue) 등 주요 함수를 다룸 |
| DDS-11 | 입력 검증, 오류 처리, 경계 | **Fail** (E-05, E-07, 경미) | DD-F-064 부분 커밋(제어는 전진, 응답 500), 초기 응답의 input_validity 0x1FFF(VALID)가 관측되지 않은 입력을 VALID로 보고 |
| DDS-12 | 상위·하위 추적 | **Fail** (E-02) | A-19가 "구현: Arbiter"라고 하지만 DD-F-021, DD-F-023, DD-F-024 어디에도 reason_code를 CMD_REJECTED_SPEED로 덮어쓰는 단계가 없음. 가정과 설계 함수 사이 추적이 끊김 |

### 2.3 공통 (COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| COM-01 | ID·버전·상태 | Pass | 4개 문서 ID와 버전, 상태 표기 |
| COM-02 | 형상 관리 | Pass | 커밋 5f1df0e에 포함. docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 | **Fail** | D-03 부분 조치 (E-01로 F-022 위반이 남음) |
| COM-04 | 상·하위 ID·용어 일관성 | **Fail** (E-01, E-03, E-06) | A-19와 SRS-F-022 충돌, RawInput_t 필드명 불일치, 경고코드 값 인용 오류 |

요약: ARC Pass 12 / Fail 1. DDS Pass 8 / Fail 4. COM Pass 2 / Fail 2.

## 3. 교차 점검

- **SRS-F-022 ↔ A-19 ↔ DD-F-040**: F-022는 priority_reason과 reason_code가 같은 값이어야 한다고 요구한다. A-19는 CMD_REJECTED_SPEED를 기록하려고 reason_code만 덮어쓰고 priority_reason은 결정 규칙 값을 유지한다. 이 경우 두 값이 달라지므로 F-022와 F-022 시험이 실패한다. DD-F-040은 두 요약 값을 따로 표시하므로 표시 결과도 F-022를 만족하지 않는다 (E-01).
- **A-19 ↔ DDS**: A-19는 구현 위치를 Arbiter라고 하지만 DD-F-021~024에 이 동작이 없다 (E-02).
- **RawInput_t 필드명**: ARCH.md §4의 RawInput_t는 crash_status, approach_risk[2], ignition_on, side, action, source, present를 쓰고, DDS-iter2 필드 표는 crash, approach[0..1], ignition, cmd_side, cmd_action, cmd_source를 쓴다. offsetof 기반 대입은 실제 멤버 이름을 써야 하므로 이름을 맞춰야 한다 (E-03).
- **CycleResult_t 요약 값과 ARCH-I-008/009**: priority_reason_summary, reason_code_summary, warning_code, input_validity는 ARCH.md §4와 ARCH-iter2 I-008/009, DDS DD-F-028·040·048·049에서 같은 이름으로 쓰인다 (확인).
- **이벤트 판정 ↔ SRS-F-025 ↔ A-20**: (좌/우 출력, state, reason_code_summary)로 판정하며 SRS 6.1 "결정"과 일치한다 (확인). 단 reason_code_summary의 값이 A-19 덮어쓰기에 따라 달라지므로 E-01의 영향을 받는다.
- **이벤트 링버퍼 ↔ SRS-F-021, L-006**: DD-F-049의 head와 count 갱신, DD-F-050의 (head + 용량 − count) % 용량 계산이 맞다 (확인).
- **Web 본문 파싱 ↔ SSR-007**: 파싱은 형식만 보고 값 집합과 유한성은 Iter-1 DD-F-002~005가 검증한다. NaN, Inf, 집합 밖 값은 INVALID가 된다 (확인).
- **HTTP 상태 ↔ ARCH-I-010**: 200, 400, 404, 405, 500의 조건이 DD-F-052~060과 일치한다 (확인).
- **상수 ↔ params.h/헤더**: VALIDITY_ITEM_COUNT(13), WEB_BODY_CAPACITY(16384), ROUTE_COUNT(5), HISTORY_CAPACITY(100)가 정의되어 사용된다 (확인).
- **Iter-1 DD-F-028 ↔ DDS-iter2 비트 순서**: DDS-iter2는 비트 i가 13개 항목 i에 대응한다고 "DD-F-028과 같은 순서"라고 쓰지만, Iter-1 DD-F-028 본문에는 비트 순서가 없다 (E-10).

## 4. 신규 결함

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| E-01 | 중대 | ASSUMPTIONS A-19 (행 36), SRS-F-022, SRS-F-003, DDS-iter2 DD-F-040 | A-19는 reason_code를 CMD_REJECTED_SPEED로 덮어쓰고 priority_reason은 유지한다. SRS-F-022는 reason_code가 priority_reason과 같은 값이어야 한다고 요구하고, F-022 시험도 이를 확인한다. 따라서 A-19는 SRS 요구를 위반하는 가정이다. A-19 상태가 "작성자 결정, 사용자 확인 대기"이므로 요구 편차로 승인되지 않았다. | 둘 중 하나를 택한다. (a) SRS-F-003 또는 F-022를 개정해 CMD_REJECTED_SPEED를 priority_reason과 별도 필드로 기록하도록 바꾸고 SRS 개정을 승인받는다. (b) 편차로 기록하고 사용자 승인을 받은 뒤, F-022 시험에 예외를 명시한다. 그 전까지 DD-F-040의 두 요약 값 분리 표시를 유지하면 F-022 위반이다. |
| E-02 | 중대 | ASSUMPTIONS A-19 ("구현: Arbiter"), DDS.md DD-F-021·023·024 | A-19의 구현 위치가 Arbiter이지만 Iter-1 DDS 함수 어디에도 reason_code를 CMD_REJECTED_SPEED로 덮어쓰는 단계가 없다. DD-F-024는 선택된 규칙이 없을 때만 info_reason을 쓴다. 가정과 설계 사이 추적이 끊긴다. | E-01의 결정 후 DD-F-021 또는 DD-F-023에 명령 거절 코드 기록 단계를 추가하고, A-19의 구현 위치를 함수 ID로 적는다. |
| E-03 | 중대 | DDS-iter2 필드 표 (§1, 16개), ARCH.md §4 RawInput_t 행 | 필드 표의 RawInput_t 이름(crash, approach[0..1], ignition, cmd_side, cmd_action, cmd_source)이 ARCH.md의 RawInput_t 정의(crash_status, approach_risk[2], ignition_on, side, action, source, present)와 다르다. offsetof를 쓰는 대입은 실제 구조체 멤버와 맞아야 하므로 구현이 불가능하거나 잘못된 필드에 쓸 수 있다. | ARCH.md의 RawInput_t 이름을 기준으로 필드 표를 맞추거나, 두 문서에 같은 이름표를 만든다. present의 처리(명령 존재 여부)를 필드 표에 명시한다. |
| E-04 | 경미 | ARCH.md §3 I-007 소비자 목록 "C-013 → C-014, C-016, 테스트", ARCH-iter2 §3 C-014 요구 IF "—" | ARCH.md는 C-014가 I-007을 직접 소비한다고 쓰지만 ARCH-iter2는 C-016이 결과를 전달한다고 쓴다. C-014의 요구 IF도 "—"이다. | ARCH.md I-007 소비자를 C-016으로 고치고 C-014의 요구 IF에 "I-007 출력(CycleResult_t, C-016 경유)"을 적는다. |
| E-05 | 경미 | DDS-iter2 DD-F-057 (초기 응답 input_validity 0x1FFF) | 초기 응답에서 입력 유효성을 전부 VALID로 보고한다. 아무 입력도 평가되지 않은 상태에서 VALID를 보고하는 것은 SRS 6.2 의미(평가 결과)와 맞지 않는다. | 초기 상태의 표시 값을 정하고 근거를 적는다(예: 0 또는 "NONE" 표현, 또는 가정으로 기록). |
| E-06 | 경미 | ARCH-iter2 §1 ARCH-D-10, §3 I-008 ("NONE"/"W-001"), 인용 "SRS 6.2" | 경고코드의 SRS 6.2 값은 "W-001"뿐이다. "NONE"은 SRS 6.2 경고코드 값에 없고 reason_code 집합에만 있다. "SRS 6.2" 인용이 부정확하다. | "NONE"을 SRS 6.2 경고코드 값 집합의 확장으로 기록하거나, 경고 없음을 필드 생략 또는 다른 표현으로 바꾼다. 인용을 정정한다. |
| E-07 | 경미 | DDS-iter2 DD-F-064, DD-F-059 | Control_Step이 성공한 뒤 Event_Record가 실패하면 제어 컨텍스트는 이미 전진했고 응답은 500이다. 같은 입력을 재시도하면 같은 시각을 다시 쓰게 된다. 실패 시 상태 일관성 규칙이 없다. | Control_Step과 Event_Record를 원자적으로 처리하거나(실패 시 컨텍스트 복원), 재시도 규칙(같은 시각 재사용 금지)을 정의한다. |
| E-08 | 중대 | DDS-iter2 DD-M-016 (web_handler: 라우팅, HTML 페이지, 본문 파싱, step·reset·history 응답) | 한 모듈이 여러 변경 이유를 가진다. 화면 변경, 입력 형식 변경, 경로 변경, 응답 형식 변경이 같은 모듈을 고친다. DD-F-059(단계 조합)는 함수 수준으로 나눴지만 모듈 수준 책임은 그대로다. | web_handler를 라우팅과 응답 조합(web_handler), HTML 상수(web_page), 본문 파싱(web_request_parser)으로 나누고 각 공개 헤더를 정의한다. |
| E-09 | 경미 | DDS-iter2 DD-F-057 (0x1FFF), DD-M-017 (포트 8081) | 리터럴이 상수로 정의되지 않았다. | 0x1FFF은 (1 << VALIDITY_ITEM_COUNT) − 1로 정의하고, 기본 포트를 이름 있는 상수로 정의한다. |
| E-10 | 경미 | DDS-iter2 §0 (비트 순서 "DD-F-028과 같은 순서"), DDS.md DD-F-028 | Iter-1 DD-F-028은 input_validity를 "VALID/INVALID 비트"라고만 쓰고 비트 순서를 정의하지 않는다. DDS-iter2의 "같은 순서"는 참조할 내용이 없다. | DDS.md DD-F-028에 비트 순서(13개 항목 순서 표)를 추가하거나, DDS-iter2의 순서 표를 단일 정의로 두고 두 곳에서 참조한다. |

## 5. 확인 사항 (결함 없음)

- DD-F-047/049 링버퍼 쓰기와 DD-F-050 읽기 인덱스 계산 (정상).
- DD-F-042 직렬화 실패의 HTTP 500 처리 (ARCH-I-008, SRS-F-023과 일치).
- DD-F-040의 validity 13개 목록과 SRS 6.1 "필수 입력 + gear" 목록 일치.
- 이벤트 판정 (좌/우 출력, state, reason_code_summary)과 SRS-F-025 "결정" 정의 일치 (A-20). 단 E-01의 영향을 받음.
- SSR-007 경계: 파싱된 값은 Iter-1 검증기가 평가 전에 거절한다.
- 개인정보(Q-002): 이벤트와 이력 JSON에 영상, 음성, 식별자 필드 없음.
- HTTP 상태 조건(200, 400, 404, 405, 500)과 ARCH-I-010 일치.
- ARCH.md §2 정적 구조와 §8 배정이 ARCH-iter2 §6과 일치.

## 6. 요약

- 종합 판정: **반려**
- 검사: ARC 13항목 (Pass 12, Fail 1), DDS 12항목 (Pass 8, Fail 4), COM 4항목 (Pass 2, Fail 2)
- 이전 결함(D-01~D-15): 조치 완료 14 / 부분 1 (D-03) / 미조치 0
- 신규 결함: 중대 4건 (E-01, E-02, E-03, E-08), 경미 6건 (E-04~E-07, E-09, E-10)
- 중대 결함 요지: A-19가 SRS-F-022를 위반하고 편차로 승인되지 않음 (E-01). A-19의 구현 위치가 설계에 없음 (E-02). 필드 이름이 RawInput_t 정의와 다름 (E-03). web_handler 모듈의 다중 책임 (E-08)

## 7. 판정 메모

- E-01은 요구 결정 사항이다. A-19를 SRS 개정으로 해결할지, 편차로 승인받을지 사용자 판단이 필요하다. 설계 문서만 고쳐서는 해결되지 않는다.
- E-02와 E-03은 설계 문서 정정으로 해결된다. E-01의 결정 후 진행한다.
- 재리뷰 시 D-03 부분 조치와 E-01~E-10의 조치 상태를 표로 확인하고, ARCH-iter2, DDS-iter2, ARCH.md §4, DDS.md DD-F-021·023·028, ASSUMPTIONS A-19·A-20, SRS-F-003·F-022를 함께 교차 점검한다.
