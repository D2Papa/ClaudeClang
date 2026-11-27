# 리뷰 보고서: Iter-2 설계 3차 재리뷰 (표시·이력·Web)

- 대상 (커밋 041ca62, `feature/iter-0-requirements`):
  - `docs/architecture/ARCH-iter2.md` (DOC-ARC-002 v0.3)
  - `docs/detailed-design/DDS-iter2.md` (DOC-DDS-002 v0.3)
  - 개정 Iter-1 문서: `docs/architecture/ARCH.md` (헤더 v0.2, RawInput_t 행 변경), `docs/detailed-design/DDS.md` (v0.3, DD-F-021·028·029, 비트 순서 표)
  - 요구·가정: `docs/requirements/SRS.md` (헤더 v1.0, SRS-F-003·F-022 변경), `docs/requirements/ASSUMPTIONS.md` (헤더 v1.0, A-19 승인, A-21 추가)
- 이전 리뷰: REVIEW-ITER2-DESIGN-1.md (D-01~D-15), REVIEW-ITER2-DESIGN-2.md (E-01~E-10)
- 산출물 종류: 아키텍처 설계(ARC-01~13), 상세 설계(DDS-01~12), 공통(COM-01~04)
- 일자: 2026-11-27
- 종합 판정: **반려**

## 1. 이전 결함 조치 상태

### 1.1 2차 결함 (E-01~E-10)

| 번호 | 심각도 | 2차 내용 | 조치 상태 | 근거 / 잔여 |
|------|--------|----------|-----------|-------------|
| E-01 | 중대 | A-19가 SRS-F-022를 위반하고 편차 승인이 없음 | 조치 완료 (요구 변경 절차 문제는 G-01) | 사용자 선택 "F-022에 예외 추가 (추천)"가 A-19 행과 편차·승인 기록에 인용됨. SRS-F-003·F-022에 CMD_REJECTED_SPEED 예외 반영 |
| E-02 | 중대 | A-19 구현 위치가 설계에 없음 | 조치 완료 | DDS.md DD-F-029 Arbiter_IsSpeedRejected 추가, DD-F-021이 DD-F-029로 reason_code를 덮어씀, DDS-iter2 §5 SRS-F-003, SRS-F-022 배정 확인 |
| E-03 | 중대 | RawInput_t 필드명 불일치 | 조치 완료 | ARCH.md §4 RawInput_t 행을 crash, approach[2], ignition, cmd_side/action/source로 변경. DDS-iter2 필드 표와 일치 (확인) |
| E-04 | 경미 | I-007 소비자 목록과 C-014 요구 IF 불일치 | **부분 조치** | ARCH-iter2 §3 C-014 요구 IF 정정 확인. 그러나 ARCH.md §4 ARCH-I-007 행은 여전히 "C-013 → C-014, C-016, 테스트" (G-02) |
| E-05 | 경미 | 초기 응답의 VALID 보고 | 조치 완료 (관련 새 결함 G-04) | ARCH-D-12와 DD-F-057: 초기 입력 유효성 전부 INVALID (mask 0) |
| E-06 | 경미 | 경고 "NONE" 인용 오류 | 조치 완료 | ARCH-D-10과 A-21로 설계 표시값임을 명시. A-21은 사용자 확인 대기 (G-06 참조) |
| E-07 | 경미 | Control_Step 성공 후 Event_Record 실패 | 조치 완료 | ARCH-D-11: Web 경로의 하위 호출 오류 불가 불변식, 500은 본문 상한 초과 직렬화에서만. DD-F-064 항상 STATUS_OK. 본문 상한 불변식은 G-05 |
| E-08 | 중대 | web_handler 다중 책임 | 조치 완료 | web_handler(DD-M-016), web_page(DD-M-018, DD-I-011, DD-F-070), web_request_parser(DD-M-019, DD-I-012, DD-F-063) 분할 확인 |
| E-09 | 경미 | 리터럴 | 조치 완료 | WEB_DEFAULT_PORT(8081) 상수화, DD-F-057의 0x1FFF 제거 (input_validity 0) 확인 |
| E-10 | 경미 | 비트 순서 미정의 | 조치 완료 | DDS.md "input_validity 비트 순서 (DD-F-028, 단일 정의)" 표 0~12 확인. 13개 항목 순서가 DDS-iter2 §1 목록과 일치 |

2차 결함 10건: 조치 완료 8, 부분 조치 1 (E-04), 미조치 0.

### 1.2 이전 1차 결함 (D-01~D-15)

D-01~D-15는 2차 리뷰에서 조치 완료 또는 2차 결함으로 이관되었다. D-03의 잔여(F-022 값 관계)는 E-01 경로로 해소되었다. 회귀는 확인되지 않았다.

## 2. 체크리스트 결과

### 2.1 아키텍처 (ARC-01~13)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| ARC-01 | 컴포넌트와 한 문장 책임 | Pass | ARCH-iter2 §3의 C-014, C-015, C-016 (C-016은 모듈 구성을 괄호로 덧붙였으나 책임 문장은 하나) |
| ARC-02 | 제공·요구 인터페이스 | Pass | C-014 요구 IF: CycleResult_t는 호출자가 넘기는 데이터이며 I-007을 호출하지 않는다고 명시. C-016 요구 IF: I-007, I-008, I-009 |
| ARC-03 | 인터페이스 정의 완전성 | **Fail** (G-03, 경미) | ARCH-I-010 오류 처리 목록이 "응답 본문 상한을 넘는 직렬화 실패 500"만 기재하지만 DD-F-056은 화면 본문 길이가 상한보다 크면 500을 낸다 |
| ARC-04 | 인터페이스로만 통신 | Pass | C-016만 C-013, C-014, C-015를 호출 |
| ARC-05 | 순환 의존 없음 | Pass | §2 정적 구조 |
| ARC-06 | 응집도 | Pass | 표시, 이력, 조합 분리 |
| ARC-07 | 결합도 | Pass | 공유 가변 데이터 없음 |
| ARC-08 | 캡슐화 | Pass | 소유 컴포넌트 소스에서만 접근 |
| ARC-09 | SOLID | Pass | S, D 근거 명시 |
| ARC-10 | 정적·동적 구조 | Pass | §2 정적 구조, §5 시퀀스 4개 |
| ARC-11 | 비기능 고려 | Pass | Q-002, Q-003, L-006 근거 |
| ARC-12 | 요구 배정과 추적 | Pass | §6 배정 SRS-F-018~026 등. 폐기 ID F-011~013 배정 없음 (확인) |
| ARC-13 | 설계 결정 근거 | **Fail** (G-04, 경미) | ARCH-D-12가 "입력 유효성 전부 INVALID"와 함께 state NORMAL을 정하지만 근거가 "아직 입력이 관측되지 않음" 하나뿐이다. SSR-015는 필수 입력이 INVALID이면 DEGRADED라고 정하므로 같은 응답 안에서 두 규칙이 충돌한다 |

### 2.2 상세 설계 (DDS-01~12)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| DDS-01 | 컴포넌트의 모듈 분해와 책임 | Pass | DD-M-014~019와 DD-M-017 진입점 |
| DDS-02 | 인터페이스 정의 기준 | Pass | §0 기준과 DD-M-017 제외 명시 |
| DDS-03 | 기준 이상 모듈의 인터페이스 | Pass | DD-M-014~016, 018, 019에 인터페이스 정의 |
| DDS-04 | 모듈 인터페이스와 ARCH-I 일치 | Pass | DD-I-008~010 시그니처 일치. DD-I-011(WebPage_Get), DD-I-012(WebParser_ParseBody)는 컴포넌트 내부 |
| DDS-05 | 공개 인터페이스로만 의존 | Pass | web_handler는 DD-M-013, 014, 015, 018, 019의 공개 헤더만 사용 |
| DDS-06 | 반환값과 void 금지 | Pass | 모든 함수 Status_t 또는 정수. DD-F-041은 출력 인자 |
| DDS-07 | 단일 반환 | Pass | §0 공통 규칙 명시 |
| DDS-08 | 단일 책임 | Pass | 모듈 3분할과 함수 분리 (DD-F-059는 순서만 조합하는 오케스트레이션) |
| DDS-09 | 이름·매직 넘버 | Pass | WEB_BODY_CAPACITY, WEB_DEFAULT_PORT, VALIDITY_ITEM_COUNT, ROUTE_COUNT 정의. 리터럴 잔여 없음 |
| DDS-10 | 함수 크기·복잡도 달성 가능성 | Pass | §4 계획 (Presenter_Write, Web_Handle, Web_RespondStep, Web_ParseValue, Web_ApplyField) |
| DDS-11 | 입력 검증, 오류 처리, 경계 | **Fail** (G-05, 경미) | 본문 상한 Web_t.body_limit이 WEB_BODY_CAPACITY를 넘지 않는다는 불변식이 없다. DD-F-051은 기본값만 정하고 상한 조건을 명시하지 않는다. 버퍼 넘침 위험 |
| DDS-12 | 상위·하위 추적 | Pass | DD-F-029 → SRS-F-003, SRS-F-022 (DDS-iter2 §5, DDS.md §5). 하위 추적은 구현 단계 |

### 2.3 공통 (COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| COM-01 | ID·버전·상태 | **Fail** (G-06, 경미) | ARCH.md는 내용(RawInput_t)이 바뀌었으나 헤더 버전이 0.2 그대로. ASSUMPTIONS는 A-19 승인과 A-21 추가 후에도 헤더가 1.0. SRS는 F-003, F-022 변경 후에도 헤더가 1.0 (G-01) |
| COM-02 | 형상 관리 | Pass | 커밋 041ca62. `git status --short docs/` 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | **Fail** (E-04 부분 조치) | E-04의 ARCH.md I-007 정정 누락 |
| COM-04 | 상·하위 ID·용어 일관성 | **Fail** (G-02, G-01) | ARCH.md I-007 소비자가 C-014를 가리키고 ARCH-iter2와 다름. SRS-F-003·F-022의 상태 열이 정의되지 않은 값 "리뷰됨"을 가짐 |

요약: ARC Pass 11 / Fail 2. DDS Pass 11 / Fail 1. COM Pass 1 / Fail 3.

## 3. 교차 점검

- **SRS-F-022 예외 ↔ SRS-F-003 ↔ DD-F-021·029 ↔ A-19**: F-003은 unlock 거절 시 reason_code를 CMD_REJECTED_SPEED로 기록하고 priority_reason은 결정 규칙 값을 유지한다고 쓴다. F-022는 같은 예외를 쓴다. DD-F-029는 driver info_reason[d]가 CMD_REJECTED_SPEED이고 운전자 순위가 제외되지 않았을 때 1을 반환하며, DD-F-021이 이때 reason_code를 덮어쓴다. A-19 문장과 일치한다. 다만 F-003·F-022 변경은 승인된 SRS 1.0의 변경이다 (G-01).
- **DD-F-029 ↔ SSR-010/011/F-031**: 운전자 순위가 state로 제외되면 DD-F-029는 0이다. 제외된 명령은 CMD_REJECTED_SPEED로 기록되지 않는다. F-003·F-031의 의도와 맞다.
- **요약 값 ↔ F-022**: priority_reason_summary와 reason_code_summary는 같은 규칙(좌 우선)을 쓴다. 예외 조건을 제외하면 두 값이 같다. DD-F-021이 reason_code를 덮어쓰는 경우는 예외 조건뿐이다. F-022와 일치한다.
- **비트 순서 ↔ DD-F-040 ↔ DDS-iter2 §1**: 13개 항목 순서가 DDS.md 단일 표, DDS-iter2 §1 목록, DD-F-040 본문에서 같다 (확인).
- **RawInput_t ↔ 필드 표 ↔ offsetof**: ARCH.md §4 RawInput_t 이름(speed, gear, source_timestamp, crash, approach[2], fire, overtemp, adult, isofix[2], ignition, sensor_fault, cmd_side, cmd_action, cmd_source)과 DDS-iter2 필드 표 16항목이 일치한다 (E-03 확인).
- **ARCH-I-007 소비자 ↔ ARCH-iter2 호출 구조**: ARCH.md는 C-014를 소비자로 쓰고 ARCH-iter2는 C-016이 결과를 전달한다고 쓴다 (G-02).
- **초기 응답 ↔ SSR-015**: GET /status 초기 응답은 state NORMAL과 입력 유효성 전부 INVALID를 함께 준다. SSR-015 규칙으로는 이 조합이 나오지 않는다 (G-04).
- **Web 경로 500 ↔ ARCH-D-11 ↔ DD-F-056·057·058**: 500은 DD-F-056(화면 길이 초과), DD-F-057(직렬화 실패), DD-F-058(이력 쓰기 실패)에서 발생한다. ARCH-I-010의 목록은 직렬화 실패만 적었다 (G-03).
- **하위 호출 오류 불가 ↔ DD-F-064**: 제어 주기 호출 오류가 없다는 불변식은 Web_Init의 초기화, 단조 시각, cycle 증가에 기대므로 설계 안에서 성립한다 (확인). 본문 상한 불변식만 빠져 있다 (G-05).
- **수치와 상수**: L-006 HISTORY_CAPACITY(100), WEB_BODY_CAPACITY(16384), WEB_DEFAULT_PORT(8081), VALIDITY_ITEM_COUNT(13) 사용 확인.
- **SRS-F-019 ↔ A-21**: SRS-F-019는 6.2 열거형 값으로 표시하라고 쓰는데 "NONE"은 경고코드 6.2 값 집합 밖이다. A-21로 설계 표시값을 정하고 사용자 확인을 기다린다. 이 상태는 A-13~A-18과 같은 절차이므로 결함으로 보지 않는다. 다만 확인 전까지 설계가 이 값에 의존한다는 점을 기록한다.

## 4. 신규 결함

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| G-01 | 중대 | SRS.md 헤더 (v1.0, 9차 리뷰 반려 조치본), SRS-F-003·F-022 행 (상태 열 "리뷰됨") | 승인된 요구 기준선(SWE.1 승인, v1.0)에서 F-003과 F-022의 내용이 바뀌었는데 버전이 올라가지 않았고 SWE.1 재리뷰를 거치지 않았다. 상태 열의 "리뷰됨"은 상태 체계에 정의되지 않은 값이며 재리뷰 결과가 아니다. CLAUDE.md 1.4는 승인 후 수정된 산출물을 다시 리뷰하도록 한다. DD-F-021·029가 이미 이 변경에 의존한다. | SRS를 v1.1로 올리고 F-003, F-022 상태를 "초안 (재리뷰 대기)"로 되돌린다. 상태 열 값 "리뷰됨"을 쓰지 않는다. aspice-reviewer로 SWE.1 재리뷰를 받은 뒤 설계를 확정한다. |
| G-02 | 경미 | ARCH.md §4 ARCH-I-007 행 ("C-013 → C-014, C-016, 테스트"), ARCH-iter2 §3 C-014 요구 IF | E-04의 ARCH.md 쪽 정정이 빠졌다. ARCH.md는 C-014가 I-007을 소비한다고 쓴다. ARCH-iter2는 C-016이 결과를 전달한다고 쓴다. | ARCH.md ARCH-I-007의 소비자를 "C-016(테스트 포함)"으로 고치고 헤더 버전을 올린다. |
| G-03 | 경미 | ARCH-iter2 §4 ARCH-I-010 오류 처리 ("응답 본문 상한을 넘는 직렬화 실패 500"), DDS-iter2 DD-F-056·058 | 화면 본문(DD-F-056)과 이력 쓰기(DD-F-058)의 500 조건이 ARCH-I-010 목록에 없다. 인터페이스 오류 목록이 설계 함수와 맞지 않는다. | ARCH-I-010 오류 처리를 "응답 본문 상한 초과(화면, 상태, 이력 직렬화 포함)"로 고친다. |
| G-04 | 경미 | ARCH-iter2 ARCH-D-12, DDS-iter2 DD-F-057 (초기 응답) | 초기 응답이 state NORMAL과 입력 유효성 전부 INVALID를 함께 준다. SSR-015에서 필수 입력 INVALID는 DEGRADED를 뜻하므로 이 조합은 평가 결과로 나올 수 없다. ARCH-D-12의 근거가 "입력 미관측" 하나뿐이다. | 초기 응답을 "평가 전" 상태로 표시하는 방식(예: state 값 없음 또는 별도 값)을 정하거나, 근거를 "평가 전 표시 값"으로 명시하고 SSR-015와의 관계를 적는다. 사용자 확인이 필요하면 가정으로 기록한다. |
| G-05 | 경미 | ARCH-iter2 ARCH-D-11, DDS-iter2 §1 Web_t (body_limit), DD-F-051 | body_limit가 WEB_BODY_CAPACITY를 넘지 않는다는 조건이 없다. 응답 버퍼가 고정 크기이므로 상한이 커지면 넘침이 생긴다. | Web_t 설명과 DD-F-051에 "0 < body_limit ≤ WEB_BODY_CAPACITY"를 명시하고 Web_Init 사전 조건으로 둔다. |
| G-06 | 경미 | ARCH.md 헤더 (v0.2), ASSUMPTIONS.md 헤더 (v1.0) | 내용이 바뀌었는데 버전 표기가 그대로다. ARCH.md의 RawInput_t 변경(E-03)과 ASSUMPTIONS의 A-19 승인·A-21 추가가 버전에 반영되지 않았다. | ARCH.md를 v0.3, ASSUMPTIONS를 v1.1로 올린다. (SRS는 G-01에서 처리.) |
| G-07 | 경미 | DDS-iter2 DD-F-062 다음 DD-F-063, DD-F-070 (ID 순서) | DD-F-061이 없고 DD-F-070이 DD-M-018에 먼저 등장하는 등 ID 번호가 순서대로 붙어 있지 않다. 추적 시 혼란을 준다. | 번호를 연속으로 다시 붙이거나, 결번을 "결번"으로 표기한다. |

## 5. 확인 사항 (결함 없음)

- E-01~E-03, E-05~E-10의 조치 확인 (1.1 표).
- DD-F-021·029와 SRS-F-003·F-022 예외 조건의 일치.
- 요약 값 규칙과 F-022 (예외 제외 시 일치).
- 13개 입력 유효성 비트 순서 단일 정의와 DD-F-040의 사용.
- RawInput_t 이름 정합 (ARCH.md ↔ DDS-iter2 필드 표 ↔ offsetof).
- 링버퍼 인덱스 계산 (DD-F-049·050 정확).
- DD-F-057, DD-F-058 직렬화 실패 500과 ARCH-I-008 일치.
- SSR-007 경계 (파싱 → Iter-1 검증기 집합·유한성 검사).
- Q-002 (필드 없음), Q-003 (이름과 값 집합), L-006 (HISTORY_CAPACITY).
- 사용자 선택(A-19 "F-022에 예외 추가")의 기록 존재 확인. 승인 사실 자체는 결함으로 보지 않음.

## 6. 요약

- 종합 판정: **반려**
- 검사: ARC 13항목 (Pass 11, Fail 2), DDS 12항목 (Pass 11, Fail 1), COM 4항목 (Pass 1, Fail 3)
- 2차 결함(E-01~E-10): 조치 완료 8 / 부분 1 (E-04) / 미조치 0
- 신규 결함: 중대 1건 (G-01), 경미 6건 (G-02~G-07)
- 중대 결함 요지: 승인된 SRS 1.0의 F-003·F-022 변경이 버전 상향과 SWE.1 재리뷰 없이 반영됨, "리뷰됨"은 미정의 상태

## 7. 판정 메모

- G-01은 설계 문서 수정이 아니다. SRS를 v1.1로 올리고 SWE.1 재리뷰를 받아야 한다. 재리뷰 결과에 따라 DD-F-021·029를 확정한다.
- G-02~G-07은 문서 정정으로 해결된다.
- 재리뷰 시 E-04 부분 조치와 G-01~G-07의 조치 상태를 표로 확인하고, ARCH-iter2, DDS-iter2, ARCH.md §4·§8, DDS.md DD-F-021·028·029, SRS-F-003·F-022, ASSUMPTIONS A-19·A-21을 함께 교차 점검한다.
