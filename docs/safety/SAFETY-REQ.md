# 안전 요구사항 (OEM 할당 ASIL B 입력에서 도출)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-SAF-001 |
| 버전 | 1.2 (SSR-007 시험 기준 보강, 헤더 정정) |
| 상태 | v1.0 aspice-reviewer 승인(10차, 2026-11-27). v1.1, v1.2의 SSR-007 변경은 재리뷰 대기, 나머지 SSR은 승인(v1.0) |
| 작성 | Claude Sonnet 5.5, 2026-11-27, 요청자 KYLee |

- 기능 안전 요구사항은 SRS(DOC-SRS-001)에 쓰지 않고 이 문서에서 관리한다. SRS는 `SSR-nnn` ID만 참조하며, 안전 규칙의 효과·조건·상태 복귀는 이 문서에만 서술한다.
- HARA와 ASIL 결정 근거는 범위 밖이며 OEM이 할당한 분류를 입력으로 사용한다. 이 문서는 ISO 26262 준수를 주장하지 않는다.
- 용어, 열거형, 우선순위 순위는 SRS 6장을 따른다. 가정(A-nn)은 ASSUMPTIONS.md를 따른다.
- 이 문서의 모든 항목은 후속 단계에서 단위 MC/DC 100%와 요구 기반 시험을 갖춘다.
- 우선순위(순위 번호)는 SRS 6.3을 따르며 이 문서에서 복제하지 않는다.

## 1. OEM 안전 입력

| 안전 입력 ID | OEM 분류 | OEM 수용기준 요지 |
|--------------|----------|--------------------|
| OEM-SR-001 | ASIL B 입력 | 충돌 CONFIRMED 시 다른 명령보다 우선해 좌, 우 해제, 300 ms 이내, 이후 주기에도 유지 |
| OEM-SR-002 | ASIL B 입력 | 후측방 접근위험 TRUE 도어는 LOCK, 해당 도어 해제 요청에도 LOCK 유지, 원인 기록 |
| OEM-SR-003 | ASIL B 입력 | 필수 입력이 200 ms 초과 미갱신이면 100 ms 이내 DEGRADED, 형식·범위 오류는 평가 전 거절 |
| OEM-SR-004 | ASIL B 입력 | sensor_fault TRUE 첫 평가주기에 출력 유지, FAULT 상태와 경고코드 제공 |

## 2. 안전 요구사항 (SSR)

| ID | 제목 | 기술 (shall) | 검증 방법 / 합격 기준 | 상위 추적 | 상태 |
|----|------|--------------|------------------------|-----------|------|
| SSR-001 | 충돌 긴급 해제 | SW는 crash_status가 유효한 CONFIRMED인 동안 매 평가주기에 좌, 우 출력을 RELEASE로 하고 reason_code CRASH_CONFIRMED를 기록해야 한다. 이 규칙은 다른 모든 명령과 규칙보다 우선한다. | 시험: CONFIRMED 주입 후 유지되는 모든 주기에서 두 출력 RELEASE, 동시 LOCK 명령·접근위험·ISOFIX·자동잠금 입력이 있어도 RELEASE | OEM-SR-001, OEM-IF-002 | 승인(v1.0) |
| SSR-002 | 충돌 해제 시간 | SW는 crash_status가 CONFIRMED가 된 시점부터 300 ms(30 평가주기) 이내에 좌, 우 출력을 RELEASE로 해야 한다. | 시험: 논리 시간 기준 30주기 이내 RELEASE, DEGRADED, FAULT 상태에서도 동일 | OEM-SR-001 | 승인(v1.0) |
| SSR-003 | 충돌 PENDING | SW는 crash_status가 PENDING이어도 출력을 변경하는 규칙을 만들지 않아야 한다. 출력은 다른 규칙이 결정하고 다른 규칙이 없으면 직전 출력을 유지하며, 다른 reason_code가 없을 때 reason_code CRASH_PENDING을 기록한다. | 시험: PENDING 단독 주입 시 출력 불변+CRASH_PENDING, PENDING과 운전자 명령 동시 주입 시 명령 규칙대로 출력되고 reason_code는 CMD_APPLIED | OEM-SR-001, OEM-IF-002 | 승인(v1.0) |
| SSR-004 | 접근위험 LOCK | SW는 접근위험이 유효한 TRUE인 쪽 도어 출력을 LOCK으로 하고 reason_code APPROACH_RISK_LOCK을 기록해야 한다. 단 SRS-F-007, SRS-F-030에 따라 override가 성립한 도어는 제외한다. | 시험: rear_left_approach_risk=TRUE → 좌 LOCK(우 불변), rear_right 대칭, override 성립 도어는 RELEASE 유지 | OEM-SR-002, OEM-IF-003 | 승인(v1.0) |
| SSR-005 | 해제 억제 | SW는 접근위험이 TRUE인 도어에 대한 unlock 요청을 적용하지 않고 reason_code APPROACH_RISK_SUPPRESSED를 기록해야 한다. SRS-F-007 override 조건을 만족한 요청은 제외한다. | 시험: unlock 요청 후 해당 도어 LOCK 유지+APPROACH_RISK_SUPPRESSED | OEM-SR-002, OEM-IF-003 | 승인(v1.0) |
| SSR-006 | 좌우 독립 | SW는 좌, 우 도어의 접근위험 판정, 억제, 출력을 서로 독립적으로 처리해야 한다. | 시험: 좌 TRUE/우 FALSE에서 우 unlock은 적용(차속 < 3 km/h), 좌 unlock은 억제 | OEM-SR-002, OEM-IF-003 | 승인(v1.0) |
| SSR-007 | 입력 검증 거절 | SW는 OEM-IF-001, 002, 003, 004, 007, 008, 009 입력의 누락, 형식 오류, 범위 오류, 값 집합 밖 값을 평가 전에 거절하고 해당 입력을 INVALID로 처리해야 한다. INVALID 입력은 그 입력을 조건으로 하는 규칙에 사용하지 않는다. 거절된 IF-004 명령은 무시하고, state가 NORMAL이면 reason_code INPUT_INVALID를 기록한다 (그 외 state에서는 SSR-009, SSR-012, SRS-F-031의 상태 이유가 우선한다. 사용자 선택 2026-11-27, A-25, SRS-F-032). gear가 INVALID이어도 필수 입력 판정과 DEGRADED에는 영향이 없고 input_validity에만 표시한다. | 시험: 속도 -0.1, 300.1, 문자열, 누락, 미등록 enum, gear 'X' 각각이 거절되고 규칙 조건에 사용되지 않음, gear 'X' 단독은 DEGRADED를 일으키지 않음. 형식 오류 IF-004 명령(예: side 범위 밖)은 무시되고 state NORMAL에서 reason_code INPUT_INVALID(priority_reason은 결정 규칙의 값, 없으면 NONE), state OFF, DEGRADED, FAULT(FAULT 첫 주기 포함)에서는 각각 IGNITION_OFF, DEGRADED_STALE, SENSOR_FAULT(규칙이 결정하면 그 규칙의 reason) | OEM-SR-003, OEM-IF-001, OEM-IF-002, OEM-IF-003, OEM-IF-004, OEM-IF-007, OEM-IF-008, OEM-IF-009 | 초안 (재리뷰 대기) |
| SSR-008 | stale 판정 | SW는 필수 입력의 fresh 조건(SRS 6.1)을 벗어나면, 즉 시간 차가 200 ms를 초과하면 그 입력을 stale로 판정해야 한다. | 시험(주기 단위): 20주기(200 ms) 경과 fresh, 21주기(210 ms) 경과 stale | OEM-SR-003 | 승인(v1.0) |
| SSR-009 | DEGRADED 전이 시간 | SW는 필수 입력이 stale 또는 INVALID가 된 시점부터 100 ms(10 평가주기) 이내에 state를 DEGRADED로 하고 reason_code DEGRADED_STALE을 기록해야 한다. SSR-015에 따라 state가 OFF 또는 FAULT로 결정되는 경우에는 DEGRADED로 전이하지 않는다. | 시험: stale 판정 주기부터 10주기 이내 state=DEGRADED, DEGRADED_STALE, ignition_on=FALSE 또는 sensor_fault=TRUE 동시 시 OFF 또는 FAULT | OEM-SR-003 | 승인(v1.0) |
| SSR-010 | DEGRADED 명령 미적용 | SW는 state가 DEGRADED인 동안 운전자 명령(OEM-IF-004)을 적용하지 않고, 어떤 규칙도 출력을 정하지 않으면 직전 출력을 유지해야 한다. | 시험: DEGRADED 중 운전자 명령 후 출력 불변, 규칙 입력(CONFIRMED, 긴급, 접근위험, ISOFIX, 자동잠금)은 SSR-013, SSR-016에 따라 적용 | OEM-SR-003 | 승인(v1.0) |
| SSR-011 | sensor_fault 명령 미적용 | SW는 sensor_fault가 유효한 TRUE인 동안 매 평가주기에 새 명령(OEM-IF-004)을 적용하지 않고, 어떤 규칙도 출력을 정하지 않으면 직전 출력을 유지해야 한다. | 시험: sensor_fault TRUE 첫 주기와 이후 주기에서 운전자 명령 후 출력 불변, 규칙 입력이 없으면 출력 불변 | OEM-SR-004, OEM-IF-009 | 승인(v1.0) |
| SSR-012 | FAULT 상태 표시 | SW는 sensor_fault가 TRUE인 첫 평가주기에 state를 FAULT로 하고 경고코드 W-001과 reason_code SENSOR_FAULT를 제공해야 한다. | 시험: 첫 주기에 FAULT, W-001, SENSOR_FAULT | OEM-SR-004, OEM-IF-009 | 승인(v1.0) |
| SSR-013 | FAULT/DEGRADED 예외 | SW는 FAULT 또는 DEGRADED 상태에서도 유효한 crash CONFIRMED(SSR-001), 긴급 강제 해제(SRS-F-014), ignition off(SRS-F-016) 규칙을 적용해야 한다. | 시험: sensor_fault TRUE 및 필수 입력 stale 상태 각각에서 세 규칙이 적용됨 | OEM-SR-004, OEM-SR-001, OEM-SR-003 | 승인(v1.0) |
| SSR-016 | FAULT/DEGRADED 중 LOCK 규칙 | SW는 FAULT 또는 DEGRADED 상태에서도 매 평가주기에 해당 입력이 유효하면 접근위험 LOCK(SSR-004), ISOFIX 강제 잠금(SRS-F-015), 자동 잠금(SRS-F-004)을 적용해야 한다. | 시험: sensor_fault TRUE 및 필수 입력 stale 상태 각각에서 세 규칙이 적용됨 | OEM-SR-004, OEM-SR-003, OEM-SR-002, OEM-FR-002, OEM-FR-006 | 승인(v1.0) |
| SSR-014 | FAULT/DEGRADED 복귀 | SW는 sensor_fault가 유효한 FALSE가 된 다음 평가주기에 FAULT 조건을 해제하고, 모든 필수 입력이 유효해진 다음 평가주기에 DEGRADED 조건을 해제해야 한다. 해제 후 state는 SSR-015에 따라 결정한다. | 시험: 조건 해소 다음 주기에 해당 조건이 사라지고 남은 조건에 따라 state가 결정됨 | OEM-SR-003, OEM-SR-004 | 승인(v1.0) |
| SSR-015 | state 결정 | SW는 state를 ignition_on이 유효한 FALSE이면 OFF, 아니면 sensor_fault가 유효한 TRUE이면 FAULT, 아니면 필수 입력이 하나라도 stale 또는 INVALID이면 DEGRADED, 아니면 NORMAL로 정해야 한다. | 시험: 조건 8가지 조합(ignition, fault, stale 각 참/거짓)에서 기대 state 일치 | OEM-SR-003, OEM-SR-004 | 승인(v1.0) |

## 변경 이력

| 버전 | 변경 | 사유 | 승인 |
|------|------|------|------|
| 1.0 | 최초 승인본 | Iter-0 | aspice-reviewer 10차 승인 |
| 1.1 | SSR-007: INPUT_INVALID 기록을 state NORMAL로 한정 | 상태 이유 우선, 사용자 선택(2026-11-27, A-25) | 1차 재리뷰에서 반려 |
| 1.2 | SSR-007: 변경된 범위의 시험 기준(NORMAL, OFF, DEGRADED, FAULT, FAULT 첫 주기) 보강 | 재리뷰 지적(Z-01) 조치 | 재리뷰 대기 |

## 3. 안전 문서 후속 계획

- 고장 모드 분석과 안전 메커니즘은 아키텍처 설계 단계에서 이 문서의 후속 개정으로 추가한다. 현재 SSR은 OEM 수용기준을 SW 요구로 도출한 것이다.
