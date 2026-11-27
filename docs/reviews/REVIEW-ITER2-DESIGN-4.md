# 리뷰 보고서: (1) SWE.1 재리뷰 v1.1, (2) Iter-2 설계 4차 재리뷰

- 대상 (커밋 1cd799b, `feature/iter-0-requirements`):
  - (1) SWE.1 재리뷰: `docs/requirements/SRS.md` v1.1, `docs/requirements/ASSUMPTIONS.md` v1.1, `docs/traceability/TRACE.md` (v1.1 Iter-1, 재생성 여부 판단)
  - (2) 설계 4차: `docs/architecture/ARCH-iter2.md` v0.4, `docs/detailed-design/DDS-iter2.md` v0.4, `docs/architecture/ARCH.md` v0.3, `docs/detailed-design/DDS.md` v0.3
- 기준선 대조: v1.0 승인 커밋 69da6db (SRS, ASSUMPTIONS, TRACE 비교). 변경 열 단위 비교 수행.
- 이전 리뷰: REVIEW-ITER2-DESIGN-1~3.md (설계), SWE.1 승인 기록(v1.0, 10차)
- 일자: 2026-11-27
- 판정: (1) SWE.1 재리뷰 **반려**, (2) 설계 4차 **반려** (G-08 중대. TRACE 결함은 (1)과 공유)

---

## Part 1. SWE.1 재리뷰 (SRS v1.1 변경분)

### 1.1 변경 범위 확인

v1.0(69da6db) 대비 요구 행의 내용 변경은 정확히 SRS-F-003, SRS-F-022, SRS-F-030 세 건이다 (열 단위 비교로 확인). 나머지 32행은 내용 동일이며 상태 열만 바뀌었다. 변경 이력 표(SRS §변경 이력, 1.1 행)가 세 건을 기록한다. 변경 3행의 상태는 "초안 (재리뷰 대기)"다 (확인).

### 1.2 체크리스트 결과 (REQ-01~12, COM)

| ID | 항목 | 판정 | 근거 |
|----|------|------|------|
| REQ-01 | 영구·고유 ID | Pass | ID 35건 유지, 신규 ID 없음. 폐기 ID 재사용 없음 |
| REQ-02 | 기능/비기능 분류 | Pass | 변경 3건 모두 기능(F) 유지 |
| REQ-03 | 안전 요구 분리 | Pass | SRS 본문에 SSR 본문 없음 |
| REQ-04 | ISO 25010 분류 | Pass | Q, L 분류 변경 없음 |
| REQ-05 | 한 요구 한 가지 | Pass | F-003의 "다른 규칙이 결정해도 reason_code는 …" 문장은 F-022 예외와 중복되어 S-05에서 경미로 기록. 단독으로는 한 가지 규칙 |
| REQ-06 | 모호 표현 없이 검증 가능 | Pass | 변경 문장의 용어(결정한 규칙, 순위)는 6.1, 6.3에 정의되어 있음. 단 "적용하지 않은 명령"의 범위는 S-03에서 중대로 기록 |
| REQ-07 | 검증 방법과 합격 기준 | **Fail** (S-04) | F-022 예외(priority_reason 유지)를 확인하는 시험 기준이 없음. F-003의 새 문장도 시험되지 않음 |
| REQ-08 | 충돌·중복 없음 | **Fail** (S-03, S-05, S-06) | F-003(적용하지 않은 명령은 CMD_REJECTED_SPEED)과 F-031(OFF 명령은 IGNITION_OFF)이 같은 명령에 서로 다른 reason을 요구함. F-003과 F-022가 같은 예외를 중복 기술. F-002(CMD_APPLIED 기록)와 F-022(상위 규칙이 결정한 reason 유지)의 관계가 불명확 |
| REQ-09 | "무엇"을 기술 | Pass | 구현 방법 지정 없음 |
| REQ-10 | 상향 추적과 내용 일치 | Pass | F-022 예외는 A-19(사용자 선택, 승인 기록 있음)로 OEM-IF-006 매핑 요구와의 편차가 기록됨 |
| REQ-11 | 하향 추적 단절 없음 | **Fail** (S-07) | TRACE가 DD-F-029(F-003, F-022 배정)를 반영하지 않고, Iter-2 DDS(DD-F-040~070)와의 하향 연결이 비어 있음 |
| REQ-12 | 근거·우선순위·상태 | Pass | 근거, 우선순위, 상태 열 존재 |
| COM-01 | 산출물 ID·버전·상태 | **Fail** (S-01, S-02) | ASSUMPTIONS 헤더 "리뷰 승인 (aspice-reviewer 10차)"은 v1.1에 대해 사실이 아님. SRS 상태 열에 정의되지 않은 값 "리뷰됨" 30여 행 |
| COM-02 | 형상 관리 | Pass | 커밋 1cd799b에 포함, docs 변경 없음 |
| COM-03 | 이전 리뷰 결함 조치 | Pass | v1.0 승인 이후 G-01(SRS v1.1 상향), 변경 이력 기록, 변경 3행 재리뷰 대기 표시 확인 |
| COM-04 | 상·하위 ID·용어 일관성 | **Fail** (S-07) | TRACE와 DDS-iter2 및 DDS.md(DD-F-029) 불일치 |

요약 (REQ·COM 16항목): Pass 11 / Fail 5 / N/A 0

- Pass: REQ-01, 02, 03, 04, 05, 06, 09, 10, 12, COM-02, COM-03
- Fail: REQ-07 (S-04), REQ-08 (S-03, S-05, S-06), REQ-11 (S-07), COM-01 (S-01, S-02), COM-04 (S-07)

### 1.3 결함 목록 (SWE.1)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| S-01 | 중대 | ASSUMPTIONS.md 헤더의 상태 행 "리뷰 승인 (aspice-reviewer 10차, 2026-11-27)" | v1.1 (A-19 승인 반영, A-20·A-21 추가)에 대해 aspice-reviewer 승인을 받은 사실이 없다. 10차 승인 시점에 A-20·A-21은 존재하지 않았다. 리뷰 기록이 허위 상태를 가진다. | 상태를 "초안 (재리뷰 대기, v1.0 승인 이후 A-19 승인 반영·A-20·A-21 추가)"로 고친다. v1.1 재리뷰 후 승인 상태를 갱신한다. |
| S-02 | 경미 | SRS.md 상태 열 (약 30행) | 상태 "리뷰됨"이 상태 체계에 정의되지 않은 값이다. v1.0 승인 시 모든 행은 "초안"이었고 변경 3행만 "초안 (재리뷰 대기)"이다. "리뷰됨"은 v1.0 승인을 뜻하는지 불분명하다. | 상태 체계(요구 상태 정의)를 SRS에 명시하거나, 승인된 행은 "승인(v1.0)"으로 통일한다. |
| S-03 | 중대 | SRS.md F-003 ("적용하지 않은 명령은 reason_code CMD_REJECTED_SPEED"), F-031 ("OFF 중 운전자 명령은 reason_code IGNITION_OFF"), SSR-010/011 (DEGRADED·FAULT 명령 미적용) | 같은 운전자 명령이 OFF 상태이고 속도가 3.0 km/h 이상이면 F-003은 CMD_REJECTED_SPEED, F-031은 IGNITION_OFF를 요구한다. F-003의 "적용하지 않은 명령"이 속도 이외의 사유(상태 억제)를 포함하는지도 불분명하다. 설계(DD-F-029)는 상태 억제 명령을 제외해 해결했지만 SRS에는 이 우선순위가 없다. 변경된 F-003 문장이 "다른 규칙이 결정해도 CMD_REJECTED_SPEED"라고 명시해 이 충돌을 드러냈다. | F-003의 범위를 "유효 차속 조건으로 적용하지 않은 명령"으로 한정하고, 상태 억제(OFF, DEGRADED, FAULT)로 적용되지 않은 명령의 reason은 F-031 또는 SSR-010/011이 정한다고 명시한다. 개정 후 SWE.1 재리뷰. |
| S-04 | 중대 | SRS.md F-022 시험 ("속도 거절 unlock이 있는 경우는 reason_code가 CMD_REJECTED_SPEED"), F-003 시험 ("3.0 km/h 미적용+CMD_REJECTED_SPEED") | 변경된 예외는 priority_reason이 결정 규칙의 값을 유지한다고 요구하지만, 어느 시험도 priority_reason 값(예: AUTO_LOCK)을 확인하지 않는다. F-003의 새 문장(다른 규칙이 결정해도)을 검증하는 조건(자동 잠금 활성 상태)도 시험에 없다. | F-003 시험에 "자동 잠금 활성(3.0 km/h, 2.9 km/h→3.0 km/h로 전환)에서 unlock 거절 시 reason_code CMD_REJECTED_SPEED이고 priority_reason AUTO_LOCK"을 추가하고, F-022 시험에 같은 케이스의 priority_reason 확인을 명시한다. |
| S-05 | 경미 | SRS.md F-003 마지막 문장, F-022 본문 | 같은 예외 규칙을 두 요구에 중복 기술한다. 한쪽을 바꾸면 불일치가 생긴다. | F-003은 거절 기록만 쓰고, 예외 규칙(reason 덮어쓰기와 priority_reason 유지)은 F-022 한 곳에 둔다. |
| S-06 | 경미 | SRS.md F-002 ("reason_code CMD_APPLIED를 기록") 와 F-022·F-028 | F-002는 명령을 적용하면 CMD_APPLIED를 기록한다고 한다. 그러나 명령이 적용되더라도 더 높은 순위(예: 접근위험 LOCK)가 출력을 결정하면 priority_reason은 그 규칙이고 reason_code는 F-022에 따라 같아야 한다. 두 요구의 관계가 명시되지 않았다. | F-002에 "명령의 효과가 출력을 결정할 때만 CMD_APPLIED"를 명시하거나, F-022 예외에 CMD_APPLIED 경우를 추가한다. |
| S-07 | 중대 | TRACE.md §1 (SRS-F-003, SRS-F-022 행과 Iter-2 행), DDS-iter2 §5, DDS.md DD-F-029 | TRACE는 gen_trace.py로 생성된 v1.1(Iter-1)이며 DD-F-029(Arbiter_IsSpeedRejected, F-003·F-022 배정)를 포함하지 않는다. SRS-F-022 행의 상세 설계 열이 비어 있다. Iter-2 요구(F-018~026)는 상세 설계 열이 비어 있는데 DDS-iter2는 DD-F-040~070에 이 요구를 배정한다. 하향 추적이 현재 설계와 맞지 않는다. | gen_trace.py를 현재 DDS(DD-F-029 포함)와 DDS-iter2를 읽도록 실행해 재생성하고, 결과가 DDS-iter2 §5 배정과 일치하는지 확인한다. 재생성 결과를 TRACE 버전 1.2로 기록한다. |
| S-08 | 경미 | TRACE.md §4 ("시스템 테스트 칸이 빈 요구: Iter-2 범위이거나 누락") | 빈 칸의 원인을 "또는"으로 얼버무린다. Iter-2 요구는 범위가 확정되어 있어 누락 여부를 구분할 수 있다. | Iter-2 요구는 "Iter-2 시스템 테스트 예정"으로, 나머지는 실제 누락으로 구분 표기한다. |

### 1.4 TRACE 재생성 판단

재생성이 필요하다. 근거: (a) DD-F-029가 DDS.md에 추가되었는데 TRACE에 없다 (S-07), (b) DDS-iter2의 Iter-2 배정 전체가 TRACE에 없다, (c) TRACE 헤더가 "Iter-1"로 되어 있어 현재 설계 범위와 맞지 않는다. 재생성 대상은 `scripts/docs/gen_trace.py`이며 입력은 SRS, DDS, DDS-iter2, ARCH, ARCH-iter2다. 재생성 후 TRACE 상태를 "gen_trace.py 생성, 설계 4차 리뷰 대상"으로 쓴다.

### 1.5 SWE.1 판정

**반려.** 중대 4건(S-01, S-03, S-04, S-07)과 경미 4건(S-02, S-05, S-06, S-08)이 있다. v1.1 세 요구 변경은 SWE.1 재리뷰 전까지 승인된 기준선으로 쓸 수 없다. S-03과 S-04는 요구 문장 수정과 시험 보강으로 해결된다. 사용자 판단이 필요한 항목은 없다.

---

## Part 2. Iter-2 설계 4차 재리뷰

### 2.1 3차 결함 조치 확인 (G-01~G-07)

| 번호 | 심각도 | 3차 내용 | 조치 상태 | 근거 / 잔여 |
|------|--------|----------|-----------|-------------|
| G-01 | 중대 | SRS v1.0 기준선 변경의 버전·재리뷰 누락 | 조치 완료 (SWE.1 판정 Part 1 참조) | SRS 헤더 1.1, 상태 "v1.1 변경 3건은 재리뷰 대기", 변경 이력 표, 변경 3행 "초안 (재리뷰 대기)" 확인. 단 S-01, S-02의 상태 표기 문제 |
| G-02 | 경미 | ARCH.md I-007 소비자 | 조치 완료 | ARCH.md §4 ARCH-I-007 "C-013 → C-016, 테스트" 확인 |
| G-03 | 경미 | ARCH-I-010 오류 목록 | 조치 완료 | 본문 상한 초과 500 목록(화면, GET /status와 POST /step 직렬화, GET /history 이력 쓰기) 확인 |
| G-04 | 경미 | 초기 응답의 state와 INVALID 충돌 | **조치 완료, 단 새 중대 결함 발생(G-08)** | 평가 전 GET /status는 HTTP 204(ARCH-D-12, DD-F-057)로 바뀌어 SSR-015 충돌은 사라졌다. 그러나 SRS-F-018 요구 위반이 생김 |
| G-05 | 경미 | body_limit 범위 불변식 | 조치 완료 | ARCH-D-13, DD-F-070 Web_SetBodyLimit (1 이상 WEB_BODY_CAPACITY 이하, 범위 밖 ERR_PARAM), Web_Init은 WEB_BODY_CAPACITY로 설정 (DD-F-051) 확인 |
| G-06 | 경미 | 버전 표기 | 조치 완료 | ARCH.md v0.3, ASSUMPTIONS v1.1 확인. SRS는 G-01 |
| G-07 | 경미 | DD-F 번호 정리 | 조치 완료 | DD-F-061(WebPage_Get, DD-M-018), DD-F-070(Web_SetBodyLimit) 배치 확인. DD-F-060에서 062로 연속 |

3차 결함 7건: 조치 완료 7 (G-04는 새 결함 G-08 발생), 부분 0, 미조치 0.

### 2.2 교차 점검

- **ARCH-I-010 ↔ DD-F-052~060 ↔ DD-F-070**: 경로, 메서드, 200·204·400·404·405·500 조건이 일치한다. Web_SetBodyLimit의 사전 조건(Web_Init 완료)과 범위가 ARCH-D-13과 일치한다.
- **ARCH-D-11 ↔ DD-F-064 ↔ DD-F-058**: 하위 호출 오류 불가 불변식과 이력 쓰기 500 조건이 일치한다.
- **ARCH-D-12 ↔ DD-F-057 ↔ SSR-015 ↔ SRS-F-018**: 204 응답은 SSR-015와 충돌하지 않는다. 그러나 SRS-F-018("상태 조회 요청에 … 응답해야 한다")의 요구를 평가 전에는 충족하지 않는다 (G-08).
- **DD-F-029 ↔ SRS-F-003·F-022 (v1.1)**: DD-F-029는 운전자 순위가 state로 제외되지 않은 경우에만 CMD_REJECTED_SPEED로 덮어쓴다. 이 조건은 S-03의 충돌을 설계에서 해소하지만 SRS에는 반영되지 않았다.
- **DDS.md DD-F-029 ↔ TRACE**: TRACE에 없다 (S-07).
- **ARCH.md ↔ ARCH-iter2**: ARCH-I-007 소비자, RawInput_t 이름, 정적 구조가 일치한다.
- **DDS.md 비트 순서 ↔ DD-F-040 ↔ ARCH-iter2 I-008**: 13항목 순서 일치.
- **DDS-iter2 번호 체계**: DD-F-040~070, 061·070 정리 후 연속 (확인).

### 2.3 체크리스트 결과 (설계)

#### ARC-01~13

| ID | 판정 | 근거 |
|----|------|------|
| ARC-01 | Pass | C-014, C-015, C-016 책임 한 문장 |
| ARC-02 | Pass | 요구 IF 정정 확인 |
| ARC-03 | Pass | I-010 오류 목록 보완 (G-03 확인) |
| ARC-04 | Pass | C-016만 호출자 |
| ARC-05 | Pass | 순환 없음 |
| ARC-06 | Pass | 응집도 |
| ARC-07 | Pass | 공유 가변 데이터 없음 |
| ARC-08 | Pass | 캡슐화 |
| ARC-09 | Pass | SOLID 근거 |
| ARC-10 | Pass | 정적·동적 구조 |
| ARC-11 | Pass | 비기능 근거 (Q-002, Q-003, L-006, body_limit) |
| ARC-12 | Pass | 요구 배정, 폐기 ID 없음 |
| ARC-13 | **Fail** (G-08, 중대) | ARCH-D-12의 204 결정은 SSR-015와의 충돌을 근거로 하지만 SRS-F-018과의 충돌을 다루지 않는다. 요구 편차가 가정으로 기록되지 않았다 |

#### DDS-01~12

| ID | 판정 | 근거 |
|----|------|------|
| DDS-01 | Pass | 모듈 분해 (DD-M-014~019) |
| DDS-02 | Pass | 인터페이스 기준 |
| DDS-03 | Pass | DD-M-014~016, 018, 019 인터페이스 정의 |
| DDS-04 | Pass | 시그니처 일치 (Web_SetBodyLimit 추가 포함) |
| DDS-05 | Pass | 공개 인터페이스만 사용 |
| DDS-06 | Pass | 반환 규칙 |
| DDS-07 | Pass | 단일 반환 명시 |
| DDS-08 | Pass | 모듈 3분할 |
| DDS-09 | Pass | 상수 정의, 리터럴 잔여 없음 |
| DDS-10 | Pass | 크기 계획 |
| DDS-11 | Pass | 본문 상한 불변식 (G-05), 평가 전 응답 정의 (G-04) |
| DDS-12 | Pass | DD-F-029와 SRS 배정 확인. TRACE 갱신은 S-07 |

#### COM-01~04

| ID | 판정 | 근거 |
|----|------|------|
| COM-01 | Pass | ARCH-iter2 0.4, DDS-iter2 0.4, ARCH.md 0.3, DDS.md 0.3 |
| COM-02 | Pass | 커밋 1cd799b |
| COM-03 | Pass | G-01~G-07 조치 확인 (G-04 새 결함은 G-08로 기록) |
| COM-04 | **Fail** (S-07) | TRACE가 DD-F-029, DDS-iter2 배정과 맞지 않음 |

### 2.4 결함 목록 (설계)

| 번호 | 심각도 | 위치 | 설명 | 수정 제안 |
|------|--------|------|------|-----------|
| G-08 | 중대 | ARCH-iter2 ARCH-D-12, ARCH-I-010 (GET /status 204), DDS-iter2 DD-F-057, SRS-F-018 | SRS-F-018은 "상태 조회 요청에 좌, 우 출력과 표시 항목을 응답해야 한다"고 요구한다. 평가 전 GET /status가 본문 없는 204를 주므로 이 요구를 충족하지 않는다. 이 편차는 가정으로 기록되지 않았고 사용자 승인도 없다. ASSUMPTIONS에 해당 항목이 없다. | (a) 편차로 기록(A-22: 평가 전에는 출력 값을 표시할 평가 결과가 없어 204 응답, 최초 출력 RELEASE 표시 없음)하고 사용자에게 승인을 요청한다. 또는 (b) SRS-F-018에 "평가 전에는 초기 출력 RELEASE와 표시 값 없음을 응답한다"를 명시하는 SRS 개정을 SWE.1 재리뷰로 받는다. 사용자 판단이 필요하다. |

### 2.5 설계 판정

**반려.** G-08 중대 1건과 TRACE 불일치(S-07, 공유 결함) 때문이다. 설계 문서 자체의 G-01~G-07 조치는 확인되었다. G-08은 사용자 판단(편차 승인 또는 SRS 개정)이 필요하다.

---

## 3. 종합 요약

| 항목 | 판정 | 결함 |
|------|------|------|
| (1) SWE.1 재리뷰 (SRS v1.1, ASSUMPTIONS v1.1, TRACE) | **반려** | 중대 4 (S-01, S-03, S-04, S-07), 경미 4 (S-02, S-05, S-06, S-08) |
| (2) 설계 4차 (ARCH-iter2 v0.4, DDS-iter2 v0.4, ARCH.md v0.3, DDS.md v0.3) | **반려** | 중대 1 (G-08) |

- G-01~G-07 조치: 7건 확인 (G-04는 새 결함 G-08 발생).
- TRACE 재생성: 필요 (S-07). gen_trace.py 재실행 후 DD-F-029, DDS-iter2 배정이 반영되었는지 재확인한다.
- 사용자 판단 필요: G-08 (204 편차 승인 또는 SRS-F-018 개정), S-03 (F-003 범위 한정은 설계 결정과 일치하므로 사용자 확인 권장).
- 재리뷰 시 S-01~S-08, G-08, TRACE 재생성 결과를 표로 확인한다.
