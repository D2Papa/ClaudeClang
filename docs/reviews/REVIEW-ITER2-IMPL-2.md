# 리뷰 보고서: Iter-2 구현·시험 2차 재리뷰

- 대상 (커밋 4af1860, 선행 ba45472, `feature/iter-0-requirements`):
  - 소스: `src/web/web_adapter.c`, `src/web/web_adapter.h` (신규), `src/web/web_main.c` (main 한 함수), 반환값 무시 사유 주석 (`src/web/*`, `src/present/*`, `src/history/*`, `src/control/*`)
  - 단위 테스트: `test/unit/web_adapter_unit_test.cpp` (신규), 절차 주석 (`test/unit/*`, `test/integration/*`, `test/system/*`)
  - 통합 테스트: `test/integration/web_integration_test.cpp` (IT-025 실제 TCP)
  - 시험 문서: `docs/test/INTEGRATION-PLAN.md` (신규), `docs/test/DEMO-web.md`, `scripts/demo/web_demo.sh` (신규), `test/system/requirements_in_scope.txt`, `CMakeLists.txt`
- 설계 기준: `docs/detailed-design/DDS-iter2.md` v0.5, `docs/detailed-design/DDS.md` v0.8, `docs/architecture/ARCH-iter2.md` v0.5
- 이전 리뷰: REVIEW-ITER2-IMPL-1.md (I-01~I-08)
- 로컬 증거 (커밋 이후 생성, 15:24): `reports/coverage/unit/summary.json`에 `web_adapter.c` MC/DC 100%, 라인 100%; 테스트 수 TDD 15, 단위 109, 통합 14, 시스템 20 (소스 grep과 일치).
- 일자: 2026-11-27
- 판정: **반려**

---

## 1. 이전 지적 (I-01~I-08) 조치 상태

| 번호 | 심각도 | 1차 지적 | 조치 상태 | 근거 |
|------|--------|----------|-----------|------|
| I-01 | 중대 | 반환값 무시에 사유 주석 없음 (SRC-11) | **부분 조치** (잔여 4곳) | 40여 곳에 `반환값을 쓰지 않는 호출: …` 주석 추가 확인. 잔여: `approach_risk_rule.c` 69행 (`ApproachRisk_EvaluateDoor` 루프, 주석 없음), `web_adapter.c` 165행 (`close(server)`), `web_adapter.c` 188행 (`close(fd)`), `web_main.c` 49행 (`WebAdapter_Run` 결과 무시) |
| I-02 | 중대 | web_main.c 전역 가변 static | 조치 완료 | `web_main.c`에 `static` 없음. `Web_t web`은 main 자동 변수. `web_adapter.c`에도 정적 가변 변수 없음 (grep 확인) |
| I-03 | 중대 | 테스트 주석에 "절차" 항목 없음 (UTS-04, ITS-05) | **부분 조치** | 지적된 파일들의 134곳에 "절차:"가 추가됨. 신규 `test/unit/web_adapter_unit_test.cpp` 10건에는 "절차:"가 0건 (이 파일은 조치 보고의 134곳에 포함되지 않음) |
| I-04 | 중대 | web_main.c 커버리지 제외 근거 부재 (UTS-10) | 조치 완료 (잔여 N-04 경미) | 소켓 처리 로직을 `web_adapter.c`(제품 라이브러리)로 이전, `web_adapter_unit_test.cpp`로 MC/DC 100% (로컬 리포트 확인). `CMakeLists.txt` 25~28행에 main 한 함수만 제외한다는 사용자 승인 예외 주석 |
| I-05 | 경미 | 시연 기록 재현 정보 부족 | 조치 완료 | `scripts/demo/web_demo.sh` 7개 시나리오(curl 기반, 판정 포함). `DEMO-web.md`에 재현 방법, 기준 커밋, 결과 기재. 시스템 범위 파일의 시연 검사가 7번 시나리오와 "시연 결과: 성공"까지 확인하도록 강화 (`requirements_in_scope.txt` 마지막 줄) |
| I-06 | 경미 | 통합 테스트 계획 부재 (ITS-01) | 조치 완료 | `docs/test/INTEGRATION-PLAN.md`: 목적, 전략, 합격 기준, 실행 절차, 파일 목록 |
| I-07 | 중대 | 인터페이스 커버리지 표 부재 (ITS-02) | 조치 완료 | `INTEGRATION-PLAN.md` §2: ARCH-I-001~010 각각의 정상·오류 경로와 통합 테스트 ID 표 |
| I-08 | 경미 | 63자 초과 경로 처리 규칙 부재 | 조치 완료 | `WebAdapter_Serve`에 "경로가 63자를 넘으면 … 404가 된다" 주석, 시연 7번으로 확인 |

집계: 조치 완료 6 (I-02, I-04, I-05, I-06, I-07, I-08), 부분 조치 2 (I-01, I-03), 미조치 0. 부분 조치 2건은 중대이므로 반려다.

---

## 2. 신규 파일과 구조 점검

| 항목 | 판정 | 근거 |
|------|------|------|
| 단일 반환 (web_adapter.c 6개 함수, web_main.c main) | Pass | 함수마다 return 1회. `WebAdapter_Run`, `WebAdapter_Serve`, `WebAdapter_Listen`, `WebAdapter_ReadRequest`, `Adapter_ReadBody`, `Adapter_Fill` 확인 |
| 함수 크기 | Pass | 가장 긴 함수 약 35행 (`WebAdapter_Serve`), Lizard 함수 기준 이내 (1차 리뷰의 로컬 Lizard 결과 기준) |
| 전역 가변 데이터 | Pass | `web_adapter.c`와 `web_main.c`에 `static` 가변 변수 없음 |
| 소켓 호출 오류 처리 | Pass | `recv` 음수, `accept` 실패, `listen` 실패 시 -1 또는 종료 처리. 상대 연결 끊김 시 다음 연결 계속 처리 주석 |
| 버퍼 경계 | Pass | `recv` 크기 `cap - 1 - total`, 본문 읽기 상한 `cap - 1`, 응답 head 256바이트 (snprintf) |
| HITL 블록 | **Fail** (N-02) | `web_adapter.c` 헤더 블록 1 + 함수 블록 2 (Adapter_Fill, Adapter_ReadBody). `WebAdapter_ReadRequest`, `WebAdapter_Serve`, `WebAdapter_Listen`, `WebAdapter_Run` 정의에는 Doxygen과 HITL 블록이 없다. `web_adapter.h`에는 있다 |
| Doxygen (SRC-08) | 경미 (N-02) | 동일 |
| 설계 일치 (SRC-04) | **Fail** (N-01) | 설계 DD-M-017은 `web_main.c`가 소켓 루프를 맡는다고 정의한다. 실제로는 `web_adapter.c`/`web_adapter.h`가 소켓 처리를 맡고 `web_main.c`는 main만 있다. DDS-iter2와 DDS.md에 신규 모듈과 함수(ReadRequest, Serve, Listen, Run)가 없다 |

---

## 3. 체크리스트 결과

### 3.1 소스 코드 (SRC-01~17)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| SRC-01 | 반환값 가짐 | Pass | `void` 없음 |
| SRC-02 | return 1회 | Pass | 2장 |
| SRC-03 | 상태 코드 반환 | Pass | 공개 API 정수 또는 Status_t |
| SRC-04 | 구현이 상세 설계와 일치 | **Fail** (N-01) | 신규 `web_adapter.c` 모듈과 4개 함수가 DDS-iter2 DD-M-017과 다르다. 설계 미갱신 |
| SRC-05 | 공개 헤더로만 의존 | Pass | `web_adapter.h`를 통해서만 노출 |
| SRC-06 | 입력 검증과 오류 처리 | Pass | 2장 |
| SRC-07 | 클린 코드 | Pass | 확인 범위 내 |
| SRC-08 | Doxygen | Pass | 헤더 선언 블록과 .c 일부 블록으로 확인 (N-02 경미 참조) |
| SRC-09 | 경고 없이 빌드 | Pass | 게이트 결과 보고 (로컬 로그 미저장) |
| SRC-10 | 데드 코드 | Pass | 없음 |
| SRC-11 | 반환값 확인 또는 사유 주석 | **Fail** (I-01 부분) | 잔여 4곳 (1장) |
| SRC-12 | 소스 → DD 추적 | Pass | `@trace` 존재. 신규 함수는 DD 없음 (N-01) |
| SRC-13 | HITL 정보 | Pass | 파일 블록과 헤더 블록 존재. 구현 함수 정의 블록 부재는 N-02 경미 |
| SRC-14 | 사용자 리뷰 표시 무결성 | Pass | 전부 "미완료" |
| SRC-15 | Cppcheck 0건, 예외 없음 | Pass | `reports/cppcheck.xml` 오류 0, suppress 없음 |
| SRC-16 | Lizard 기준 | Pass | 로컬 리포트 함수 최대 CCN 8 (1차 확인). 신규 함수 포함 재확인 (커버리지 리포트 생성 시점) |
| SRC-17 | TDD 분리 | Pass | `test/tdd/iter2_tdd_test.cpp` 별도 |

### 3.2 단위 테스트 (UTS-01~13)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| UTS-01 | MC/DC 100% | Pass | `web_adapter.c` MC/DC 100% (로컬 리포트) |
| UTS-02 | LLVM 증거와 분리 | Pass | `reports/coverage/unit`, `tdd` 분리 |
| UTS-03 | 조건 독립성 | Pass | MC/DC 100% |
| UTS-04 | Doxygen에 목적·입력·절차·예상 | **Fail** (I-03 부분) | `web_adapter_unit_test.cpp` 10건에 "절차:" 없음 |
| UTS-05 | 예상값 도출 | Pass | 리터럴 기대값 |
| UTS-06 | 의미 있는 assertion | Pass | 확인 범위 내 |
| UTS-07 | 기법 기록 | Pass | "기법:" 기록 |
| UTS-08 | Mock 남용 없음 | Pass | socketpair와 실제 소켓 사용 |
| UTS-09 | 오류 경로 | Pass | 널, 실패 소켓 경로 |
| UTS-10 | 커버리지 예외 없음 | **Fail** (N-04, 경미) | 제외는 `web_main.c` main 한 함수뿐이며 CMake 주석에만 기록되어 있다. 예외 목록 문서가 없다 |
| UTS-11 | 추적 | Pass | `@trace` 존재 |
| UTS-12 | 테스트 독립성 | **Fail** (N-03, 경미) | `web_adapter_unit_test.cpp`가 `RLIMIT_NOFILE`을 0으로 낮췄다가 복원한다. 중간 assert 실패 시 복원되지 않아 같은 프로세스의 다른 테스트에 영향을 줄 수 있다 |
| UTS-13 | 결과 기록 | Pass | `reports/test/unit.xml` |

### 3.3 통합 테스트 (ITS-01~12)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| ITS-01 | 통합 전략·계획 | Pass | `INTEGRATION-PLAN.md` §1 |
| ITS-02 | 함수 커버리지 100%, 인터페이스 커버리지 표 | Pass | 통합 함수 100% (로컬 리포트), 표 §2 |
| ITS-03 | 인터페이스별 정상·경계·무효·오류 | Pass | §2 표 |
| ITS-04 | 인터페이스 중심 | Pass | IT-025 실제 TCP 경유 |
| ITS-05 | Doxygen에 절차 | Pass | `web_integration_test.cpp` 5건 "절차:" 확인 (IT-024, IT-025 포함). 기존 Iter-1 통합 8건도 확인 |
| ITS-06 | 예상값 도출 | Pass | 인터페이스 정의 |
| ITS-07 | 의미 없는 케이스 없음 | Pass | 확인 범위 내 |
| ITS-08 | Stub 사용 | Pass | 실제 컴포넌트 연결 |
| ITS-09 | 기법 기록 | Pass | 기록 |
| ITS-10 | 호출 순서 시나리오 | Pass | StepJson 시나리오 |
| ITS-11 | 추적 | Pass | `@trace ARCH-I-*` |
| ITS-12 | 결과 기록 | Pass | `reports/test/integration.xml` |

### 3.4 시스템 (SWE.6 관점)

| 항목 | 판정 | 근거 |
|------|------|------|
| 합격 기준 정합 | Pass | 1차 리뷰 확인 유지 |
| 시연 증거 | Pass | `web_demo.sh` 7개 시나리오, `DEMO-web.md` 재현 기록, 시스템 검사 정규식 강화 |
| 추적성 | Pass | `RecordProperty("requirements")`와 범위 파일 일치 |

---

## 4. 결함 목록

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| I-01 (잔여) | 중대 | `src/control/approach_risk_rule.c` 66~69행 (루프 내 `ApproachRisk_EvaluateDoor`), `src/web/web_adapter.c` 165행 (`close(server)`), 188행 (`close(fd)`), `src/web/web_main.c` 49행 (`WebAdapter_Run`) | 반환값을 쓰지 않으면서 사유 주석이 없다 (SRC-11). | 각 호출 앞에 `/* 반환값을 쓰지 않는 호출: <사유> */`을 추가한다. `WebAdapter_Run`은 반환하는 처리 건수를 쓰지 않는 이유를 적는다. |
| I-03 (잔여) | 중대 | `test/unit/web_adapter_unit_test.cpp` (10건) | 신규 단위 테스트의 Doxygen에 "절차:" 항목이 없다 (UTS-04). 134곳 수치에 이 파일이 빠져 있다. | 10건 모두에 "절차: 1) … 2) …"를 추가한다. |
| N-01 | 중대 | `src/web/web_adapter.c`, `src/web/web_adapter.h`, `docs/detailed-design/DDS-iter2.md` §2·§3·§5, `docs/detailed-design/DDS.md`, `docs/architecture/ARCH-iter2.md` | 소켓 처리 로직이 설계 DD-M-017(`web_main.c`)에서 신규 모듈 `web_adapter.c`(WebAdapter_ReadRequest, Serve, Listen, Run)로 옮겨졌다. 설계 문서가 갱신되지 않아 SRC-04와 SRC-12를 충족하지 못한다. 사용자 승인된 예외는 main 한 함수에 대한 커버리지 제외뿐이고, 모듈 구조 변경의 설계 승인은 아니다. | DDS-iter2에 DD-M-020 `web/web_adapter.c/.h`를 추가하고 DD-M-017은 main만 남긴다. 4개 함수에 DD-F 번호와 시그니처를 부여하고 §5 배정과 TRACE에 반영한다. ARCH-iter2 §3 C-016 모듈 설명도 맞춘다. 설계 변경은 설계 재리뷰 대상이다. |
| N-02 | 경미 | `src/web/web_adapter.c` (`WebAdapter_ReadRequest`, `WebAdapter_Serve`, `WebAdapter_Listen`, `WebAdapter_Run` 정의) | 구현 파일의 함수 정의에 Doxygen과 HITL 블록이 없다. 헤더 선언에만 있다. SRC-08, SRC-13은 함수 주석을 요구하므로 정의 위치의 주석 유무를 명확히 해야 한다. | 4개 정의에 Doxygen 블록(또는 헤더를 참조하는 한 줄 블록)과 HITL 블록을 추가한다. |
| N-03 | 경미 | `test/unit/web_adapter_unit_test.cpp` (`RLIMIT_NOFILE` 조작 테스트) | 프로세스 전역 자원 한도를 바꾸고 `ASSERT_*`로 중간에 빠져나가면 복원이 건너뛰어진다. 다른 테스트의 독립성(UTS-12)을 해칠 수 있다. | RAII 가드(소멸자에서 복원)로 감싼다. |
| N-04 | 경미 | `CMakeLists.txt` 25~28행, `docs/test/` | 커버리지 제외 예외(`web_main.c` main)가 CMake 주석에만 있고 예외 목록 문서가 없다 (UTS-10). | `docs/test/`에 커버리지 예외 목록(파일, 함수, 사유, 승인 근거, 대체 검증)을 만든다. 승인 근거는 질문 응답 기록을 인용한다. |

---

## 5. 판정 메모

- 1차 중대 지적 중 I-02(전역 static), I-04(커버리지 예외 근거를 구조 변경으로 해소), I-07(인터페이스 표)은 해결되었다. I-05, I-06, I-08도 해결되었다.
- I-01과 I-03은 부분 조치다. 잔여 호출 4곳과 신규 테스트 10건을 고치면 해결된다.
- N-01(설계 불일치)이 가장 큰 항목이다. 구조 변경을 설계에 반영하고 설계를 재리뷰받아야 한다. 이번 판정은 구현의 정합성 검토이며 설계 승인은 아니다.
- N-02, N-03, N-04는 경미다.
- 재리뷰 시 I-01, I-03, N-01~N-04의 조치 상태를 표로 확인하고, DDS-iter2 DD-M-017/020과 §5 배정, ARCH-iter2 §3, `web_adapter.c` 함수 주석, 단위 테스트 RLIMIT 가드를 함께 교차 점검한다.

---

## 6. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| Iter-2 구현·시험 2차 (소스, 단위, 통합, 시스템, 신규 파일) | **반려** | 중대 3 (I-01 잔여, I-03 잔여, N-01), 경미 3 (N-02, N-03, N-04) |

- 다음 단계 진행: **불가**. 중대 3건과 경미 3건을 조치하고 재리뷰가 필요하다. N-01은 설계 재승인이 함께 필요하다.
