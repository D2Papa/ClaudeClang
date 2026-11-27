# 리뷰 보고서: (1) SWE.1 재리뷰 (SAFETY-REQ v1.1, ASSUMPTIONS v1.6, SRS v1.6, TRACE v1.5), (2) Iter-2 설계 10차 재리뷰

- 대상 (커밋 b853568, `feature/iter-0-requirements`):
  - (1) `docs/safety/SAFETY-REQ.md` v1.1 (SSR-007 변경, 변경 이력), `docs/requirements/ASSUMPTIONS.md` v1.6 (A-25), `docs/requirements/SRS.md` v1.6 (변경 없음, SSR-007 정합 확인), `docs/traceability/TRACE.md` v1.5 (재생성)
  - (2) `docs/detailed-design/DDS.md` v0.8 (DD-F-029 추적 열 V-05, §5 배정 Y-01·Y-03), `docs/architecture/ARCH.md` v0.7, `docs/architecture/ARCH-iter2.md` v0.5, `docs/detailed-design/DDS-iter2.md` v0.5
- 기준선 대조: SAF v1.0(69da6db) 열 단위 비교, SRS v1.6(ee9b511)과 b853568 diff (SRS 변경 0줄 확인), DDS.md·TRACE 8·9차 판 대비 diff
- 이전 리뷰: REVIEW-ITER2-DESIGN-9.md (X-03, Y-01, Y-03, V-05)
- 일자: 2026-11-27
- 판정: (1) SWE.1 재리뷰 **반려**, (2) 설계 10차 **승인** (다만 입력 SAF v1.1·SRS v1.6의 SWE.1 승인 전까지 다음 단계 진행 불가)

---

## Part 1. SWE.1 재리뷰 (SAFETY-REQ v1.1, ASSUMPTIONS v1.6, SRS v1.6, TRACE v1.5)

### 1.1 변경 범위 확인

- SAF v1.0 → v1.1: SSR-007 내용만 변경 (열 단위 비교로 확인). "거절된 IF-004 명령은 … state가 NORMAL이면 reason_code INPUT_INVALID를 기록한다 (그 외 state에서는 SSR-009, SSR-012, SRS-F-031의 상태 이유가 우선한다 …)". SSR-007 행 상태 "초안 (재리뷰 대기)", 나머지 15 SSR "승인(v1.0)" (확인). 변경 이력 1.1 행 있음 (확인).
- SRS v1.6: b853568에서 변경 없음 (diff 0줄 확인). SRS-F-032 "state가 NORMAL일 때 (b) … INPUT_INVALID, OFF/DEGRADED/FAULT는 상태 이유" 와 SSR-007 v1.1이 일치 (확인).
- ASM v1.6: A-25 "SSR-007을 v1.1로 개정함" 반영 (확인). 편차 표와 TRACE §3에 A-25 반영 (확인).
- TRACE: 헤더 "입력 문서 버전: SRS v1.6, SAFETY-REQ v1.1, ASSUMPTIONS v1.6, ARCH v0.7, ARCH-iter2 v0.5, DDS v0.8, DDS-iter2 v0.5" (확인). SRS-F-031 행에 DD-F-024, DD-F-029 (Y-01 확인). SRS-F-022 행에 DD-F-021 없음 (Y-03 확인).

### 1.2 이전 결함 조치 상태 (X-03, Y-01, Y-03)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| X-03 | 중대 | SSR-007과 SRS-F-032 충돌, 안전 편차 미승인 | 조치 완료 (새 결함 Z-01) | 사용자 선택 "상태 이유 우선"에 맞춰 SSR-007을 v1.1로 개정. SSR-007 행은 재리뷰 대기 표시, 변경 이력 1.1 추가. SRS-F-032와 문구 일치 확인 |
| Y-01 | 경미 | SRS-F-031 배정 누락 | 조치 완료 | DDS.md §5와 TRACE에서 SRS-F-031에 DD-F-024, DD-F-029 포함 (확인) |
| Y-03 | 경미 | SRS-F-022의 DD-F-021 배정 | 조치 완료 | DDS.md §5와 TRACE에서 SRS-F-022 행의 DD-F-021 제거. SRS-F-032 행에 DD-F-021, DD-F-029 (확인) |

조치 완료 3, 부분 0, 미조치 0.

### 1.3 체크리스트 결과 (REQ-01~12, COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| REQ-01 | 영구·고유 ID | Pass | SSR 16건 유지, SRS 36건 유지, 재사용 없음 |
| REQ-02 | 기능/비기능 분류 | Pass | 변경 없음 |
| REQ-03 | 안전 요구 분리 | Pass | SRS 본문에 SSR 본문 없음. SSR은 SAF에만 있음 |
| REQ-04 | ISO 25010 분류 | Pass | 변경 없음 |
| REQ-05 | 한 요구 한 가지 | Pass | SSR-007 변경은 INPUT_INVALID 기록 범위 한 가지 |
| REQ-06 | 모호 표현 없이 검증 가능 | Pass | "state가 NORMAL이면", "그 외 state에서는 상태 이유 우선"으로 상태별 값이 정의됨 |
| REQ-07 | 검증 방법과 합격 기준 | **Fail** (Z-01) | SSR-007의 시험 열이 변경된 INPUT_INVALID 기록 범위(NORMAL 한정)를 검증하지 않는다. 시험은 거절과 규칙 미사용만 본다 |
| REQ-08 | 충돌·중복 없음 | Pass | SSR-007 v1.1, SRS-F-032, SRS-F-031, SSR-009, SSR-012, 6.3 순위 8이 같은 reason 규칙을 말한다 (3장 참조) |
| REQ-09 | "무엇"을 기술 | Pass | 구현 방법 지정 없음 |
| REQ-10 | 상향 추적과 내용 일치 | Pass | SSR-007의 상위 OEM-SR-003 편차는 A-25 (사용자 선택)에 기록. 상위 OEM 수용기준(거절)은 유지 |
| REQ-11 | 하향 추적 단절 없음 | Pass | TRACE 재생성에서 SRS-F-031, SRS-F-022 배정 정정 확인 |
| REQ-12 | 근거·우선순위·상태 | Pass | SAF 표에 상태, SRS 표에 근거·상태 존재 |
| COM-01 | 산출물 ID·버전·상태 | **Fail** (Z-02, Z-03, 경미) | ASM 상태 행이 A-23~A-25를 재리뷰 대상으로 나열하지 않는다. TRACE 버전이 1.5 그대로인데 Y-01·Y-03 반영으로 내용이 바뀌었다 |
| COM-02 | 형상 관리 | Pass | 커밋 b853568, docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | Pass | X-03, Y-01, Y-03 조치 확인 |
| COM-04 | 상·하위 ID·용어 일관성 | Pass | SSR-007 ↔ SRS-F-032 ↔ A-25 ↔ TRACE §3 ↔ DD-F-029 일치 |

요약 (REQ·COM 16항목): Pass 14 / Fail 2 / N/A 0 (Fail: REQ-07, COM-01)

### 1.4 결함 목록 (SWE.1)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| Z-01 | 중대 | SAFETY-REQ.md SSR-007 (시험 열) | SSR-007 v1.1의 새 조건(INPUT_INVALID는 state NORMAL에서만 기록, 그 외 상태는 상태 이유 우선)에 대한 검증 기준이 없다. 시험 열은 거절과 규칙 미사용만 본다. 변경된 요구가 검증되지 않으면 REQ-07을 충족하지 않는다. 같은 동작을 SRS-F-032가 검증하지만 SSR-007의 합격 기준으로 추적되지 않는다. | SSR-007 시험 열에 "형식 오류 IF-004 명령: state NORMAL → reason_code INPUT_INVALID, state OFF·DEGRADED·FAULT → 각각 IGNITION_OFF·DEGRADED_STALE·SENSOR_FAULT(규칙 결정 시 그 규칙 값), FAULT 첫 주기 SENSOR_FAULT 유지"를 추가하고 SSR-007 변경 이력과 재리뷰를 갱신한다. SAF 개정이므로 SWE.1 재리뷰 대상이다. |
| Z-02 | 경미 | ASSUMPTIONS.md 헤더 상태 행 | 상태 행이 "v1.1 이후 추가·변경(A-19 승인, A-20, A-21, A-22)은 재리뷰 대기"로만 쓰여 있고 A-23, A-24, A-25 (v1.4~v1.6 추가)가 빠졌다. | 상태 행을 "A-19 승인, A-20~A-25 재리뷰 대기"로 고친다. |
| Z-03 | 경미 | TRACE.md 헤더 버전 | TRACE 내용(SRS-F-022, SRS-F-031 배정)이 b853568에서 바뀌었는데 버전이 1.5 그대로다. 입력 버전 행은 SRS v1.6 등을 기록하고 있다. | TRACE 버전을 1.6으로 올린다. |

### 1.5 SWE.1 판정

**반려.** 중대 1건(Z-01, SSR-007 시험 기준)과 경미 2건(Z-02, Z-03)이 남아 있다. Z-01은 SAF 시험 정정이며 SAF를 다시 개정하므로 SWE.1 재리뷰 대상이다.

---

## Part 2. Iter-2 설계 10차 재리뷰

### 2.1 이전 결함 조치 상태 (V-05)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| V-05 | 경미 | DD-F-029 추적 열의 SSR-007 편차 미표시 | 조치 완료 | DDS.md DD-F-029 추적 열: "SRS-F-032, SRS-F-003, SSR-007 (NORMAL 한정, A-25)" 확인. DDS v0.8 |

조치 완료 1, 미조치 0.

### 2.2 교차 점검

- **DD-F-029 ↔ SRS-F-032 ↔ SSR-007 v1.1**: NORMAL에서만 INPUT_INVALID와 CMD_REJECTED_SPEED로 덮어쓰고 non-NORMAL에서는 덮어쓰지 않는다. 세 요구가 같은 규칙을 말한다 (확인).
- **DD-F-024 ↔ SSR-009·012 ↔ 6.3 순위 8 ↔ A-24**: DEGRADED는 DEGRADED_STALE, FAULT는 SENSOR_FAULT, OFF는 순위 3, NORMAL은 정보성 이유 또는 NONE. 일치 (확인).
- **ARCH.md ARCH-I-006 ↔ DDS.md DD-F-021·024·029**: 사후 조건과 함수 설명이 일치 (확인).
- **DDS.md §5 ↔ ARCH.md §8 ↔ TRACE**: SRS-F-031 (DD-F-021, 022, 024, 029), SRS-F-022 (DD-F-040, 043), SRS-F-032 (DD-F-021, 029). 세 곳이 일치 (확인).
- **ARCH-iter2, DDS-iter2**: 변경 없음 (확인). 정적 구조, 인터페이스, Web 필드 표 변경 없음.
- **DDS-iter2 필드 표 ↔ 설계 입력 유효성 13항목**: 변경 없음 (확인).

### 2.3 체크리스트 결과 (설계)

#### ARC-01~13

| ID | 판정 | 근거 |
|----|------|------|
| ARC-01~13 | Pass | 컴포넌트, 인터페이스, 배정, 결정 근거가 변경분과 일치. ARCH.md ARCH-I-006은 SSR-007 v1.1 규칙을 반영 (확인) |

#### DDS-01~12

| ID | 판정 | 근거 |
|----|------|------|
| DDS-01~11 | Pass | DD-F-029는 SSR-007 편차를 A-25로 명시. 함수 분할, 상수, 오류 처리 변경 없음 |
| DDS-12 | Pass | DD-F-029 → SRS-F-032, SRS-F-003, SSR-007 (편차 표시). DD-F-024 → SSR-009, SSR-012, SRS-F-003. §5 배정과 TRACE 일치 |

#### COM-01~04

| ID | 판정 | 근거 |
|----|------|------|
| COM-01 | Pass | ARCH.md 0.7, DDS.md 0.8, ARCH-iter2 0.5, DDS-iter2 0.5 |
| COM-02 | Pass | 커밋 b853568 |
| COM-03 | Pass | V-05, Y-01, Y-03 조치 확인 |
| COM-04 | Pass | DDS.md §5, ARCH.md §8, TRACE의 배정과 SSR·SRS·A-25 참조가 일치 |

### 2.4 결함 목록 (설계)

없음.

### 2.5 설계 판정

**승인.** 설계 문서 4건(ARCH-iter2 v0.5, DDS-iter2 v0.5, ARCH.md v0.7, DDS.md v0.8)에 결함이 없다. 다만 설계가 따르는 입력(SAF v1.1 SSR-007, SRS v1.6 SRS-F-032)이 SWE.1 재리뷰 승인을 받지 않았다. 따라서 설계 승인의 효력은 SWE.1 승인(Z-01 조치 포함) 이후에 발생하며, 그 전까지 다음 단계로 진행하지 않는다. 설계 승인이 조건부인 것은 아니다. 설계 내용에 대한 결함은 없고, 입력 승인 절차만 남아 있다.

---

## 3. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| (1) SWE.1 재리뷰 (SAF v1.1, ASM v1.6, SRS v1.6, TRACE v1.5) | **반려** | 중대 1 (Z-01), 경미 2 (Z-02, Z-03) |
| (2) 설계 10차 (ARCH-iter2 v0.5, DDS-iter2 v0.5, ARCH.md v0.7, DDS.md v0.8) | **승인** | 없음 (입력 SWE.1 승인 대기) |

- 이전 결함 조치: X-03, Y-01, Y-03, V-05 모두 조치 완료.
- 다음 단계 진행: **불가**. SWE.1 재리뷰가 반려이므로 Z-01(SAF SSR-007 시험 기준 추가, SAF 재개정)과 Z-02·Z-03(문서 정정)을 조치하고 재리뷰를 받아야 한다. 설계 승인은 SWE.1 승인 이후 효력이 있다.
- 재리뷰 시 Z-01~Z-03의 조치 상태를 표로 확인하고, SAFETY-REQ SSR-007 (시험 열, 변경 이력), SRS-F-032, ASSUMPTIONS 상태 행·A-25, TRACE 버전과 입력 버전을 함께 교차 점검한다.
