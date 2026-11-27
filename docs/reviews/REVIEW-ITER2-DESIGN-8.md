# 리뷰 보고서: (1) SWE.1 재리뷰 v1.5, (2) Iter-2 설계 8차 재리뷰

- 대상 (커밋 3b842e8, `feature/iter-0-requirements`):
  - (1) `docs/requirements/SRS.md` v1.5 (변경: SRS-F-002, 003, 018, 022, 030, 031, 6.3 순위 8), `docs/requirements/ASSUMPTIONS.md` v1.4 (A-19 범위 정정, A-23~A-25 추가), `docs/traceability/TRACE.md` v1.4 (입력 버전 SRS v1.5, ASM v1.4, ARCH v0.6, DDS v0.6 기록)
  - (2) `docs/detailed-design/DDS.md` v0.6 (DD-F-029 Arbiter_OverlayReason, DD-F-021), `docs/architecture/ARCH.md` v0.6 (ARCH-I-006 사후 조건), `docs/architecture/ARCH-iter2.md` v0.5, `docs/detailed-design/DDS-iter2.md` v0.5
- 기준선 대조: SRS v1.4(c39662b) 열 단위, SAF v1.0(변경 없음 확인), DDS.md·ARCH.md v0.5 대비 diff
- 이전 리뷰: REVIEW-ITER2-DESIGN-7.md (W-01, W-02, V-03)
- 일자: 2026-11-27
- 판정: (1) SWE.1 재리뷰 **반려**, (2) 설계 8차 **반려**

---

## Part 1. SWE.1 재리뷰 (SRS v1.5, ASSUMPTIONS v1.4, TRACE v1.4)

### 1.1 변경 범위 확인

- SRS v1.4 → v1.5 내용 변경 (열 단위): SRS-F-022(형식 오류 예외 (b) 추가, 시험 추가), SRS-F-003(형식 오류가 아닌 명령 한정 문구), SRS-F-031(형식 오류가 아닌 명령 한정). 6.3 순위 8 행에 형식 오류 문장 추가. SRS-F-002, 018, 030은 내용 동일.
- 변경 이력 1.5 행 확인. 상태 값: 변경 행은 "초안 (재리뷰 대기)", 나머지 "승인(v1.0)".
- SSR 16건 내용 동일 (SAF v1.0 변경 없음, 확인).
- ASM v1.4: A-19 행 범위가 NORMAL로 정정됨 (확인). A-23(F-022 NORMAL 한정), A-24(6.3 순위 8 state별 reason), A-25(형식 오류 INPUT_INVALID) 추가 확인. 편차 표에 A-23~A-25 포함 (확인).
- TRACE v1.4: 입력 버전 헤더 확인, §3에 A-19, A-13~A-18·A-20~A-25 포함 확인.

### 1.2 이전 결함 조치 상태 (W-01, W-02)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| W-01 | 중대 | 상태 억제 중 형식 오류 명령의 reason 우선순위 미정의 | 조치 완료 (새 결함 X-01) | SRS-F-022 (b) 추가: 형식 오류 운전자 명령이면 state와 무관하게 reason_code INPUT_INVALID, priority_reason 유지. SSR-007 문구와 일치. A-25 기록. 단 SSR-009, SSR-012와 충돌(X-01) |
| W-02 | 중대 | U-01·U-02 결정의 가정 기록 누락 | 조치 완료 | A-23(F-022 NORMAL 한정), A-24(6.3 순위 8 state별 reason), A-25 추가. A-19 범위 정정. 편차 표와 TRACE §3에 A-23~A-25 반영 (확인) |

조치 완료 2, 부분 0, 미조치 0.

### 1.3 체크리스트 결과 (REQ-01~12, COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| REQ-01 | 영구·고유 ID | Pass | ID 35건 유지, 신규·재사용 없음 |
| REQ-02 | 기능/비기능 분류 | Pass | 변경 행 모두 기능 |
| REQ-03 | 안전 요구 분리 | Pass | SRS 본문에 SSR 본문 없음. SSR 참조만 있음 |
| REQ-04 | ISO 25010 분류 | Pass | 변경 없음 |
| REQ-05 | 한 요구 한 가지 | **Fail** (Y-02, 경미) | SRS-F-022가 매핑 규칙, CMD_REJECTED_SPEED 예외 (a), INPUT_INVALID 예외 (b) 세 요구를 한 행에 담는다 |
| REQ-06 | 모호 표현 없이 검증 가능 | **Fail** (X-02, 경미) | F-022 (b)의 "형식 오류"가 SSR-007의 입력 오류 범위(누락, 형식, 범위, 값 집합 밖)와 다른 용어로 쓰였고, 시험 예시 "side 범위 밖"은 범위 오류다. (a)와 (b)가 같은 명령에 겹칠 때의 우선순위가 쓰여 있지 않다 |
| REQ-07 | 검증 방법과 합격 기준 | Pass | F-022 예외 시험에 형식 오류 4 state 케이스 추가 확인 |
| REQ-08 | 충돌·중복 없음 | **Fail** (X-01, 중대) | F-022 (b) "state와 무관하게 INPUT_INVALID"가 SSR-009(DEGRADED 전이 시 DEGRADED_STALE), SSR-012(FAULT 첫 주기 SENSOR_FAULT)와 같은 주기에 다른 reason을 요구한다 |
| REQ-09 | "무엇"을 기술 | Pass | 구현 방법 지정 없음 |
| REQ-10 | 상향 추적과 내용 일치 | **Fail** (X-01) | A-25는 SSR-007을 따른다고 쓰지만 SSR-009·012(상위 OEM-SR-003, OEM-SR-004 수용기준)와의 편차를 기록하지 않았다 |
| REQ-11 | 하향 추적 단절 없음 | **Fail** (Y-01, 경미) | TRACE의 SRS-F-031 행은 DD-F-021, DD-F-022만 배정하고 IGNITION_OFF와 INPUT_INVALID 값을 만드는 DD-F-024, DD-F-029를 배정하지 않는다 |
| REQ-12 | 근거·우선순위·상태 | Pass | 모든 표에 근거, 우선순위, 상태 존재 |
| COM-01 | 산출물 ID·버전·상태 | Pass | SRS 1.5, ASM 1.4, TRACE 1.4, SAF 1.0(변경 없음) 헤더 일관 |
| COM-02 | 형상 관리 | Pass | 커밋 3b842e8, docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | Pass | W-01, W-02 조치 확인 (1.2 표). 새 결함은 X-01, X-02, Y-01, Y-02 |
| COM-04 | 상·하위 ID·용어 일관성 | Pass | ASM 편차 표와 TRACE §3 일치, TRACE 입력 버전 기록 일치 |

요약 (REQ·COM 16항목): Pass 11 / Fail 5 / N/A 0 (Fail: REQ-05, REQ-06, REQ-08, REQ-10, REQ-11)

### 1.4 결함 목록 (SWE.1)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| X-01 | 중대 | SRS.md SRS-F-022 (예외 (b)), SAFETY-REQ.md SSR-009·SSR-012, ASSUMPTIONS A-25 | SRS-F-022 (b)는 형식 오류 운전자 명령이 있으면 state와 무관하게 reason_code를 INPUT_INVALID로 한다. SSR-009는 필수 입력이 stale 또는 INVALID가 된 전이 주기에 state DEGRADED와 reason_code DEGRADED_STALE을 기록한다고 쓰고, SSR-012는 FAULT의 첫 주기에 SENSOR_FAULT를 제공한다고 쓴다. 전이 또는 첫 주기에 형식 오류 명령이 함께 있으면 같은 reason_code에 두 shall이 충돌한다. A-25는 SSR-007을 따른다고만 쓰고, SSR-009·012를 덮어쓴다는 사실과 그 편차를 기록하지 않았다. SAF는 변경되지 않았으므로 상위 안전 요구의 수정이 없다. | 둘 중 하나를 결정한다. (a) SRS-F-022 (b)를 전이 주기와 FAULT 첫 주기에서는 SSR-009·012의 reason을 우선하도록 한정하고, 그 밖의 주기에 INPUT_INVALID를 쓴다. (b) SSR-009·012를 같은 방식으로 SAF에서 개정한다(SAF 개정은 SWE.1 재리뷰 대상). A-25에 편차(SSR-009·012와의 우선순위)를 기록한다. 사용자 판단이 필요하다. |
| X-02 | 경미 | SRS.md SRS-F-022 (b)와 시험 예시, SSR-007 | F-022 (b)는 "형식 오류"라고 쓰지만 SSR-007은 누락, 형식, 범위, 값 집합 밖을 포함한다. 시험 예시 "side 범위 밖"은 범위 오류이다. (a)(속도 거절 unlock)와 (b)(형식 오류)가 같은 명령에 겹칠 때 (b)가 우선한다는 문장이 없다. | (b)의 용어를 SSR-007의 입력 오류 범위와 같은 말로 쓰고, "형식 오류 명령은 속도 거절 예외 (a)에 우선한다"를 명시한다. |
| Y-01 | 경미 | TRACE.md SRS-F-031 행 (DD-F-021, DD-F-022) | SRS-F-031의 IGNITION_OFF와 INPUT_INVALID 값은 DD-F-024, DD-F-029가 만든다. TRACE 배정에 빠져 있다. | gen_trace.py 입력(DDS.md 배정 열)에서 SRS-F-031에 DD-F-024, DD-F-029를 포함해 재생성한다. |
| Y-02 | 경미 | SRS.md SRS-F-022 (전체 행) | 한 행에 매핑 규칙, 예외 (a), 예외 (b)가 함께 있다. 요구 단위가 섞인다. | 예외 (a)와 (b)를 F-022 아래 별도 요구(예: SRS-F-032, SRS-F-033)로 분리하거나, 기존 규칙 F-003에 (a)를 두고 F-022는 매핑만 남긴다. 분리 시 ID 이력 기록을 추가한다. |

### 1.5 SWE.1 판정

**반려.** 중대 1건(X-01)과 경미 3건(X-02, Y-01, Y-02)이 남아 있다. X-01은 SAF(SSR-009·012)와 SRS의 우선순위를 정하는 사용자 판단이 필요하다. X-02, Y-01, Y-02는 문서 정정이다.

---

## Part 2. Iter-2 설계 8차 재리뷰

### 2.1 이전 결함 조치 상태 (W-01 설계 측, V-03)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| V-03 | 중대 | 상태 억제 중 형식 오류 명령의 INPUT_INVALID 기록(SSR-007) 누락 | 조치 완료 (새 결함 V-04) | DDS.md DD-F-029를 Arbiter_OverlayReason으로 개정: INPUT_INVALID는 state와 무관하게 덮어쓰기, CMD_REJECTED_SPEED는 NORMAL에서만 덮어쓰기, priority_reason 유지. DD-F-021 설명 일치. ARCH.md ARCH-I-006 사후 조건에 같은 규칙 반영 (확인) |

조치 완료 1, 부분 0, 미조치 0.

### 2.2 교차 점검

- **DD-F-029(Arbiter_OverlayReason) ↔ SRS-F-022 (b)·(a) ↔ SSR-007**: 형식 오류 명령은 state와 무관하게 INPUT_INVALID이고, NORMAL 속도 거절은 CMD_REJECTED_SPEED이다. 순서(INPUT_INVALID 먼저)가 설계에 적혀 있다. SRS-F-022와 일치한다 (X-02에서 SRS 순서 문장이 없음을 기록).
- **DD-F-029 ↔ SSR-009·012 (전이·첫 주기)**: 형식 오류 명령이 전이 주기 또는 FAULT 첫 주기에 있으면 설계는 INPUT_INVALID를 쓰고 SSR-009·012의 DEGRADED_STALE·SENSOR_FAULT를 쓰지 않는다. 설계가 X-01의 한쪽을 택했다 (V-04).
- **DD-F-024 ↔ 6.3 순위 8 ↔ A-24**: 활성 규칙이 없는 주기의 state 이유(NORMAL 정보성 또는 NONE, DEGRADED DEGRADED_STALE, FAULT SENSOR_FAULT, OFF 순위 3). 일치 (확인).
- **ARCH.md ARCH-I-006 ↔ DDS.md**: 사후 조건과 DD-F-021·024·029가 일치한다 (확인).
- **TRACE ↔ DDS.md**: DD-F-029 배정 반영. SRS-F-031에 DD-F-024, DD-F-029가 빠짐 (Y-01).
- **ARCH-iter2, DDS-iter2**: 변경 없음 (확인).

### 2.3 체크리스트 결과 (설계)

#### ARC-01~13

| ID | 판정 | 근거 |
|----|------|------|
| ARC-01~12 | Pass | 변경 없음 또는 V-03 반영 |
| ARC-03 | Pass | ARCH-I-006 사후 조건이 상태별 reason과 형식 오류 덮어쓰기를 모두 정의 |
| ARC-13 | Pass | DD-F-029 본문에 A-19, A-23, A-25, SSR-007 근거 기록 |

#### DDS-01~12

| ID | 판정 | 근거 |
|----|------|------|
| DDS-01~10 | Pass | 변경 없음 |
| DDS-11 | **Fail** (V-04) | 전이 주기와 FAULT 첫 주기에 형식 오류 명령이 있으면 설계는 SSR-009·012의 reason을 덮어쓴다. 상위 안전 요구와의 충돌을 설계가 처리하지 않는다 |
| DDS-12 | Pass | DD-F-029 → SRS-F-003, SRS-F-022, SSR-007. DD-F-024 → SSR-009, SSR-012, SRS-F-003. 추적 성립 |

#### COM-01~04

| ID | 판정 | 근거 |
|----|------|------|
| COM-01 | Pass | ARCH.md 0.6, DDS.md 0.6, ARCH-iter2 0.5, DDS-iter2 0.5 |
| COM-02 | Pass | 커밋 3b842e8 |
| COM-03 | Pass | V-03 조치 확인 |
| COM-04 | Pass | ARCH.md ↔ DDS.md ↔ SRS F-022 일치. SAF 충돌은 DDS-11에서 기록 |

### 2.4 결함 목록 (설계)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| V-04 | 중대 | DDS.md DD-F-029 (Arbiter_OverlayReason), DD-F-021; ARCH.md ARCH-I-006 | 설계는 형식 오류 명령이 있으면 state와 무관하게 reason을 INPUT_INVALID로 덮어쓴다. 전이 주기(SSR-009)와 FAULT 첫 주기(SSR-012)에는 SAF의 상태 reason shall과 충돌한다. 설계가 SRS-F-022 (b)를 따르지만 X-01이 SRS에서 결정되지 않아 이 동작을 확정할 수 없다. | X-01 결정 후 DD-F-029에 전이 주기와 FAULT 첫 주기 예외를 넣거나, SAF 개정에 맞춘다. ARCH-I-006도 같은 규칙을 반영한다. 결정 전에는 이 경우의 시험 케이스를 확정하지 않는다. |

### 2.5 설계 판정

**반려.** V-04(중대)는 SRS·SAF X-01 결정에 종속된다.

---

## 3. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| (1) SWE.1 재리뷰 (SRS v1.5, ASSUMPTIONS v1.4, TRACE v1.4) | **반려** | 중대 1 (X-01), 경미 3 (X-02, Y-01, Y-02) |
| (2) 설계 8차 (DDS.md v0.6, ARCH.md v0.6, ARCH-iter2 v0.5, DDS-iter2 v0.5) | **반려** | 중대 1 (V-04, X-01 종속) |

- 이전 결함 조치: W-01(설계 측 V-03), W-02 조치 완료.
- 사용자 판단 필요: X-01 (형식 오류 INPUT_INVALID와 SSR-009·012 상태 reason의 우선순위). 결정에 따라 SRS-F-022 (b)를 한정하거나 SAF를 개정한다.
- 문서 정정으로 해결: X-02, Y-01, Y-02.
- 재리뷰 시 X-01, X-02, Y-01, Y-02, V-04의 조치 상태를 표로 확인하고, SRS F-022·F-003·F-031·6.3 순위 8, SAFETY-REQ SSR-007·009·012, ASSUMPTIONS A-19·A-23~A-25와 편차 표, TRACE 배정, DDS.md DD-F-021·024·029, ARCH.md ARCH-I-006을 함께 교차 점검한다.
