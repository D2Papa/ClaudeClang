# 리뷰 보고서: (1) SWE.1 재리뷰 v1.4, (2) Iter-2 설계 7차 재리뷰

- 대상 (커밋 c39662b, `feature/iter-0-requirements`):
  - (1) `docs/requirements/SRS.md` v1.4 (변경: SRS-F-002, 003, 018, 022, 030, 6.3 순위 8), `docs/requirements/ASSUMPTIONS.md` v1.3, `docs/traceability/TRACE.md` v1.3 (SAF v1.0은 변경 없음)
  - (2) `docs/architecture/ARCH.md` v0.5 (V-01: ARCH-I-006 사후 조건), `docs/detailed-design/DDS.md` v0.5 (V-02: DD-F-029), `docs/architecture/ARCH-iter2.md` v0.5, `docs/detailed-design/DDS-iter2.md` v0.5
- 기준선 대조: SRS v1.3(44ab31a) 열 단위 비교, DDS.md·ARCH.md 6차 판 대비 diff
- 이전 리뷰: REVIEW-ITER2-DESIGN-6.md (U-01~U-03, V-01, V-02)
- 일자: 2026-11-27
- 판정: (1) SWE.1 재리뷰 **반려**, (2) 설계 7차 **반려**

---

## Part 1. SWE.1 재리뷰 (SRS v1.4, ASSUMPTIONS v1.3, TRACE v1.3)

### 1.1 변경 범위 확인

- SRS v1.3 → v1.4 내용 변경 (열 단위 비교): SRS-F-022(NORMAL 한정 예외 및 시험), SRS-F-003(시험 보강과 상태별 reason 시험). SRS-F-002, 018, 030은 v1.3과 내용 같음 (변경 이력의 "F-002, 003, 018, 022, 030, 6.3" 중 실제 변경은 F-003, F-022, 6.3 순위 8). 헤더 버전 1.4와 변경 이력 1.4 행 확인.
- 6.3 순위 8: "활성 규칙이 없으면 state NORMAL은 NONE(정보성 이유가 있으면 그 값), DEGRADED는 DEGRADED_STALE(SSR-009), FAULT는 SENSOR_FAULT(SSR-012), OFF는 순위 3" 명시 (확인).
- 상태 값: 변경 행 (F-003, F-022 등)은 "초안 (재리뷰 대기)", 나머지 "승인(v1.0)" (확인).
- ASSUMPTIONS 헤더 1.3, 상태 체계 설명 확인. A-19 행은 v1.3과 같으며 NORMAL 한정이라는 U-01 결정은 반영되지 않음 (W-02).
- TRACE 1.3: 입력 문서 버전 "SRS v1.4, SAFETY-REQ v1.0, ASSUMPTIONS v1.3, ARCH v0.5, ARCH-iter2 v0.5, DDS v0.5, DDS-iter2 v0.5"를 헤더에 기록 (U-03 확인).

### 1.2 이전 결함 조치 상태 (U-01~U-03)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| U-01 | 중대 | F-003 상태 문장과 F-022 속도 예외의 충돌 | 조치 완료 (새 결함 W-01: 악성 명령 경우 미해결, W-02: 결정 기록 누락) | F-022 예외가 state NORMAL로 한정됨. F-003 시험에 DEGRADED·FAULT 속도 3.0 km/h 케이스(AUTO_LOCK) 추가. 구현 DD-F-029와 일치 |
| U-02 | 중대 | 6.3 순위 8 reason과 SSR-009·012 불일치 | 조치 완료 | 6.3 순위 8에 state별 reason 명시. DD-F-024·ARCH-I-006과 일치 (확인) |
| U-03 | 경미 | TRACE 버전과 입력 버전 | 조치 완료 | TRACE 1.3, 입력 버전 헤더 기록 확인 |

조치 완료 3, 부분 0, 미조치 0.

### 1.3 체크리스트 결과 (REQ-01~12, COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| REQ-01 | 영구·고유 ID | Pass | ID 35건 유지, 신규·재사용 없음 |
| REQ-02 | 기능/비기능 분류 | Pass | 변경 행 모두 기능 |
| REQ-03 | 안전 요구 분리 | Pass | SRS 본문에 SSR 본문 없음. SSR은 ID로 참조 |
| REQ-04 | ISO 25010 분류 | Pass | 변경 없음 |
| REQ-05 | 한 요구 한 가지 | Pass | F-003은 거절 조건, 기록 값, 참조 규칙을 한 규칙으로 다룸 |
| REQ-06 | 모호 표현 없이 검증 가능 | Pass | 상태별 reason 값이 명시됨 (F-003, 6.3 순위 8). 악성 명령 경우는 REQ-08(W-01)에서 기록 |
| REQ-07 | 검증 방법과 합격 기준 | Pass | F-003 시험에 NORMAL·OFF·DEGRADED·FAULT별 케이스, F-022에 NORMAL 한정 예외 시험 |
| REQ-08 | 충돌·중복 없음 | **Fail** (W-01) | SSR-007("거절된 IF-004 명령 … INPUT_INVALID 기록")과 F-031(OFF: IGNITION_OFF), F-003(DEGRADED·FAULT: 상태 reason), 6.3 순위 8(NORMAL은 정보성 이유 가능) 사이에 거절 명령의 reason이 충돌 |
| REQ-09 | "무엇"을 기술 | Pass | 구현 방법 지정 없음 |
| REQ-10 | 상향 추적과 내용 일치 | Pass | F-022 예외는 A-19 승인 기록으로 OEM-IF-006 편차 추적 |
| REQ-11 | 하향 추적 단절 없음 | Pass | TRACE 1.3에 DD-F-024·029 배정 반영 |
| REQ-12 | 근거·우선순위·상태 | Pass | 모든 표에 근거, 우선순위, 상태 존재 |
| COM-01 | 산출물 ID·버전·상태 | Pass | SRS 1.4, ASSUMPTIONS 1.3, TRACE 1.3, SAF 1.0 (변경 없음) 헤더 일관 |
| COM-02 | 형상 관리 | Pass | 커밋 c39662b, docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | Pass | U-01~U-03 조치 확인 |
| COM-04 | 상·하위 ID·용어 일관성 | **Fail** (W-02) | U-01(F-022 NORMAL 한정)과 U-02(순위 8 state reason)는 작성자가 정한 결정인데 ASSUMPTIONS 가정 행과 TRACE §3 편차 기록에 없다. A-19 행은 NORMAL 한정을 반영하지 않는다 |

요약 (REQ·COM 16항목): Pass 14 / Fail 2 / N/A 0 (Fail: REQ-08, COM-04)

### 1.4 결함 목록 (SWE.1)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| W-01 | 중대 | SRS.md F-003 (상태별 reason 문장), 6.3 순위 8 (NORMAL 정보성 이유), SAFETY-REQ.md SSR-007 | SSR-007은 "거절된 IF-004 명령은 무시하고 reason_code INPUT_INVALID를 기록한다"고 쓴다. 같은 주기에 명령이 OFF, DEGRADED, FAULT 중 하나여서 적용되지 않으면 F-031(OFF: IGNITION_OFF), F-003(DEGRADED·FAULT: DEGRADED_STALE 또는 SENSOR_FAULT)이 다른 reason을 요구한다. 명령이 NORMAL이면 6.3 순위 8이 정보성 이유 INPUT_INVALID를 허용하므로 NORMAL에서는 맞지만, 상태 억제 시에는 우선순위가 정해지지 않았다. 설계(DD-F-024)는 상태 reason을 쓰고 INPUT_INVALID를 버린다 (V-03 참조). | 상태로 적용되지 않은 명령이 거절(형식 오류)되었을 때 reason을 정한다(예: INPUT_INVALID 우선 또는 상태 reason 우선). 결정을 F-003, F-031, 6.3 순위 8에 같은 문장으로 쓰고 시험을 추가한다. 사용자 판단이 필요하다. |
| W-02 | 중대 | ASSUMPTIONS.md A-19 행, 편차 및 승인 기록 표, TRACE.md §3 편차 기록 | U-01(F-022 예외를 NORMAL로 한정, OFF·DEGRADED·FAULT는 결정 규칙 reason)과 U-02(6.3 순위 8의 state별 reason)는 요구 의미를 바꾸는 작성자 결정이다. ASSUMPTIONS에 가정 ID가 없고 편차 기록이 없다. A-19 행은 "CMD_REJECTED_SPEED로 덮어쓰고 SRS-F-022에 예외로 반영"만 쓰며 NORMAL 한정이 빠졌다. 요구 추적이 결정 근거를 잃었다. | A-23(U-01 F-022 NORMAL 한정, 작성자 결정, 사용자 확인 대기)과 A-24(U-02 순위 8 state reason, 작성자 결정, 사용자 확인 대기)를 추가하고 편차 표와 TRACE §3에 반영한다. A-19 행의 적용 범위를 NORMAL로 고친다. |

### 1.5 SWE.1 판정

**반려.** 중대 2건(W-01, W-02)이 남아 있다. W-02는 문서 정정으로 해결된다. W-01은 상태 억제 중 형식 오류 명령의 reason 우선순위 결정이라 사용자 판단이 필요하다.

---

## Part 2. Iter-2 설계 7차 재리뷰

### 2.1 이전 결함 조치 상태

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| V-01 | 경미 | ARCH.md ARCH-I-006 사후 조건 보강 | 조치 완료 | ARCH-I-006에 상태 미적용 명령의 reason 규칙(state 이유, OFF는 IGNITION_OFF), 활성 규칙 없는 주기의 reason(FAULT SENSOR_FAULT, DEGRADED DEGRADED_STALE, 그 외 정보성 이유 또는 NONE) 명시. ARCH.md v0.5 |
| V-02 | 중대 | DD-F-029·DD-F-024의 SRS 불일치 | 조치 완료 (새 결함 V-03) | DD-F-029를 state NORMAL 한정으로 수정, FAULT·DEGRADED·OFF에서는 0이고 reason은 결정 규칙 값이라고 명시. DD-F-024는 6.3 순위 8과 일치 (DDS.md v0.5) |
| H-01 | 중대 | 상태 억제 명령의 reason 규칙 미기재 | 조치 완료 | DDS.md DD-F-024와 ARCH.md ARCH-I-006 모두 기재 |

3건 모두 조치 완료. 부분·미조치 없음.

### 2.2 교차 점검

- **DD-F-029 ↔ SRS-F-022(NORMAL 한정) ↔ 구현**: DD-F-029는 state NORMAL, 운전자 순위 미제외, info_reason CMD_REJECTED_SPEED일 때만 1. SRS-F-022 예외와 일치 (확인).
- **DD-F-024 ↔ 6.3 순위 8**: NORMAL은 info reason 또는 NONE, DEGRADED는 DEGRADED_STALE, FAULT는 SENSOR_FAULT, OFF는 순위 3 규칙. 일치 (확인).
- **DD-F-024 ↔ SSR-007**: 상태(DEGRADED, FAULT, OFF)에서 형식 오류로 거절된 IF-004 명령의 INPUT_INVALID가 기록되지 않는다. DD-F-024는 state reason을 쓰고 info_reason INPUT_INVALID는 NORMAL에서만 보인다. SSR-007의 shall 위반 가능성 (V-03).
- **ARCH-I-006 ↔ DDS.md**: 사후 조건과 DD-F-024·029의 값이 일치한다 (확인).
- **ARCH-iter2, DDS-iter2 ↔ 변경분**: 이번 차수에서 변경 없음 (c39662b 대비 확인).
- **TRACE 1.3 ↔ DDS.md 0.5**: DD-F-024, DD-F-029 배정 반영 (확인).

### 2.3 체크리스트 결과 (설계)

#### ARC-01~13

| ID | 판정 | 근거 |
|----|------|------|
| ARC-01~12 | Pass | 변경 없음 또는 V-01 반영 |
| ARC-03 | Pass | ARCH-I-006 사후 조건이 상태별 reason을 정의 (V-01 확인) |
| ARC-13 | Pass | 근거 기록 (DD-F-024·029 본문에 SRS-F-003, SRS-F-022 참조) |

#### DDS-01~12

| ID | 판정 | 근거 |
|----|------|------|
| DDS-01~10 | Pass | 변경 없음 |
| DDS-11 | **Fail** (V-03) | 상태 억제 중 형식 오류 명령의 INPUT_INVALID 기록 누락 (SSR-007 위반 가능) |
| DDS-12 | Pass | DD-F-024 → SRS-F-003, SSR-009, SSR-012. DD-F-029 → SRS-F-003, SRS-F-022. 추적 성립 |

#### COM-01~04

| ID | 판정 | 근거 |
|----|------|------|
| COM-01 | Pass | ARCH.md 0.5, DDS.md 0.5, ARCH-iter2 0.5, DDS-iter2 0.5 |
| COM-02 | Pass | 커밋 c39662b |
| COM-03 | Pass | H-01, V-01, V-02 조치 확인 |
| COM-04 | **Fail** (W-02, 공유) | 설계가 따르는 결정(U-01, U-02)의 가정 기록이 없다 |

### 2.4 결함 목록 (설계)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| V-03 | 중대 | DDS.md DD-F-024 (상태 억제 명령의 이유 규칙), DD-F-029; ARCH.md ARCH-I-006 (상태 reason 사후 조건) | SSR-007은 거절된 IF-004 명령이 INPUT_INVALID를 기록해야 한다고 쓴다(shall). 설계는 DEGRADED, FAULT, OFF에서 형식 오류 명령에도 상태 reason을 쓰고 INPUT_INVALID를 기록하지 않는다. 안전 요구(SSR-007) 위반이며 W-01 결정 전까지 설계를 확정할 수 없다. | W-01 결정 후 DD-F-024를 그 우선순위에 맞춘다(INPUT_INVALID 우선이면 상태 억제 중 형식 오류 명령에 INPUT_INVALID를 쓰는 분기를 추가). ARCH-I-006도 같은 규칙을 반영한다. 결정 전에는 이 항목의 시험 케이스를 확정하지 않는다. |

### 2.5 설계 판정

**반려.** V-03(중대)은 SRS W-01 결정에 종속된다. W-02(COM-04)는 ASSUMPTIONS와 TRACE의 문서 정정으로 해결된다.

---

## 3. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| (1) SWE.1 재리뷰 (SRS v1.4, ASSUMPTIONS v1.3, TRACE v1.3) | **반려** | 중대 2 (W-01, W-02) |
| (2) 설계 7차 (ARCH.md v0.5, DDS.md v0.5, ARCH-iter2 v0.5, DDS-iter2 v0.5) | **반려** | 중대 1 (V-03), W-02 공유 |

- 이전 결함 조치: U-01~U-03, V-01, V-02, H-01 모두 조치 완료.
- 사용자 판단 필요: W-01 (상태 억제 중 형식 오류 명령의 reason 우선순위). 결정이 V-03과 SSR-007 해석을 함께 결정한다.
- 문서 정정으로 해결: W-02 (A-23, A-24 추가 및 편차 표·TRACE §3 반영).
- 재리뷰 시 W-01, W-02, V-03의 조치 상태를 표로 확인하고, SRS F-003·F-031·6.3 순위 8, SAFETY-REQ SSR-007, ASSUMPTIONS A-19·A-23·A-24와 편차 표, TRACE §3, DDS.md DD-F-024·029, ARCH.md ARCH-I-006을 함께 교차 점검한다.
