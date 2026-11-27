# 리뷰 보고서: (1) SWE.1 재리뷰 v1.6, (2) Iter-2 설계 9차 재리뷰

- 대상 (커밋 ee9b511, `feature/iter-0-requirements`):
  - (1) `docs/requirements/SRS.md` v1.6 (신설 SRS-F-032, 변경 SRS-F-002, 003, 018, 022, 030, 031, 6.3 순위 8, 개수 요약, 변경 이력), `docs/requirements/ASSUMPTIONS.md` v1.5 (A-25), `docs/traceability/TRACE.md` v1.5
  - (2) `docs/detailed-design/DDS.md` v0.7 (DD-F-029 Arbiter_OverlayReason, DD-F-021, §5 배정), `docs/architecture/ARCH.md` v0.7 (ARCH-I-006, §8 배정), `docs/architecture/ARCH-iter2.md` v0.5, `docs/detailed-design/DDS-iter2.md` v0.5
  - 참고: `docs/safety/SAFETY-REQ.md` v1.0 (변경 없음)
- 기준선 대조: SRS v1.5(3b842e8) 열 단위 비교, SAF v1.0 내용 확인, DDS.md·ARCH.md 8차 판 대비 diff
- 이전 리뷰: REVIEW-ITER2-DESIGN-8.md (X-01, X-02, Y-01, Y-02, V-04)
- 일자: 2026-11-27
- 판정: (1) SWE.1 재리뷰 **반려**, (2) 설계 9차 **반려**

---

## Part 1. SWE.1 재리뷰 (SRS v1.6, ASSUMPTIONS v1.5, TRACE v1.5)

### 1.1 변경 범위 확인

- 신설: SRS-F-032 (운전자 명령 이유 덮어쓰기). 요구 수 F 24, 합계 36 (§7 개수 요약 일치, 실제 행 수와 일치 확인).
- 변경: SRS-F-022는 매핑만 남기고 예외를 F-032로 분리. F-003, F-031, 6.3 순위 8은 F-032를 참조. F-002, F-018, F-030은 내용 동일.
- 변경 이력 1.6 행 확인.
- SAF v1.0은 SSR 16건 내용 동일 (변경 없음). 단 SSR-007 문구("거절된 IF-004 명령은 … INPUT_INVALID를 기록한다")는 SRS-F-032의 non-NORMAL 상태 규칙과 다르다 (X-03).
- ASM v1.5: A-25에 사용자 선택 "상태 이유 우선 (추천)"과 SSR-007 편차(NORMAL 한정) 기록 확인. 편차 표와 TRACE §3에 A-25 반영 확인.
- TRACE v1.5: SRS-F-032 행 있음 (DD-F-021, DD-F-029). SRS-F-031 행은 여전히 DD-F-021, DD-F-022만 (Y-01 미조치).

### 1.2 이전 결함 조치 상태 (X-01, X-02, Y-01, Y-02)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| X-01 | 중대 | SRS-F-022 (b) vs SSR-009·012 | 조치 완료 (새 결함 X-03) | 사용자 선택 "상태 이유 우선 (추천)". SRS-F-032가 non-NORMAL에서 상태 이유를 우선. FAULT 첫 주기 형식 오류 시험 추가 확인 |
| X-02 | 경미 | 용어(형식 오류)와 우선순위(a 대 b) | 조치 완료 | F-032가 "입력 검증으로 거절된 운전자 명령(SSR-007의 누락, 형식, 범위, 값 집합 오류)"으로 SSR-007과 용어를 맞춤. (a)와 (b)가 한 행에 있고 시험에서 순서가 확인됨 |
| Y-01 | 경미 | TRACE SRS-F-031 배정 누락 | **부분 조치** | TRACE의 SRS-F-031 행은 여전히 DD-F-021, DD-F-022. DD-F-024, DD-F-029 배정 없음 (확인). DDS.md §5의 SRS-F-031도 DD-F-021, DD-F-022만 (확인). 조치 보고(SRS-F-032 추가)는 Y-01의 대상(F-031)을 다루지 않았다 |
| Y-02 | 경미 | 한 행에 세 요구 | 조치 완료 | 예외 (a), (b)가 SRS-F-032로 분리. F-022는 매핑만 남김. ID 이력은 §8 "신설"로 기록되어 있음 (확인) |

조치 완료 3, 부분 1 (Y-01), 미조치 0.

### 1.3 체크리스트 결과 (REQ-01~12, COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| REQ-01 | 영구·고유 ID | Pass | 신규 SRS-F-032, 기존 ID 재사용 없음. 폐기 ID는 §8에만 있음 |
| REQ-02 | 기능/비기능 분류 | Pass | F-032 기능 |
| REQ-03 | 안전 요구 분리 | Pass | SRS 본문에 SSR 본문 없음. SSR-007과의 차이는 REQ-08(X-03) |
| REQ-04 | ISO 25010 분류 | Pass | 변경 없음 |
| REQ-05 | 한 요구 한 가지 | Pass | F-032는 NORMAL의 두 덮어쓰기 규칙과 non-NORMAL의 상태 이유 규칙을 하나의 "이유 덮어쓰기" 요구로 다룸. F-022는 매핑만 |
| REQ-06 | 모호 표현 없이 검증 가능 | Pass | 입력 검증 거절 범위가 SSR-007과 같은 말로 정의됨. state별 reason 값 명시 |
| REQ-07 | 검증 방법과 합격 기준 | Pass | F-032 시험이 NORMAL 두 케이스와 DEGRADED, FAULT, OFF, FAULT 첫 주기를 포함 |
| REQ-08 | 충돌·중복 없음 | **Fail** (X-03) | SRS-F-032(non-NORMAL에서 INPUT_INVALID를 쓰지 않음)와 SSR-007("거절된 IF-004 명령은 … INPUT_INVALID를 기록한다")의 충돌. 편차는 A-25에 기록되었으나 SAF 요구는 그대로다 |
| REQ-09 | "무엇"을 기술 | Pass | 구현 방법 지정 없음 |
| REQ-10 | 상향 추적과 내용 일치 | Pass | A-25 편차 기록과 사용자 선택 근거가 있음 |
| REQ-11 | 하향 추적 단절 없음 | **Fail** (Y-01, Y-03, 경미) | TRACE의 SRS-F-031 행에 IGNITION_OFF를 만드는 DD-F-024, DD-F-029가 없다. SRS-F-022 행의 DD-F-021은 F-022 매핑과 무관한 덮어쓰기 적용 단계로 남아 있다 |
| REQ-12 | 근거·우선순위·상태 | Pass | 모든 표에 근거, 우선순위, 상태 존재 |
| COM-01 | 산출물 ID·버전·상태 | Pass | SRS 1.6, ASM 1.5, TRACE 1.5, SAF 1.0(변경 없음) |
| COM-02 | 형상 관리 | Pass | 커밋 ee9b511, docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | **Fail** (Y-01 부분 조치) | Y-01 (TRACE/DDS 배정)이 여전히 미조치 |
| COM-04 | 상·하위 ID·용어 일관성 | **Fail** (X-03, Y-01, Y-03) | SAF SSR-007과 SRS 불일치. TRACE·DDS.md의 F-031과 F-022 배정 불일치 |

요약 (REQ·COM 16항목): Pass 12 / Fail 4 / N/A 0 (Fail: REQ-08, REQ-11, COM-03, COM-04)

### 1.4 결함 목록 (SWE.1)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| X-03 | 중대 | SAFETY-REQ.md SSR-007 (v1.0 그대로), SRS-F-032 (non-NORMAL), ASSUMPTIONS A-25 | SSR-007은 ASIL B 입력(OEM-SR-003)에서 도출된 안전 요구이며 "거절된 IF-004 명령은 무시하고 reason_code INPUT_INVALID를 기록한다"고 쓴다. SRS-F-032는 OFF, DEGRADED, FAULT에서 이 기록을 하지 않는다. 사용자 선택으로 편차를 허용했고 A-25에 기록했다. 그러나 SAF 요구는 개정되지 않았으므로 상위 안전 요구와 하위 SRS 요구가 함께 승인된 상태가 되지 않는다. 편차는 안전 요구 수준에서 승인되어야 한다. | (a) SSR-007을 SRS-F-032와 같은 상태 규칙으로 SAF 개정하고 SWE.1 재리뷰를 받는다(SAF 개정은 재리뷰 대상). 또는 (b) 편차를 SAF 쪽에 주석으로 남기고 안전 관점의 영향(진단 정보 손실)을 A-25에 적어 사용자가 안전 편차로 승인한다. 사용자 판단이 필요하다. |
| Y-01 | 경미 | TRACE.md SRS-F-031 행, DDS.md §5 SRS-F-031 행 | SRS-F-031의 IGNITION_OFF는 DD-F-024(state 이유)와 DD-F-029(덮어쓰기 없음)에서 나오는데, 두 배정 표 모두 DD-F-021, DD-F-022만 있다. 이전 Y-01 조치가 F-031을 빠뜨렸다. | DDS.md §5 SRS-F-031에 DD-F-024, DD-F-029를 추가하고 gen_trace.py로 TRACE를 재생성한다. |
| Y-03 | 경미 | DDS.md §5 SRS-F-022 행 (DD-F-021), TRACE SRS-F-022 행 (DD-F-021, DD-F-040, DD-F-043) | F-022는 이제 매핑만 다룬다. DD-F-021은 덮어쓰기 적용 단계(F-032)이므로 F-022 배정으로는 부적절하다. F-022 매핑 구현은 DD-F-040, DD-F-043이다. | DDS.md §5와 TRACE에서 SRS-F-022의 DD-F-021 배정을 SRS-F-032로 옮긴다(F-032 행에 이미 DD-F-021, DD-F-029가 있다). |

### 1.5 SWE.1 판정

**반려.** 중대 1건(X-03)과 경미 2건(Y-01, Y-03)이 남아 있다. X-03은 SSR-007(SAF) 개정 또는 안전 편차 승인이라는 사용자 판단이 필요하다. Y-01, Y-03은 배정 표 정정이다.

---

## Part 2. Iter-2 설계 9차 재리뷰

### 2.1 이전 결함 조치 상태 (V-04)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| V-04 | 중대 | DD-F-029의 형식 오류 덮어쓰기가 SSR-009·012 reason을 덮어씀 | 조치 완료 | DDS.md DD-F-029를 NORMAL 한정 덮어쓰기로 개정: 운전자 순위가 state로 제외되면(state가 NORMAL이 아니면) REASON_NONE. 그 외 info_reason이 CMD_REJECTED_SPEED 또는 INPUT_INVALID이면 그 값. DD-F-021 설명 일치. ARCH.md ARCH-I-006 사후 조건 일치. non-NORMAL의 상태 reason 우선 확인 |

조치 완료 1, 미조치 0.

### 2.2 교차 점검

- **DD-F-029 ↔ SRS-F-032**: NORMAL에서만 CMD_REJECTED_SPEED와 INPUT_INVALID로 덮어쓰고 priority_reason은 유지. non-NORMAL은 덮어쓰지 않아 DD-F-024의 state 이유 또는 OFF의 IGNITION_OFF가 남는다. 일치 (확인).
- **DD-F-024 ↔ SSR-009·012 ↔ 6.3 순위 8 ↔ A-24**: DEGRADED는 DEGRADED_STALE, FAULT는 SENSOR_FAULT, OFF는 순위 3. 일치 (확인).
- **ARCH.md ARCH-I-006 ↔ DDS.md**: 사후 조건과 DD-F-021·024·029 일치 (확인).
- **DD-F-029 ↔ SSR-007 (추적)**: DD-F-029의 추적 열은 SSR-007을 적고 있지만, 이 함수는 non-NORMAL에서 SSR-007의 INPUT_INVALID 기록을 하지 않는다. 추적 열이 편차를 드러내지 않는다 (V-05, 경미).
- **DDS.md §5 및 TRACE 배정**: SRS-F-031 (Y-01), SRS-F-022 (Y-03) 배정 불일치.
- **ARCH-iter2, DDS-iter2**: 변경 없음. ARCH-iter2 정적 구조와 DDS-iter2 필드 표 확인.

### 2.3 체크리스트 결과 (설계)

#### ARC-01~13

| ID | 판정 | 근거 |
|----|------|------|
| ARC-01~13 | Pass | ARCH-I-006 사후 조건이 SRS-F-032와 일치, §8 배정에 SRS-F-032 포함 (ARCH.md 171행) |

#### DDS-01~12

| ID | 판정 | 근거 |
|----|------|------|
| DDS-01~11 | Pass | V-04 조치 확인. 편차는 A-25에 기록되어 설계 함수 결정이 추적 가능 |
| DDS-12 | **Fail** (Y-01, Y-03, V-05) | DDS.md §5의 SRS-F-031 배정에 DD-F-024, DD-F-029가 없고, SRS-F-022에 DD-F-021이 배정되어 있다. DD-F-029 추적 열이 SSR-007 편차를 드러내지 않는다 |

#### COM-01~04

| ID | 판정 | 근거 |
|----|------|------|
| COM-01 | Pass | ARCH.md 0.7, DDS.md 0.7, ARCH-iter2 0.5, DDS-iter2 0.5 |
| COM-02 | Pass | 커밋 ee9b511 |
| COM-03 | Pass | V-04 조치 확인 |
| COM-04 | **Fail** (Y-01, Y-03) | 배정 불일치 (SWE.1과 공유) |

### 2.4 결함 목록 (설계)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| V-05 | 경미 | DDS.md DD-F-029 추적 열 (SRS-F-032, SRS-F-003, SSR-007) | DD-F-029는 non-NORMAL에서 SSR-007의 INPUT_INVALID를 기록하지 않는다(A-25 편차). 추적 열이 SSR-007을 편차 없이 적고 있다. | 추적 열에 "SSR-007: NORMAL에서만 준수, 편차 A-25"를 명시한다. X-03 결정 후 SAF 개정이 있으면 함께 갱신한다. |
| Y-01, Y-03 | 경미 | DDS.md §5 (SRS-F-031, SRS-F-022 행) | SWE.1 항목과 같음. DDS.md 배정을 TRACE 재생성의 입력으로 정정해야 한다. | §5 배정 정정 후 gen_trace.py 재생성. |

### 2.5 설계 판정

**반려.** 경미 결함 V-05, Y-01, Y-03이 남아 있다 (DDS-12, COM-04). V-04는 조치되었다. 설계 기능 자체의 결함은 없다. 다만 X-03 결정(SSR-007 개정 또는 안전 편차 승인)에 따라 V-05의 문구가 바뀔 수 있다.

---

## 3. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| (1) SWE.1 재리뷰 (SRS v1.6, ASSUMPTIONS v1.5, TRACE v1.5) | **반려** | 중대 1 (X-03), 경미 2 (Y-01, Y-03) |
| (2) 설계 9차 (DDS.md v0.7, ARCH.md v0.7, ARCH-iter2 v0.5, DDS-iter2 v0.5) | **반려** | 경미 2 (V-05, Y-01/Y-03 공유) |

- 이전 결함 조치: X-01, X-02, Y-02, V-04 조치 완료. Y-01은 부분 조치 (F-031 배정 누락).
- 사용자 판단 필요: X-03 (SSR-007의 안전 요구 개정 또는 안전 편차 승인). 이 결정이 A-25, SAF, V-05를 함께 결정한다.
- 문서 정정으로 해결: Y-01, Y-03, V-05 (배정 표 정정과 gen_trace.py 재생성).
- 재리뷰 시 X-03, Y-01, Y-03, V-05의 조치 상태를 표로 확인하고, SRS F-022·F-032·F-031·6.3 순위 8, SAFETY-REQ SSR-007, ASSUMPTIONS A-25와 편차 표, TRACE 배정, DDS.md §5·DD-F-029, ARCH.md §8을 함께 교차 점검한다.
