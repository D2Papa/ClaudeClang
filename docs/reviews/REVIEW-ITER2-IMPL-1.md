# 리뷰 보고서: Iter-2 구현·시험 (표시·이력·Web)

- 대상 (커밋 30e1789 및 Iter-1 변경 7b46ae5, `feature/iter-0-requirements`):
  - 소스: `src/present/*`, `src/history/*`, `src/web/*` (web_main.c 포함), `src/control/arbiter.c`, `src/control/control_cycle.c`, `src/common/types.h`
  - 단위 테스트: `test/unit/present_history_unit_test.cpp`, `test/unit/web_unit_test.cpp`, `test/unit/override_arbiter_unit_test.cpp`(변경분)
  - 통합 테스트: `test/integration/web_integration_test.cpp`
  - 시스템 테스트: `test/system/web_system_test.cpp`, `test/system/control_system_test.cpp`, `test/system/requirements_in_scope.txt`
  - TDD: `test/tdd/iter2_tdd_test.cpp`
  - 기타: `scripts/ci/req_coverage.py`, `docs/test/DEMO-web.md`, `docs/traceability/TRACE.md`
- 설계 기준: `docs/detailed-design/DDS-iter2.md` v0.5, `docs/detailed-design/DDS.md` v0.8, `docs/architecture/ARCH-iter2.md` v0.5 (설계는 승인 완료)
- 품질 게이트 증거: `reports/` (로컬 실행 산출물, gitignore). 확인한 것: `reports/lizard.xml`, `reports/cppcheck.xml`, `reports/coverage/{unit,integration,tdd,system}/summary.json`, `reports/requirements_coverage.txt`.
- 일자: 2026-11-27
- 판정: **반려**

---

## 1. 판정 근거 확인 (특히 엄격히 본 항목)

| 번호 | 확인 항목 | 결과 | 근거 |
|------|-----------|------|------|
| (1) | 단위 테스트가 DDS 기대 동작에서 독립적으로 설계되었는가 | 대체로 Pass | 기대값이 리터럴(예: `"SENSOR_FAULT"`, `"W-001"`, 고정 JSON)이고 DDS 규칙에서 도출됨. 샘플 확인: `present_history_unit_test.cpp`의 Build 매핑, 링 버퍼 100건(ID 151~250), override 경계. 단 주석에 "절차" 필드가 없음 (I-03) |
| (2) | 도달 불가 분기를 숨기려고 방어 코드를 뺐는가 | 일치 | `Web_ExecuteCycle`은 Control_Step·Event_Record 반환값으로 분기하지 않음(ARCH-D-11과 일치). `Event_Query`의 cap 검사와 `Presenter_Serialize`의 cap=0 검사는 설계에 있음. 방어 코드 누락 없음. 단 반환값을 무시하는 근거 주석이 없음 (I-01) |
| (3) | web_main.c의 단일 반환, 함수 크기, HITL, 시연 기록 | 단일 반환·크기·HITL 통과. 전역 가변 static과 반환값 무시에 문제 (I-01, I-02). 시연 기록 부족 (I-05) | 6개 함수 모두 return 1회. 최대 길이 약 40행. 함수마다 HITL 블록 존재. `static Web_t web`, `static char buf[]`, `static WebResponse_t resp`가 있음 |
| (4) | TDD와 단위 테스트 분리 | Pass | `test/tdd/iter2_tdd_test.cpp`(6건)와 단위 테스트가 별개 파일. 커버리지 리포트도 `reports/coverage/tdd`, `unit`로 분리 |
| (5) | CycleResult_t 변경이 Iter-1 문서와 일치 | Pass | `types.h`의 `priority_reason_summary`, `reason_code_summary`, `warning_code`(int), `input_validity`(unsigned bit mask)가 ARCH.md §4와 DDS.md DD-F-028 설명과 일치. `Control_Summary`는 좌가 NONE이 아니면 좌, 아니면 우 규칙 구현 |

---

## 2. 품질 증거 확인 (로컬 산출물)

- Lizard (`reports/lizard.xml`): 함수 단위 88개, 최대 CCN 8, 최대 NCSS 29. 기준(CCN 10 이하, 80라인 이하) 위반 없음. (파일 단위 항목은 기준 대상이 아니므로 제외해 계산)
- Cppcheck (`reports/cppcheck.xml`): 오류 0건. `cppcheck-suppress` 및 `cppcheck:disable` 없음 (grep 확인).
- 커버리지 (`reports/coverage/unit/summary.json`): 신규 파일 `arbiter.c`, `control_cycle.c`, `event_recorder.c`, `status_presenter.c`, `web_handler.c`, `web_page.c`, `web_request_parser.c` 모두 MC/DC 100%, 라인 100%, 함수 100%. 통합(`integration/summary.json`) 함수 커버리지도 100%.
- 요구 커버리지: `test/system/requirements_in_scope.txt`에 SRS-F-018~026, SRS-F-032, SRS-Q-002/003, SRS-L-006, SRS-C-004(시연)가 포함. 시스템 테스트 7건(web_system)이 `RecordProperty("requirements", ...)`로 태깅.
- 커버리지 제외: `CMakeLists.txt` 25~28행에서 `web_main.c`를 제품 라이브러리와 테스트에서 제외(시연 검증). 제외 목록은 CMake 주석에만 있음 (I-04).

---

## 3. 체크리스트 결과

### 3.1 소스 코드 (SRC-01~17)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| SRC-01 | 모든 함수가 반환값을 가짐 | Pass | `void` 함수 없음 (grep 확인) |
| SRC-02 | return 함수 끝에 1회 | Pass | 함수별 return 1회 (web_handler.c, status_presenter.c, event_recorder.c, arbiter.c, web_main.c 확인). 조기 반환 없음 |
| SRC-03 | 상태 코드 반환 | Pass | 공개 API 모두 Status_t 또는 정수 |
| SRC-04 | 구현이 상세 설계와 일치 | Pass | 라우팅·메서드·상태 코드(200/204/400/404/405/500), 링 버퍼 인덱스 `(head + 용량 − count) % 용량`, 덮어쓰기 규칙, 요약 값 규칙이 DDS와 일치 |
| SRC-05 | 공개 헤더로만 다른 모듈 사용 | Pass | web_handler.c가 DD-M-013/014/015 공개 헤더만 사용. 내부 함수는 static |
| SRC-06 | 입력 검증과 오류 처리 | Pass | 널 검사, cap 검사, 본문 파싱 오류 400, 값 길이 상한(`WEB_VALUE_MAX`) 검사 후 복사 (web_request_parser.c 262~273행) |
| SRC-07 | 클린 코드 | Pass | 미사용 변수 없음 (Event_Init의 `cleared`는 `*ctx = cleared`로 사용됨 확인). 중복 적음 |
| SRC-08 | Doxygen(@brief, @param, @return, @trace) | Pass | 함수마다 존재 |
| SRC-09 | 경고 없이 빌드 | Pass | 빌드 성공 보고 (로컬 게이트 결과). 빌드 로그는 저장소에 없음 |
| SRC-10 | 데드 코드·주석 처리 코드 없음 | Pass | 확인 범위 내 없음 |
| SRC-11 | 반환값 확인 또는 무시 사유 주석 | **Fail** (I-01) | 아래 목록의 반환값 무시에 사유 주석이 없음 |
| SRC-12 | 소스 → DD 추적 | Pass | 함수 주석 `@trace DD-F-0xx` 존재 |
| SRC-13 | HITL 정보 | Pass | 모든 파일과 함수에 `@par AI 작성 정보`, `@par 사용자 리뷰`(미완료) |
| SRC-14 | 사용자 리뷰 완료 표시 무결성 | Pass | 전부 "미완료" |
| SRC-15 | Cppcheck 0건, 예외 처리 없음 | Pass | 1장 2절 |
| SRC-16 | Lizard 기준 | Pass | 2절 |
| SRC-17 | TDD 테스트 분리와 별도 결과·커버리지 | Pass | 3장 |

반환값 무시 목록 (I-01): `web_handler.c` 305·307행(Control_Step, Event_Record), 336·337행(Web_ExecuteCycle, Web_RespondStatus), 266행(Event_Query), 396~412행(Web_Dispatch의 응답 함수들), 367·369행(Web_Init), 445행(Event_Init), 490행(Web_Dispatch), 482·486행(Web_SetText); `web_request_parser.c` 224행(Web_ParseValue), 323행(Web_DefaultRaw); `status_presenter.c` 214~218행(Presenter_Name 5회), 228행(Presenter_Build); `event_recorder.c` 117행(Event_Append); `web_main.c` 122행(Adapter_ReadBody), 170·171행(send), 201행(setsockopt), 231행(Web_Init), 248행(close), 244행(Adapter_Serve); Iter-1 `control_cycle.c` 170~172, 287~293, 308~309행, `arbiter.c` 234행.

### 3.2 단위 테스트 (UTS-01~13)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| UTS-01 | MC/DC 100%, 성공률 100% | Pass | `summary.json` 신규 파일 MC/DC 100% (로컬 게이트 결과) |
| UTS-02 | LLVM 증거, TDD와 분리 측정 | Pass | `reports/coverage/unit`, `reports/coverage/tdd` 분리 |
| UTS-03 | 조건 독립성 입증 | Pass | MC/DC 100%로 확인 |
| UTS-04 | Doxygen에 목적·입력·절차·예상 출력 | **Fail** (I-03) | 테스트 주석은 "목적, 입력, 기대, 추적, 기법"만 있고 절차 항목이 없다 (`grep "절차"` 0건) |
| UTS-05 | 예상 출력이 설계·요구에서 도출 | Pass | 리터럴 기대값, DDS 규칙과 대조 (1장 (1) 확인) |
| UTS-06 | 의미 있는 assertion | Pass | 샘플 확인 |
| UTS-07 | 경계·동치·PICT 기법 기록 | Pass | `기법:` 항목 기록 (예: "오류 추정(MC/DC)", "동치") |
| UTS-08 | Stub/Mock 남용 없음 | Pass | Mock 사용 없음 |
| UTS-09 | 무효 입력·오류 경로 | Pass | Null, 미초기화, cap 부족 테스트 존재 |
| UTS-10 | 커버리지 예외 처리 없음 | **Fail** (I-04) | `web_main.c`가 CMake에서 제품 라이브러리·테스트에서 제외되어 커버리지 측정 대상이 아니다. 예외 목록(제외 주석)이 없고 근거는 CMake 주석 한 줄뿐이다 |
| UTS-11 | 추적 (테스트 → DD/SRS) | Pass | `@trace`, 시스템 `RecordProperty` |
| UTS-12 | 독립성 (순서·공유 상태 무의존) | Pass | 픽스처 SetUp에서 초기화 (확인 범위) |
| UTS-13 | 결과 기록과 실패 조치 | Pass | `reports/test/unit.xml` 등 기록 |

### 3.3 통합 테스트 (ITS-01~12)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| ITS-01 | 통합 전략·순서와 근거가 테스트 계획에 있음 | **Fail** (I-06, 경미) | `docs/test/`에 통합 테스트 계획 문서가 없다 (`DEMO-web.md`만 있음) |
| ITS-02 | 함수 커버리지 100%, 인터페이스 커버리지 표 | **Fail** (I-07) | 함수 커버리지 100%는 확인. 인터페이스 커버리지 표가 없다 (`grep` 0건) |
| ITS-03 | 인터페이스별 정상·경계·무효·오류 전파 케이스 | Pass | ARCH-I-008(Serialize 500), ARCH-I-009(Query cap), ARCH-I-010(라우트) 케이스 존재 (통합 5건 + 단위) |
| ITS-04 | 인터페이스 중심 | Pass | Web_Handle을 통해 세 컴포넌트를 조합 |
| ITS-05 | Doxygen에 목적·입력·절차·예상 | **Fail** (I-03) | UTS-04와 같음. 통합 주석의 `IT-020 목적 … 입력 …`에 절차 없음 |
| ITS-06 | 예상 출력이 인터페이스 정의에서 도출 | Pass | ARCH-I-010 경로 표와 일치 |
| ITS-07 | 의미 없는 케이스 없음 | Pass | 확인 범위 내 |
| ITS-08 | Stub/Mock은 외부 대상에만 | Pass | Stub 없음 |
| ITS-09 | 경계·동치·PICT 기법 기록 | Pass | 기법 기록 |
| ITS-10 | 호출 순서·시나리오 테스트 | Pass | StepJson 시나리오(접근위험 → unlock → OVERRIDE) |
| ITS-11 | 추적 (테스트 → ARCH-I/SRS) | Pass | `@trace ARCH-I-007~010` |
| ITS-12 | 결과 기록과 실패 조치 | Pass | `reports/test/integration.xml` |

### 3.4 시스템 테스트 (SWE.6 관점, 요구 합격 기준 정합·추적성)

| 항목 | 판정 | 근거 |
|------|------|------|
| 요구 합격 기준과 시험 정합 | Pass | ST-101(SRS-F-018/019/022, 평가 전 204), SerializationFailure(F-023 500), 링 버퍼(F-020/021/024/025/026, L-006), ReasonOverlay(F-032, F-003, F-022), NoPersonalDataFields(Q-002), InterfaceValueSets(Q-003) |
| 추적성 (테스트 ↔ 요구) | Pass | 각 테스트 `RecordProperty("requirements")`와 `requirements_in_scope.txt`가 일치 |
| 시연 (SRS-C-004) | **Fail** (I-05, 경미) | 검사 항목 `inspect-present docs/test/DEMO-web.md 시연 결과: 성공`은 기록 문서 안의 문자열만 확인한다. 시연 기록에는 실행 명령(curl), 날짜·시각·커밋 해시, 전체 응답이 없고 발췌가 잘려 있다 |

---

## 4. 결함 목록

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| I-01 | 중대 | 3.1절 목록 (web_handler.c, web_request_parser.c, status_presenter.c, event_recorder.c, web_main.c, Iter-1 control_cycle.c, arbiter.c) | 반환값을 확인하지 않고 무시하면서 사유 주석이 없다 (SRC-11). 설계(ARCH-D-11)는 Control_Step·Event_Record 등이 Web 경로에서 오류를 내지 않는다는 근거를 제시하지만 코드에는 근거 주석이 없다. Iter-1 control_cycle.c와 arbiter.c도 같은 패턴이다. | 무시하는 호출마다 `/* 반환값 무시: <사유, ARCH-D-11 등> */` 주석을 남기거나, 반환값을 확인해 `STATUS_OK`가 아니면 500 등으로 처리한다. Presenter_Name은 항상 STATUS_OK이므로 사유를 주석에 쓴다. |
| I-02 | 중대 | web_main.c 225~, 144~ (`static Web_t web`, `static char buf[REQUEST_CAPACITY]`, `static WebResponse_t resp` in Adapter_Serve) | DDS-iter2 §0은 "전역 가변 데이터 없음"을 공통 규칙으로 정했다. web_main.c는 정적 기억 영역의 가변 객체 세 개를 쓴다. Adapter_Serve의 `static resp`는 호출 간 상태를 공유한다. | 세 객체를 main의 자동 변수로 두고 Adapter_* 함수에 포인터로 넘긴다. 함수 시그니처에 저장소를 명시한다. |
| I-03 | 중대 | test/unit/*.cpp, test/integration/web_integration_test.cpp, test/tdd/iter2_tdd_test.cpp (주석 블록) | UTS-04, ITS-05는 Doxygen 주석에 목적, 입력값, 절차, 예상 출력값을 요구한다. 테스트 주석은 목적·입력·기대·추적·기법만 있고 절차가 없다. | 각 테스트 주석에 "절차: 1) … 2) …" 항목을 추가한다. 절차는 Arrange/Act/Assert 순서로 쓴다. |
| I-04 | 중대 | CMakeLists.txt 25~28행 (web_main.c 제외) | web_main.c는 MC/DC 측정 대상이 아니다. 설계(DD-M-017)는 진입점으로 기준 미만이라 단위 테스트를 요구하지 않는다는 근거를 준다. 그러나 CLAUDE.md 2.1 기준(단위 MC/DC 100%)과 UTS-10(예외 목록)에서 제외는 기록된 예외 목록에 있어야 한다. 현재 근거는 CMake 주석뿐이다. | 제외를 `docs/test/`의 커버리지 예외 목록(파일, 사유, 설계 근거, 대체 검증 수단)으로 기록한다. 또는 web_main의 로직(인자 파싱, 요청 읽기)을 테스트 가능한 함수로 옮기고 나머지만 제외한다. 사용자 판단 권장. |
| I-05 | 경미 | docs/test/DEMO-web.md | 시연 기록에 실행 명령, 날짜·시각, 커밋 해시, 전체 응답이 없고 발췌가 잘렸다("source_timestam", "warni" 등). 검사 항목은 기록 문서의 문자열만 본다. | curl 명령 6종과 전체 응답을 붙이고 커밋 해시와 실행 시각을 기록한다. 검사 정규식은 "시연 결과: 성공" 대신 응답 필드를 확인하도록 강화한다. |
| I-06 | 경미 | docs/test/ (통합 테스트 계획 부재) | ITS-01: 통합 전략·순서와 근거를 적은 계획이 없다. | docs/test/에 통합 테스트 계획(순서: Presenter/Event 단위 → Web 조합 → 시연)을 작성한다. |
| I-07 | 중대 | docs/test/ (인터페이스 커버리지 표 부재) | ITS-02: 인터페이스 커버리지 표가 없다. ARCH-I-008~010, DD-I-011~012의 시험 대응을 한눈에 확인할 수 없다. | 인터페이스 → 통합 테스트 ID → 커버리지 결과 표를 작성한다. |
| I-08 | 경미 | src/web/web_main.c Adapter_Serve (`path[64]`, `%63s`) | 경로가 63바이트를 넘으면 잘린 경로로 처리되어 잘못된 404 또는 다른 라우트가 될 수 있다. 설계에 길이 제한 규칙이 없다. | 긴 경로는 414 등으로 거절하는 규칙을 DD-M-017에 추가한다. |

---

## 5. 판정 메모

- 로직과 설계 일치, 방어 코드 누락, 단일 반환, 함수 크기, Cppcheck, Lizard, 추적 태그는 확인되었다. 품질 게이트 수치(단위 MC/DC 100%, 통합 함수 100%, 시스템 요구 52/52)는 로컬 리포트와 일치한다.
- 반려 사유는 SRC-11(반환값 주석, I-01), 전역 가변 static(I-02), 테스트 절차 항목 누락(I-03, UTS-04/ITS-05), 커버리지 제외 근거(I-04), 인터페이스 커버리지 표(I-07)다.
- I-01은 Iter-1 제어 코어(control_cycle.c, arbiter.c)에도 같은 패턴이 있다. 이번 이터레이션 변경 범위에서 함께 정리한다.
- 사용자 판단 권장: I-04(web_main 커버리지 제외를 예외로 받을지, 로직을 분리해 측정 대상으로 넣을지).
- 재리뷰 시 I-01~I-08의 조치 상태를 표로 확인하고, 수정으로 생긴 변경(주석 추가, web_main 자동 변수 전환, 테스트 주석 절차 추가)이 설계와 여전히 맞는지 교차 점검한다.

---

## 6. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| Iter-2 구현·시험 (소스, 단위, 통합, 시스템, TDD) | **반려** | 중대 5 (I-01, I-02, I-03, I-04, I-07), 경미 3 (I-05, I-06, I-08) |

- 조치 필요 중대 항목: I-01(SRC-11), I-02(전역 가변), I-03(UTS-04/ITS-05 절차), I-04(커버리지 제외 기록), I-07(인터페이스 커버리지 표)
- 조치 필요 경미 항목: I-05(시연 기록), I-06(통합 계획), I-08(경로 길이)
- 다음 단계 진행: **불가**. 모든 부적합 조치 후 재리뷰가 필요하다.
