# 리뷰 보고서: SWE.1 재리뷰 (입력 문서 승인 여부 판정, Iter-2 설계 11차)

- 대상 (커밋 5afdaa7, `feature/iter-0-requirements`):
  - `docs/safety/SAFETY-REQ.md` v1.2 (SSR-007 시험 열 보강, 변경 이력 1.2 행)
  - `docs/requirements/ASSUMPTIONS.md` v1.7 (헤더 상태 정정)
  - `docs/requirements/SRS.md` v1.6 (변경 없음 확인)
  - `docs/traceability/TRACE.md` v1.7 (입력 버전 기록)
- 설계 문서(ARCH-iter2 v0.5, DDS-iter2 v0.5, ARCH.md v0.7, DDS.md v0.8)는 10차에서 승인되었으므로 이번 판정 대상이 아니다. 입력 승인 여부만 판정한다.
- 이전 리뷰: REVIEW-ITER2-DESIGN-10.md (Z-01, Z-02, Z-03)
- 기준선 대조: b853568 → 5afdaa7 diff (5 files), SRS ee9b511 → 5afdaa7 diff 0줄.
- 일자: 2026-11-27
- 판정: **SWE.1 반려** (경미 1건 AA-01 잔존)

---

## 1. 이전 결함 조치 상태 (Z-01, Z-02, Z-03)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| Z-01 | 중대 | SSR-007 시험 기준에 변경된 INPUT_INVALID 범위(NORMAL 한정)와 상태별 reason 검증 누락 | 조치 완료 | SSR-007 시험 열에 형식 오류 IF-004 명령(side 범위 밖)의 케이스 추가: NORMAL → INPUT_INVALID(priority_reason은 결정 규칙 값, 없으면 NONE), OFF → IGNITION_OFF, DEGRADED → DEGRADED_STALE, FAULT 첫 주기 포함 → SENSOR_FAULT, 규칙 결정 시 그 규칙 reason. SRS-F-032 시험과 일치 확인. 변경 이력 1.2 행 추가 |
| Z-02 | 경미 | ASM 헤더 상태가 A-23~A-25를 나열하지 않음 | 조치 완료 | ASM 헤더 상태 "A-19 승인, A-20~A-24 작성자 결정, A-25 승인" 확인. 편차 표와 TRACE §3과 일치 |
| Z-03 | 경미 | TRACE 버전 1.5 미상향 | 조치 완료 | TRACE 헤더 버전 1.7, 입력 문서 버전 "SRS v1.6, SAFETY-REQ v1.2, ASSUMPTIONS v1.7, ARCH v0.7, ARCH-iter2 v0.5, DDS v0.8, DDS-iter2 v0.5" 확인 |

조치 완료 3, 부분 0, 미조치 0.

---

## 2. 체크리스트 결과 (REQ-01~12, COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| REQ-01 | 영구·고유 ID | Pass | SSR 16건, SRS 36건 유지, 재사용 없음 |
| REQ-02 | 기능/비기능 분류 | Pass | 변경 없음 |
| REQ-03 | 안전 요구 분리 | Pass | SRS 본문에 SSR 본문 없음 |
| REQ-04 | ISO 25010 분류 | Pass | 변경 없음 |
| REQ-05 | 한 요구 한 가지 | Pass | SSR-007은 입력 검증 거절과 그에 따른 reason 기록 두 문장이지만 한 규칙의 범위 |
| REQ-06 | 모호 표현 없이 검증 가능 | Pass | state별 reason 값과 FAULT 첫 주기 조건이 명시됨 |
| REQ-07 | 검증 방법과 합격 기준 | Pass | SSR-007 시험이 NORMAL, OFF, DEGRADED, FAULT, FAULT 첫 주기를 모두 다룸 (Z-01 확인) |
| REQ-08 | 충돌·중복 없음 | Pass | SSR-007 v1.2, SRS-F-032, SRS-F-031, SSR-009, SSR-012, 6.3 순위 8, A-25가 같은 reason 규칙 |
| REQ-09 | "무엇"을 기술 | Pass | 구현 방법 지정 없음 |
| REQ-10 | 상향 추적과 내용 일치 | Pass | SSR-007의 OEM-SR-003 편차는 A-25 (사용자 선택)로 기록 |
| REQ-11 | 하향 추적 단절 없음 | Pass | TRACE v1.7이 입력 버전과 배정을 기록. 설계 승인 시점의 배정과 일치 |
| REQ-12 | 근거·우선순위·상태 | Pass | 상태 열과 근거 존재 |
| COM-01 | 산출물 ID·버전·상태 | **Fail** (AA-01, 경미) | SAF 헤더 상태 "v1.1의 SSR-007 변경은 재리뷰 대기"가 v1.2 변경을 가리키지 않는다. v1.2는 같은 SSR-007의 시험 열 변경이므로 헤더 상태가 현재 재리뷰 대상과 맞지 않는다 |
| COM-02 | 형상 관리 | Pass | 커밋 5afdaa7, docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | Pass | Z-01, Z-02, Z-03 조치 확인 |
| COM-04 | 상·하위 ID·용어 일관성 | Pass | SSR-007 ↔ SRS-F-032 ↔ A-25 ↔ TRACE §3 ↔ DD-F-029 일치. SRS 6.3 순위 8 일치 |

요약 (REQ·COM 16항목): Pass 15 / Fail 1 / N/A 0 (Fail: COM-01)

---

## 3. 결함 목록

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| AA-01 | 경미 | SAFETY-REQ.md 헤더 상태 행 (7행) | "v1.0 승인. v1.1의 SSR-007 변경은 재리뷰 대기"로 쓰여 있으나 현재 SSR-007은 v1.2다. 헤더가 재리뷰 대상을 정확히 가리키지 않는다. | 헤더 상태를 "v1.0 승인. v1.1·v1.2의 SSR-007 변경(상태 이유 우선, 시험 기준 보강)은 재리뷰 대기"로 고친다. |

---

## 4. 확인 사항 (결함 없음)

- SSR 15건은 v1.0 승인 내용과 상태가 같다 (b853568 이후 변경 없음).
- SRS v1.6은 5afdaa7에서 변경되지 않았다 (diff 0줄). SRS-F-032와 SSR-007 v1.2 문구가 일치한다.
- ASM v1.7: A-19 승인, A-20~A-24 작성자 결정, A-25 승인 (편차 표와 TRACE §3 일치).
- TRACE v1.7: 입력 버전 행이 현재 문서 버전과 일치한다.
- 수치 (300 ms, 200 ms, 100 ms, 3 km/h, 10 s, 100건, 0.0~300.0 km/h)와 OEM 원문 대조 결과 변경 없음.
- SRS 본문과 SAF 본문에 ASIL 표기 없음 (SAF 본문의 "ASIL B 입력" 표기는 SAF 자체 분류이며 SRS에는 없음).

---

## 5. SWE.1 판정

**반려.** 입력 문서(SAF v1.2, ASM v1.7, SRS v1.6, TRACE v1.7)에 경미 1건(AA-01, SAF 헤더 상태 행)이 남아 있다. 내용 결함은 없고 헤더 상태 한 줄의 정정이다. 이 정정이 끝나면 헤더만 바뀌는 것이므로 재리뷰 없이 확인할 수 있다.

설계 승인(10차)의 효력은 SWE.1 승인 이후에 발생한다. 이 판정이 반려이므로 설계 효력도 아직 발생하지 않는다. AA-01 정정 후 재리뷰를 받아야 한다.

---

## 6. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| SWE.1 입력 승인 (SAF v1.2, ASM v1.7, SRS v1.6, TRACE v1.7) | **반려** | 경미 1 (AA-01) |

- 이전 결함(Z-01 중대, Z-02·Z-03 경미): 모두 조치 완료.
- 다음 단계 진행: **불가**. AA-01 정정 후 재리뷰가 필요하다.
