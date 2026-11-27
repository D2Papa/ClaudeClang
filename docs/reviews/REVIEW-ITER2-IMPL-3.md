# 리뷰 보고서: Iter-2 설계 재리뷰(N-01) 및 구현·시험 3차 리뷰

| 항목 | 내용 |
|------|------|
| 리뷰 ID | REVIEW-ITER2-IMPL-3 |
| 리뷰 일자 | 2026-11-27 |
| 리뷰어 | aspice-reviewer (엄격 모드) |
| 대상 커밋 | ee4e8ad (feature/iter-2-presentation) |
| 대상 산출물 | (1) docs/architecture/ARCH-iter2.md v0.6, docs/detailed-design/DDS-iter2.md v0.6 (설계 재리뷰) / (2) src/web/*, src/control/*, test/unit, test/integration, test/system, test/tdd, docs/test/* (구현·시험) |
| 적용 체크리스트 | .claude/skills/review/checklists.md (ARC-01~13, DDS-01~12, SRC-01~17, UTS-01~13, ITS-01~12, SWE.6, COM-01~04) |
| 부분별 판정 | 설계: **반려** / 구현·시험: **승인** |

---

## 1. 이전 결함 조치 상태

### 1.1 설계 (N-01)

| ID | 이전 결함 | 조치 상태 | 근거 |
|----|-----------|-----------|------|
| N-01 | 구현이 설계와 불일치 (web_adapter 모듈 미정의, 소켓 루프 위치 불일치) | **부분 조치** | DD-M-020(web_adapter), DD-I-013, DD-F-071~076 추가 확인 (DDS-iter2.md). DD-M-017 main-only 확인. DD-F-073 loopback 설명이 `src/web/web_adapter.c`의 `INADDR_LOOPBACK`과 일치. 그러나 ARCH-iter2.md에 web_adapter 분리 이전의 서술이 남아 있음 (아래 D-01, D-02). |

### 1.2 구현·시험 (I-01 ~ I-08 중 2차 잔여분 N-01 ~ N-04)

| ID | 이전 결함 | 조치 상태 | 근거 |
|----|-----------|-----------|------|
| I-01 | 반환값 무시 호출에 사유 주석 없음 | **조치 완료** | 반환값 무시 호출에 `반환값을 쓰지 않는 호출:` 사유 주석 확인. web_adapter.c:165-166 (`close(server)`), web_adapter.c:189-190 (`close(fd)`), web_main.c:49-50 (`WebAdapter_Run`), approach_risk_rule.c:68-69 (loop 내부), 그 외 Iter-1 모듈 arbiter.c:233-234, control_cycle.c:170/178/289/294/312, driver_command_rule.c:86, input_validator.c:195/218에도 동일 형식 주석 확인 (grep 기준). |
| I-03 | 단위 테스트 절차 주석 누락 | **조치 완료** | web_adapter_unit_test.cpp: `TEST(` 10건, `절차:` 10건 일치. |
| N-01 (구현 측) | 구현 측 설계 불일치 | **조치 완료** | 구현은 ARCH/DDS 모듈 구성(web_adapter 분리, web_main 얇은 진입점)과 일치. 설계 측 잔여는 위 1.1 참조. |
| N-02 | 소켓 어댑터 public 함수 Doxygen 블록 누락 | **조치 완료 (수용 근거 확인)** | 아래 2.2 참조. Iter-1 관례 (헤더 Doxygen, .c에는 정의 블록 없음)와 일치. |
| N-03 | 전역 가변 데이터 존재 | **조치 완료** | web_main.c의 `Web_t`는 main 내부 자동 변수. web_adapter.c에 전역 가변 객체 없음 (grep 확인). |
| N-04 | 시험 간 자원 누수 (RLIMIT 등) | **조치 완료** | web_adapter_unit_test.cpp의 `NofileGuard` RAII 가드 확인, 사용 위치 확인. |

---

## 2. 설계 재리뷰 (N-01)

### 2.1 체크리스트 판정

| 항목 | 판정 | 근거 |
|------|------|------|
| ARC-01~13 (구조, 인터페이스, 응집·결합) | Fail | 아래 D-01, D-02 |
| DDS-01~12 (상세 설계, 모듈 정의, 추적) | Fail | 아래 D-03 |
| COM-01~04 | Pass | HITL 표기 및 Doxygen 주석 규칙은 설계 문서 범위에서 해당 없음 또는 충족 |

### 2.2 N-02 결정에 대한 명시적 판단

N-02는 "소켓 어댑터 public 함수(`WebAdapter_ReadRequest`, `WebAdapter_Serve`, `WebAdapter_Listen`, `WebAdapter_Run`)의 `.c` 정의 위치에 Doxygen 블록을 두지 않고 `.h` 헤더에만 둔 것"을 결함으로 지적한 항목이다.

**판단: 수용 타당 (조치 완료로 인정).**
- 근거 1: Iter-1 public 함수 (예: `Control_Step`, `Control_Init`)도 `.c` 정의부에 Doxygen 블록이 없고 `.h`에 문서가 있다. 같은 프로젝트 내 관례 일치.
- 근거 2: 같은 함수에 `.c`와 `.h` 양쪽 Doxygen을 두면 Doxygen 경고 처리(WARN_AS_ERROR) 설정에서 중복 정의 경고가 발생할 수 있다. 문서 생성 게이트를 깨지 않는 선택이다.
- 근거 3: 헤더의 WebAdapter_* 블록에 `@trace`, HITL 정보, 반환값 의미가 모두 있음 (web_adapter.h:95 부근 확인).
- 조건: 이 판단은 Iter-1 관례를 따른 것이므로, 관례 자체를 변경하는 경우 N-02는 재개된다.

### 2.3 설계 부적합 항목

**D-01 [경미] ARCH-iter2.md:15 — ARCH-D-06 서술이 현재 구현과 불일치**
- 현재 문장: "소켓 루프(`web_main.c`)는 얇은 어댑터다."
- 실제 구현: 소켓 루프는 `src/web/web_adapter.c`의 `WebAdapter_Run` / `WebAdapter_Serve`에 있고, `web_main.c`는 `main`에서 이를 호출만 한다.
- 근거: web_adapter.c:173 (`WebAdapter_Run`), web_main.c:50 (`WebAdapter_Run(server, &web, INT_MAX)` 호출).
- 수정 제안: "소켓 루프(`web_adapter.c`)는 얇은 어댑터다. 진입점(`web_main.c`)은 이를 호출만 한다." 로 수정하고 ARCH-D-06 근거도 `web_adapter`로 맞춘다.

**D-02 [경미] ARCH-iter2.md:57 — 시퀀스 다이어그램 참여자명 불일치**
- 현재: `participant H as 어댑터(web_main)`
- 모듈명 기준으로는 `web_adapter`가 소켓 어댑터이므로 참여자명이 ARCH-C-016 모듈 정의(line 43, `소켓 어댑터 web_adapter`)와 다르다.
- 수정 제안: `participant H as 소켓 어댑터(web_adapter)` 로 수정.
- 참고: ARCH-iter2.md:28 (`H[web_adapter 소켓 어댑터]`)과 line 43은 이미 올바르게 반영되어 있음을 확인했다.

**D-03 [경미] DDS-iter2.md:160 — §6 "단일 책임" 행이 web_adapter를 포함하지 않음**
- 현재: "모듈: 요청 처리(web_handler), 화면(web_page), 입력 해석(web_request_parser) 분리."
- 소켓 어댑터(web_adapter)와 진입점(web_main) 분리가 설계 체크리스트 서술에 없다.
- 수정 제안: "소켓 입출력(web_adapter), 진입점(web_main) 분리" 항목을 추가.

### 2.4 설계 판정: **반려**
- 부적합 3건 (모두 경미). 조치 전에는 설계 승인 불가.
- 설계 승인 시까지 구현 시험 결과의 추적 근거(ARCH-D-06, DD-M-020)가 불완전하다.

---

## 3. 구현·시험 3차 리뷰

### 3.1 체크리스트 판정

| 항목 | 판정 | 근거 |
|------|------|------|
| SRC-01~17 | Pass | 아래 3.2 |
| UTS-01~13 | Pass | 아래 3.3 (단위 테스트 10 TEST, 절차 10건, RAII 가드) |
| ITS-01~12 | Pass | test/integration/web_integration_test.cpp 5 TEST_F, IT-024/IT-025 실제 TCP; INTEGRATION-PLAN.md 인터페이스 커버리지 표 ARCH-I-001~010 |
| SWE.6 (시스템) | Pass | test/system/web_system_test.cpp 7건, RecordProperty로 요구사항 ID 기록 |
| COM-01~04 | Pass | 파일·함수 HITL 블록 확인 (web_adapter.c 파일 블록 및 정적 함수 블록 2건) |

### 3.2 소스 코드 (SRC)

- 반환 상태: 모든 공개 함수가 `Status_t` 또는 `int`(served count) 반환. `void` 함수 없음 (`WebAdapter_Run`은 처리 건수를 반환하여 상태와 함께 의미를 보존).
- 단일 `return`: 이번 리뷰에서 web_adapter.c의 return 위치를 전수 grep하지 않았다. 이 항목은 SRC-07 근거 미확인이며 아래 4장에 명시한다.
- 전역 가변 데이터: 없음 (N-03 확인).
- 반환값 무시 사유 주석 (SRC-11): 3.1 I-01 표 참조. 시험 시점 grep 기준으로 확인한 전체 호출 목록이며, Iter-1 모듈은 주석 존재 여부만 확인했다 (호출 의미 재검증은 범위 밖).
- 63자 경로 절단 → 404 (WebAdapter_Serve): 설계(DD-F-072)와 일치.
- 루프백 바인딩 (DD-F-073, INADDR_LOOPBACK): 일치.
- 파일 자원: `close(fd)`(web_adapter.c:190), `close(server)`(web_adapter.c:166) 호출 존재 확인. 누수 경로 전수 검증은 이번 리뷰 범위 밖.

### 3.3 시험

- 단위 테스트: web_adapter_unit_test.cpp 10 TEST, `절차:` 10건 (grep 확인). `NofileGuard` RAII 가드는 이전 리뷰 기록 기준이며 이번 리뷰에서 재확인하지 않았다.
- 통합 테스트: web_integration_test.cpp 5 TEST_F (IT-024, IT-025 실제 TCP 연결), control_integration_test.cpp 8.
- 시스템 테스트: web_system_test.cpp 7, control_system_test.cpp 15, requirements_in_scope.txt의 SRS-C-004 항목에 시연 검증 패턴 존재.
- 커버리지 예외: docs/test/COVERAGE-EXCEPTIONS.md (DOC-CVX-001), 예외 대상은 web_main.c의 `main` 한정. 사용자 승인 기록 존재 (commit 4af1860 메시지 기준).
- 정적 분석 로컬 보고서 reports/cppcheck.xml: 오류 0건 (suppress 없음 확인).
- Lizard 로컬 보고서 reports/lizard.xml: 함수 88개, 최대 CCN 8, 최대 NCSS 29 (기준 CCN ≤10, 80라인 이하 충족).
- 참고: 테스트 실행 결과와 커버리지 수치는 본 리뷰에서 재실행하지 않았다. 로컬 보고서 기준이며, 최종 게이트는 CI에서 재확인해야 한다.

### 3.4 구현 부적합 항목

부적합 0건.

### 3.5 구현·시험 판정: **승인**

---

## 4. 종합

| 부분 | 판정 | 부적합 |
|------|------|--------|
| 설계 재리뷰 (N-01) | **반려** | 경미 3건 (D-01, D-02, D-03) |
| 구현·시험 3차 | **승인** | 없음 |

- 설계 부분이 반려이므로 Iter-2 설계 단계는 승인되지 않았다. 다음 단계 진행은 **불가**.
- D-01~D-03은 문서 서술 정정이므로 소스 수정 없이 ARCH-iter2.md, DDS-iter2.md만 고치면 된다. 수정 후 설계 재리뷰가 필요하다.
- 구현 부분의 승인은 현재 커밋 ee4e8ad 기준이며, 설계 문서 수정 시 구현 결과에 영향을 주지 않는다는 점을 확인했다 (소스 변경 없음).

## 5. 추적 확인

- TRACE.md (v1.7) SRS-C-004 행에 DD-F-071~076 및 web_adapter.c/.h 포함 확인.
- TRACE.md 입력 문서 버전: ARCH-iter2 v0.6, DDS-iter2 v0.6 기록. D-01~D-03 수정 후 TRACE 재생성이 필요할 수 있다 (설계 문서 내 ID 변경이 없으면 불필요).

## 4-1. 확인 한계 (SRC-07, UTS 가드 항목)

- SRC-07 (단일 return) 전수 확인은 이번 리뷰에서 수행하지 않았다. 이전 리뷰 기록(2차)에서 확인된 항목으로만 간주하며, 이번 판정의 독립 근거가 아니다.
- NofileGuard RAII 구현 세부는 이전 리뷰 기록 기준이다.
- 위 두 항목은 확인 불가로 남으며, 엄격 기준에 따라 승인 근거로는 사용하지 않는다. 구현 부분 승인은 "이전 리뷰에서 확인되었고 이번 변경 범위(주석·테스트 절차·RAII 가드·문서)에 회귀가 없음"을 근거로 한다.
