# 리뷰 보고서: (1) SWE.1 재리뷰 v1.3, (2) Iter-2 설계 6차 재리뷰

- 대상 (커밋 44ab31a → HEAD, `feature/iter-0-requirements`):
  - (1) `docs/requirements/SRS.md` v1.3, `docs/requirements/ASSUMPTIONS.md` v1.3, `docs/safety/SAFETY-REQ.md`(상태 표기), `docs/traceability/TRACE.md`(gen_trace.py 재생성)
  - (2) `docs/detailed-design/DDS.md` v0.4(DD-F-024 H-01 조치), `docs/architecture/ARCH.md` v0.4, `docs/architecture/ARCH-iter2.md` v0.5, `docs/detailed-design/DDS-iter2.md` v0.5
- 기준선 대조: SRS v1.2(d15ad37), SAFETY-REQ v1.0 내용(69da6db), TRACE 1.2, DDS.md 0.3, ARCH.md 0.3 열 단위와 diff 비교
- 이전 리뷰: REVIEW-ITER2-DESIGN-5.md (T-01~T-06, H-01)
- 일자: 2026-11-27
- 판정: (1) SWE.1 재리뷰 **반려**, (2) 설계 6차 **반려**

---

## Part 1. SWE.1 재리뷰 (SRS v1.3, ASSUMPTIONS v1.3, SAFETY-REQ, TRACE)

### 1.1 변경 범위 확인

- SRS v1.2 → v1.3 내용 변경: SRS-F-002, SRS-F-003 (열 단위 비교로 확인). 나머지 33행은 내용 동일.
- 상태 값: "승인(v1.0)" 30행, "초안 (재리뷰 대기)" 5행 (F-002, 003, 018, 022, 030). 헤더 상태 체계 정의 유지.
- SAFETY-REQ: 16개 SSR 내용 동일 (v1.2 대비 확인). 헤더 상태 "승인(v1.0): aspice-reviewer 10차 승인 (2026-11-27). 내용 변경 없음"으로 SRS 체계와 일치 (T-06 확인).
- ASSUMPTIONS v1.3: A-22 대상 항목 "OEM-FR-004 (SRS-F-018)" 반영 (T-05 확인). 편차 표의 A-19 행과 "A-20~A-22" 항목 확인.
- TRACE §3: A-19, A-20, A-21, A-22가 편차 기록에 포함됨 (T-03 확인). §4 Iter-2 범위 목록 확인.

### 1.2 이전 결함 조치 상태 (T-01~T-06)

| 번호 | 심각도 | 내용 | 조치 상태 | 근거 |
|------|--------|------|-----------|------|
| T-01 | 중대 | DEGRADED·FAULT 미적용 명령의 reason_code 미정의 | 조치 완료 (새 결함 U-01, U-02) | SRS-F-003에 OFF → IGNITION_OFF, DEGRADED → DEGRADED_STALE(SSR-009), FAULT → SENSOR_FAULT(SSR-012), 다른 규칙이 결정하면 그 규칙 값 정의. 시험 케이스 추가 |
| T-02 | 중대 | 변경 이력에 F-022 누락 | 조치 완료 | 변경 이력 1.2 행에 SRS-F-022(예외 시험 보강) 추가, 1.3 행 추가 확인 |
| T-03 | 중대 | TRACE §3 편차 기록 누락 | 조치 완료 | TRACE §3에 A-19(사용자 선택), A-13~A-18·A-20~A-22 행 확인. ASM 편차 표와 일치 |
| T-04 | 경미 | F-002 상위 규칙 결정 시 reason 시험 부재 | 조치 완료 | F-002 시험에 "더 높은 순위 규칙이 결정하는 주기의 reason_code" 케이스 추가 확인 (표현이 구체적 값을 쓰지 않음, 경미 참고) |
| T-05 | 경미 | A-22 대상 OEM 항목 누락 | 조치 완료 | A-22 "OEM-FR-004 (SRS-F-018)" 확인 |
| T-06 | 경미 | SAF 헤더 상태 표기 불일치 | 조치 완료 | "승인(v1.0): aspice-reviewer 10차 승인 (2026-11-27). 내용 변경 없음" 확인 |

조치 완료 6 (T-01은 새 결함 U-01·U-02로 이어짐), 부분 0, 미조치 0.

### 1.3 체크리스트 결과 (REQ-01~12, COM-01~04)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| REQ-01 | 영구·고유 ID | Pass | ID 35건, 신규·재사용 없음 |
| REQ-02 | 기능/비기능 분류 | Pass | 변경 행 모두 기능 |
| REQ-03 | 안전 요구 분리 | Pass | SRS 본문에 SSR 본문 없음. SSR은 ID 참조 |
| REQ-04 | ISO 25010 분류 | Pass | 변경 없음 |
| REQ-05 | 한 요구 한 가지 | Pass | F-003은 거절 조건, 기록 값, 참조의 한 규칙 |
| REQ-06 | 모호 표현 없이 검증 가능 | Pass | T-01로 미적용 명령의 reason이 정의됨. 상태별 값이 명시됨 |
| REQ-07 | 검증 방법과 합격 기준 | **Fail** (U-01) | FAULT 상태이면서 속도 3.0 km/h 이상인 unlock(자동 잠금이 결정) 케이스가 시험에 없고, 그 reason이 SRS 본문에서 상충함 |
| REQ-08 | 충돌·중복 없음 | **Fail** (U-01, U-02) | F-003·F-022의 속도 거절 규칙과 F-003의 상태 규칙이 같은 명령에 다른 reason을 요구. 6.3 순위 8(NONE)과 SSR-009·012(DEGRADED_STALE·SENSOR_FAULT)가 상충 |
| REQ-09 | "무엇"을 기술 | Pass | 구현 방법 지정 없음 |
| REQ-10 | 상향 추적과 내용 일치 | Pass | A-22 대상 OEM-FR-004 명시. F-003·F-022의 편차는 A-19에 기록 |
| REQ-11 | 하향 추적 단절 없음 | Pass | TRACE 재생성에 DD-F-029 및 DD-F-024 배정 반영 |
| REQ-12 | 근거·우선순위·상태 | Pass | 모든 표에 근거, 우선순위, 상태 존재 |
| COM-01 | 산출물 ID·버전·상태 | **Fail** (U-03, 경미) | TRACE는 재생성되었으나 헤더 버전이 1.2 그대로다. 내용 변경 시 버전이 올라가야 한다 |
| COM-02 | 형상 관리 | Pass | 커밋 44ab31a 이후 docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | Pass | T-01~T-06 조치 확인 (1.2 표). 새 결함은 U-01~U-03 |
| COM-04 | 상·하위 ID·용어 일관성 | Pass | TRACE §3과 ASM 편차 표 일치, SAF 상태 체계 일치 |

요약 (REQ·COM 16항목): Pass 13 / Fail 3 / N/A 0 (Fail: REQ-07, REQ-08, COM-01)

### 1.4 결함 목록 (SWE.1)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| U-01 | 중대 | SRS.md SRS-F-003 (첫 문장, 상태 억제 문장), SRS-F-022 (예외), SRS.md 6.3 | 운전자 unlock이 FAULT 또는 DEGRADED 상태에서 적용되지 않고 동시에 속도 3.0 km/h 이상이어서 거절되는 경우, 요구가 두 개로 갈린다. F-003 첫 문장과 F-022 예외는 "속도 조건으로 거절된 unlock"이면 reason_code를 CMD_REJECTED_SPEED로 한다고 쓴다. F-003의 상태 문장은 "다른 규칙이 결정하면 그 규칙의 값(예: AUTO_LOCK)"을, 다른 규칙이 없으면 DEGRADED_STALE 또는 SENSOR_FAULT를 쓴다. 자동 잠금이 FAULT에서도 적용되므로(SSR-016) 두 요구가 같은 주기에 모두 성립해 reason_code 값이 충돌한다. OFF에서는 F-003 시험이 IGNITION_OFF를 명시해 상태가 우선하지만, FAULT·DEGRADED에서는 우선순위가 정해지지 않았다. | 상태 억제 명령이 속도 조건으로도 거절되는 경우의 reason_code를 정한다(예: OFF와 같이 상태 reason 우선, 또는 속도 거절 우선). 결정한 규칙을 F-003과 F-022에 같은 문장으로 쓰고, FAULT·DEGRADED에서 속도 3.0 km/h 이상 unlock 시험 케이스를 추가한다. 사용자 판단이 필요하다. |
| U-02 | 중대 | SRS.md 6.3 순위 8 ("해당 규칙 없음 → reason_code NONE"), SAFETY-REQ.md SSR-009·SSR-012, SRS-F-003 (DEGRADED·FAULT 문장) | 6.3 순위 8은 활성 규칙이 없으면 reason_code를 NONE으로 정한다. 그러나 SSR-009는 DEGRADED 전이 시 DEGRADED_STALE을, SSR-012는 FAULT의 첫 주기에 SENSOR_FAULT를 요구하고, F-003은 DEGRADED·FAULT 중 다른 규칙이 없는 주기마다 DEGRADED_STALE·SENSOR_FAULT를 쓴다. 활성 규칙이 없는 주기에 세 요구의 값이 다르다(NONE 대 상태 reason). 설계 DD-F-024는 상태 reason을 쓴다. SRS에서 순위 8의 reason이 상태에 따라 달라진다는 것이 명시되지 않았다. | 6.3 순위 8의 reason_code를 "활성 규칙이 없으면 상태별 값: NORMAL → NONE, DEGRADED → DEGRADED_STALE, FAULT → SENSOR_FAULT, OFF → IGNITION_OFF"로 고쳐 SSR-009·012·F-003과 맞춘다. 6.3 순위 8과 F-003 문장을 같은 표로 둔다. |
| U-03 | 경미 | TRACE.md 헤더 (버전 1.2, 재생성됨) | TRACE 내용이 gen_trace.py 재생성으로 바뀌었는데 버전이 1.2 그대로다. 재생성 시 버전을 올려야 한다. | TRACE를 1.3으로 올리고 "gen_trace.py로 생성, 입력 SRS v1.3·ASSUMPTIONS v1.3·DDS v0.4"를 기록한다. |

### 1.5 SWE.1 판정

**반려.** 중대 2건(U-01, U-02)과 경미 1건(U-03)이 남아 있다. U-01은 상태 억제 명령의 reason 우선순위 결정이라 사용자 판단이 필요하다. U-02는 6.3 순위 8과 SSR의 값을 맞추는 문서 정정이다. U-02는 설계(DD-F-024)가 이미 상태 reason을 쓰므로 SRS 쪽을 맞추는 것이 맞다.

---

## Part 2. Iter-2 설계 6차 재리뷰

### 2.1 H-01 조치 확인

| 항목 | 확인 내용 |
|------|-----------|
| DDS.md DD-F-024 | DEGRADED·FAULT·OFF로 적용되지 않은 운전자 명령의 이유 규칙을 명시 (다른 규칙이 없으면 state 이유, OFF는 IGNITION_OFF) (확인) |
| ARCH.md ARCH-I-006 | 사후 조건은 여전히 "억제된 명령의 reason_code를 설정"만 쓴다. 값은 정의되지 않았다. H-01 조치 보고는 ARCH.md 갱신을 포함했으나 diff에 해당 변경이 없다 (V-01) |

H-01은 DDS 쪽에서만 조치되었다 (부분 조치). ARCH-I-006 정정이 빠졌다.

### 2.2 교차 점검

- **DD-F-024 ↔ SRS-F-003(v1.3)**: 제외 명령의 상태 reason(DEGRADED_STALE, SENSOR_FAULT, IGNITION_OFF)과 "다른 규칙이 결정하면 그 규칙의 값"을 구현한다. 이는 F-003 상태 문장과 일치한다. 그러나 F-003 첫 문장과 F-022 예외(속도 거절 → CMD_REJECTED_SPEED)와 충돌하는 조합(FAULT·DEGRADED에서 3.0 km/h 이상 unlock, 자동 잠금 결정)은 설계가 한쪽(상태 문장)을 택했다. 이 선택은 SRS에서 아직 결정되지 않았다 (U-01, 설계 V-02).
- **DD-F-024 ↔ 6.3 순위 8(NONE)**: 설계는 활성 규칙이 없는 주기마다 상태 reason을 쓴다. 6.3 순위 8과 맞지 않으며 SSR-009·012(전이 시점, 첫 주기)보다 넓다 (U-02, 설계 V-02에 포함).
- **DD-F-029 ↔ SRS-F-022 예외**: DD-F-029는 상태로 제외된 명령을 덮어쓰기에서 뺀다. 그래서 FAULT·DEGRADED에서 속도 거절 unlock은 자동 잠금이 결정하면 AUTO_LOCK이 된다. F-022 예외의 "속도 거절 unlock" 조건과 맞지 않을 수 있다 (V-02).
- **ARCH-I-006 ↔ DDS.md DD-F-021·023·024**: ARCH-I-006의 사후 조건은 억제된 명령의 reason_code 값을 정하지 않는다. DDS는 값을 정한다. 인터페이스 사양이 설계 세부를 따라가지 못한다 (V-01).
- **ARCH-D-11, D-12, D-13 ↔ DDS-iter2**: 변경 없음, 이전 판정 유지.
- **DDS-iter2 ↔ DDS.md**: 비트 순서, 호출 관계 변경 없음.
- **TRACE 1.2 ↔ DDS.md 0.4**: DD-F-024·029 배정 반영. 재생성 버전은 U-03.

### 2.3 체크리스트 결과 (설계)

#### ARC-01~13

| ID | 판정 | 근거 |
|----|------|------|
| ARC-01~02 | Pass | 변경 없음 |
| ARC-03 | **Fail** (V-01, 경미) | ARCH-I-006 사후 조건이 억제된 명령의 reason 값을 정하지 않는다. 설계(DD-F-024)와 불일치 |
| ARC-04~12 | Pass | 변경 없음 |
| ARC-13 | Pass | A-22(204) 근거, 상태 reason 근거는 DD-F-024 본문에 기록됨 |

#### DDS-01~12

| ID | 판정 | 근거 |
|----|------|------|
| DDS-01~10 | Pass | 변경 없음 |
| DDS-11 | **Fail** (V-02, 중대) | 상태 억제 명령의 reason이 속도 거절과 겹칠 때(FAULT·DEGRADED, 3.0 km/h 이상, 자동 잠금 결정) 설계는 AUTO_LOCK을 쓰고 SRS-F-022의 CMD_REJECTED_SPEED 예외와 다르다. 활성 규칙이 없을 때 설계의 상태 reason은 6.3 순위 8과 다르다. 두 경우 모두 SRS 결정(U-01, U-02)에 따라 확정되어야 한다 |
| DDS-12 | Pass | DD-F-024 → SRS-F-003, DD-F-029 → SRS-F-003, SRS-F-022 배정 확인. 값의 확정은 V-02 |

#### COM-01~04

| ID | 판정 | 근거 |
|----|------|------|
| COM-01 | Pass | ARCH-iter2 0.5, DDS-iter2 0.5, ARCH.md 0.4, DDS.md 0.4 |
| COM-02 | Pass | 커밋 44ab31a 이후 변경 없음 (설계 문서 기준) |
| COM-03 | **Fail** (H-01 부분 조치) | H-01의 ARCH-I-006 정정이 빠졌다 (V-01) |
| COM-04 | **Fail** (V-01) | ARCH-I-006과 DDS.md DD-F-024 불일치 |

### 2.4 결함 목록 (설계)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| V-01 | 경미 | ARCH.md ARCH-I-006 (사후 조건 "억제된 명령의 reason_code를 설정") | 인터페이스 사양에 억제 명령의 reason 값 규칙이 없다. DDS.md DD-F-024가 규칙을 정한다. H-01 조치 보고는 ARCH.md 정정을 포함했으나 반영되지 않았다. | ARCH-I-006 사후 조건에 "DEGRADED·FAULT·OFF로 적용되지 않은 운전자 명령의 reason_code는 DD-F-024 규칙(상태 값, 다른 규칙이 결정하면 그 규칙 값)"을 쓰고 ARCH.md 버전을 올린다. |
| V-02 | 중대 | DDS.md DD-F-024, DD-F-029; DDS-iter2 (추적 DD-F-029 ↔ SRS-F-022); SRS 6.3 순위 8·F-003·F-022 (U-01, U-02) | 설계는 SRS가 결정하지 않은 두 값을 구현한다. (a) FAULT·DEGRADED에서 3.0 km/h 이상 unlock이 자동 잠금에 의해 결정되는 경우 AUTO_LOCK(F-003 상태 문장을 따름), 그러나 F-022 예외는 CMD_REJECTED_SPEED를 요구한다. (b) 활성 규칙이 없는 주기마다 DEGRADED_STALE·SENSOR_FAULT(DD-F-024), 그러나 6.3 순위 8은 NONE이다. 두 결정이 SRS에서 확정되기 전에는 설계 값의 추적이 성립하지 않는다. | U-01, U-02를 SRS에서 결정한 뒤 DD-F-024·DD-F-029를 그 결정에 맞춘다. 결정 전까지 설계 테스트 케이스(UT)를 확정하지 않는다. |

### 2.5 설계 판정

**반려.** V-02(중대)는 SRS U-01, U-02 결정에 종속된다. V-01(경미)은 ARCH.md 문서 정정으로 해결된다. H-01은 부분 조치다.

---

## 3. 종합

| 항목 | 판정 | 결함 |
|------|------|------|
| (1) SWE.1 재리뷰 (SRS v1.3, ASSUMPTIONS v1.3, SAFETY-REQ, TRACE) | **반려** | 중대 2 (U-01, U-02), 경미 1 (U-03) |
| (2) 설계 6차 (DDS.md v0.4, ARCH.md v0.4, ARCH-iter2 v0.5, DDS-iter2 v0.5) | **반려** | 중대 1 (V-02), 경미 1 (V-01) |

- 이전 결함 조치(T-01~T-06): 6건 조치 완료. T-01은 U-01·U-02로 이어졌다. H-01은 DDS 쪽만 조치되었다 (V-01).
- 사용자 판단 필요: U-01 (상태 억제 명령이 속도 거절과 겹칠 때 reason_code 우선순위). U-02는 SRS 6.3 순위 8을 상태 값으로 고치는 문서 정정이며 설계 DD-F-024와 일치한다.
- 재리뷰 시 U-01~U-03, V-01, V-02, H-01의 조치 상태를 표로 확인하고, SRS F-003·F-022·6.3, SAFETY-REQ SSR-009·SSR-012, ARCH-I-006, DDS.md DD-F-024·DD-F-029, TRACE를 함께 교차 점검한다.
