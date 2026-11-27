# SRS — 전자식 차일드락 제어 SW 요구사항 명세서

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-SRS-001 |
| 버전 | 0.2 (Iter-0, 1차 리뷰 반려 조치본) |
| 상태 | 초안 (재리뷰 대기) |
| 작성 | Claude Sonnet 5.5, 2026-11-27, 요청자 KYLee |
| 입력 | OEM-SWR-001 Rev 1.0 (OEM-A 공급자 입력, 교육용 가상 사양) |

- 기능 안전 요구사항은 이 문서에 쓰지 않는다. `docs/safety/SAFETY-REQ.md`(DOC-SAF-001)에서 관리하고, 이 문서는 ID(`SSR-nnn`)만 참조한다 (2.절 목록).
- 상위 추적 ID: `OEM-SR-*`(안전 입력), `OEM-FR-*`, `OEM-NFR-*`, `OEM-IF-*`. OEM의 `SWR-nnn`은 OEM 추적표의 참조 번호이며 `OEM SWR` 열로 교차 참조한다. SWR-019는 결번이다.
- 가정(A-nn)은 `ASSUMPTIONS.md`(DOC-ASM-001)를 참조한다.
- 용어, 열거형, 우선순위 표, 전이 조건은 6장에서 정의하며 6장은 요구사항의 일부다.

## 1. 기능 요구사항 (SRS-F)

| ID | 제목 | 기술 (shall) | 근거 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | OEM SWR | 상태 |
|----|------|--------------|------|-----------------------|----------|-----------|---------|------|
| SRS-F-001 | 운전자 명령 수신 | SW는 physical_button, avn, voice, mobile_app 네 source로 수신한 side(left/right/all)와 action(lock/unlock) 명령을 수신하고, 요청 시각으로 VehicleSnapshot.timestamp_s를 사용해야 한다. | 운전자 조작 경로, 요청 시각 계약 | 시험: 4 source × 3 side × 2 action = 24개 조합이 수신되고 요청 시각이 해당 평가주기의 VehicleSnapshot.timestamp_s와 같다 | 필수 | OEM-FR-001, OEM-IF-004 | SWR-001 | 초안 |
| SRS-F-002 | 선택 도어 출력 적용 | SW는 적용이 허용된 운전자 명령을 side가 지정한 도어 출력에만 적용해야 한다. | 좌우 독립 제어 | 시험: 차속 0 km/h, 정상 입력에서 source 4종 각각에 대해 left→좌만, right→우만, all→양쪽이 LOCK/RELEASE로 변경되고 선택되지 않은 출력은 변화 없음 | 필수 | OEM-FR-001, OEM-IF-005 | SWR-002 | 초안 |
| SRS-F-003 | 일반 해제 허용 조건 | SW는 운전자 unlock 명령을 유효 차속이 SRS-L-001 미만일 때만 적용해야 한다. 적용하지 않은 명령은 reason_code CMD_REJECTED_SPEED로 기록해야 한다. | 주행 중 의도치 않은 해제 방지 (A-06) | 시험: 2.9 km/h 적용, 3.0 km/h 미적용+CMD_REJECTED_SPEED | 필수 | OEM-FR-001, OEM-IF-001 | SWR-004 | 초안 |
| SRS-F-004 | 속도 자동 잠금 | SW는 유효 차속이 SRS-L-001 이상인 첫 평가주기에 좌, 우 출력을 LOCK으로 해야 한다. 히스테리시스는 없다(A-08). | 이동 시작 시 자동 보호 | 시험: 2.9 km/h 미동작, 3.0 km/h 첫 평가주기에 두 출력 LOCK+AUTO_LOCK | 필수 | OEM-FR-002, OEM-IF-001 | SWR-003 | 초안 |
| SRS-F-005 | 해제 override | SW는 접근위험으로 해제가 억제(SSR-005)된 도어에 대해 억제 후 SRS-L-002 이하 안에 같은 도어의 같은 unlock 명령이 재입력되면 해당 도어를 RELEASE하고 reason_code OVERRIDE를 생성해야 한다. override는 SRS-F-007(ISOFIX 강제 잠금)에는 적용하지 않는다. | 운전자의 명시적 의도 (A-03) | 시험: 10.0 s 이하 재입력 → RELEASE+OVERRIDE, 10.1 s 재입력 → 미적용(SSR-005 유지), ISOFIX TRUE 도어는 재입력해도 LOCK | 필수 | OEM-FR-003 | SWR-006 | 초안 |
| SRS-F-006 | 긴급 강제 해제 | SW는 fire_detected, overtemperature_detected, adult_present 중 하나가 유효한 TRUE이면 다음 평가주기에 좌, 우 출력을 RELEASE로 하고 입력별 reason_code(FIRE, OVERTEMP, ADULT_PRESENT)를 기록해야 한다. 이 규칙은 FAULT, DEGRADED 상태에서도 적용한다(SSR-013). | 화재, 과온, 성인 탑승 시 탈출 확보 | 시험: 3개 입력 단독 주입 시 다음 주기에 RELEASE+해당 이유코드, sensor_fault TRUE 상태에서도 동일 | 필수 | OEM-FR-005, OEM-IF-007 | SWR-017 | 초안 |
| SRS-F-007 | ISOFIX 강제 잠금 | SW는 isofix_left 또는 isofix_right가 유효한 TRUE이면 다음 평가주기에 해당 쪽 출력을 LOCK으로 하고 반대쪽 출력은 유지하며 reason_code ISOFIX_LOCK을 기록해야 한다. | 어린이 카시트 장착 도어 보호 | 시험: 한쪽만 TRUE → 해당 출력만 LOCK+ISOFIX_LOCK, 반대쪽 불변 | 필수 | OEM-FR-006, OEM-IF-008 | SWR-018 | 초안 |
| SRS-F-008 | ignition off 해제 | SW는 ignition_on이 유효한 FALSE인 첫 평가주기에 좌, 우 출력을 RELEASE로 하고 state를 OFF, reason_code를 IGNITION_OFF로 해야 한다. | 시동 꺼짐 시 초기 상태 (A-14) | 시험: ignition_on=FALSE 첫 주기에 RELEASE, state=OFF, reason_code=IGNITION_OFF (ISOFIX TRUE 동시 입력에서도 동일) | 필수 | OEM-FR-007, OEM-IF-009 | SWR-020 | 초안 |
| SRS-F-009 | OFF 복귀 | SW는 ignition_on이 유효한 TRUE로 바뀐 다음 평가주기에 state OFF를 해제하고 6.3의 우선순위에 따라 평가해야 한다. | OFF 복귀 조건 정의 | 시험: FALSE→TRUE 후 다음 주기에 state가 NORMAL(다른 이상 없음)이고 출력은 직전(RELEASE)에서 규칙대로 갱신 | 필수 | OEM-FR-007, OEM-IF-009 | SWR-020 | 초안 |
| SRS-F-010 | 우선순위 중재 | SW는 둘 이상의 규칙이 서로 다른 출력을 요구하면 6.3 우선순위 표에서 순위가 높은 규칙을 적용하고 그 규칙의 reason_code를 priority_reason으로 기록해야 한다. | 결정론적 충돌 해소 (A-01) | 시험: 6.3 표의 규칙 중 효과가 서로 다른 모든 쌍(규칙 수에 따른 전체 조합)에서 상위 규칙의 효과만 적용되고 priority_reason이 상위 규칙의 reason_code이다 | 필수 | OEM-SR-001, OEM-SR-002, OEM-FR-002, OEM-FR-005, OEM-FR-006, OEM-FR-007 | (파생) | 초안 |
| SRS-F-011 | 상태 조회 응답 | SW는 상태 조회 요청에 좌 출력, 우 출력, state, input_validity, 최근 결정의 reason_code를 응답해야 한다. | 표시 및 진단 | 통합 시험: 응답에 5개 항목이 모두 포함되고 값이 마지막 평가 결과와 같다 | 필수 | OEM-FR-004 | SWR-014 | 초안 |
| SRS-F-012 | 표시 항목 제공 | SW는 표시 인터페이스에 state, priority_reason, reason_code, input_validity를 제공해야 한다. | 표시 계약 | 통합 시험: 4개 항목이 응답에 있고 6.2 열거형 값 집합에 속한다 | 필수 | OEM-FR-004, OEM-IF-006 | SWR-015 | 초안 |
| SRS-F-013 | priority_reason 매핑 | SW는 priority_reason 값을 같은 값의 reason_code로도 제공해야 한다. | OEM-IF-006 매핑 요구 | 통합 시험: 모든 priority_reason 값에서 reason_code가 같은 값 | 필수 | OEM-IF-006 | SWR-015 | 초안 |
| SRS-F-014 | 직렬화 실패 응답 | SW는 표시 인터페이스 응답의 직렬화에 실패하면 HTTP 500으로 응답해야 한다. | OEM-IF-006 오류 처리 | 통합 시험: 직렬화 실패 주입 시 HTTP 상태 500 | 필수 | OEM-IF-006 | SWR-015 | 초안 |
| SRS-F-015 | 이벤트 ID 부여 | SW는 제어 결정마다 이전에 부여한 적이 없는 고유 이벤트 ID를 부여해야 한다. | 결정 추적 | 시험: 1,000건 기록에서 이벤트 ID 중복 0건 | 필수 | OEM-NFR-002 | SWR-010 | 초안 |
| SRS-F-016 | 이벤트 필드 한정 | SW는 이벤트에 이벤트 ID, 논리 시각, 좌 출력, 우 출력, state, reason_code 필드만 기록해야 한다. | 허용 필드 한정 (A-09) | 시험: 모든 이벤트의 필드 집합이 정의된 6개와 정확히 일치 | 필수 | OEM-NFR-002 | SWR-011 | 초안 |
| SRS-F-017 | 결정 기록 조건 | SW는 출력, state, reason_code 중 하나라도 직전 평가주기와 달라진 평가주기를 하나의 결정으로 보고 이벤트를 기록해야 한다. | "결정" 단위 정의 (A-15) | 시험: 변화 없는 100주기에 이벤트 0건, 변화가 있는 주기마다 이벤트 1건 | 필수 | OEM-NFR-002 | SWR-010 | 초안 |
| SRS-F-018 | 결정 이력 보존 | SW는 최근 SRS-L-003 건의 결정 이벤트를 오래된 순서대로 조회할 수 있게 보존해야 한다. | 이력 조회 | 시험: 101건 입력 후 최신 100건만 시간순 조회 | 필수 | OEM-NFR-002 | SWR-012 | 초안 |
| SRS-F-019 | 이력 휘발성 | SW는 결정 이력을 메모리에만 두고 프로세스 재기동 후에는 이력이 남지 않게 해야 한다. | 휘발성 요구 | 시험: 재기동 후 조회 건수 0 | 필수 | OEM-NFR-002 | SWR-012 | 초안 |

## 2. 안전 요구사항 참조 (ID만, 본문은 DOC-SAF-001)

| 안전 요구 ID | 상위 | 이 SRS와의 관계 |
|--------------|------|-----------------|
| SSR-001 ~ SSR-003 | OEM-SR-001 | 우선순위 표 순위 1, SRS-F-010 |
| SSR-004 ~ SSR-006 | OEM-SR-002 | 순위 6, SRS-F-005 |
| SSR-007 ~ SSR-010, SSR-014 | OEM-SR-003 | 입력 검증, DEGRADED, 6.4 |
| SSR-011 ~ SSR-013 | OEM-SR-004 | 순위 4, SRS-F-006, F-008 |

## 3. 품질 요구사항 (SRS-Q, ISO 25010)

| ID | 특성 > 하위 특성 | 기술 (shall) | 근거 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | OEM SWR | 상태 |
|----|------------------|--------------|------|-----------------------|----------|-----------|---------|------|
| SRS-Q-001 | 신뢰성 > 성숙성(결정론) | SW는 같은 입력 순서와 초기 상태에서 같은 제어 결과를 생성해야 한다. | 재현 가능한 검증 | 시험: 고정 시계로 같은 입력 벡터를 1,000회 재생했을 때 공개 결과(6.1)의 SHA-256 해시가 모두 같다 | 필수 | OEM-NFR-001 | SWR-016 | 초안 |
| SRS-Q-002 | 보안성 > 기밀성 | SW는 영상, 음성 원본과 개인 식별자를 입력받거나 저장하지 않아야 한다. | 개인정보 보호 | 검사: 입력 스키마와 이벤트 스키마(SRS-F-016)에 해당 필드가 없음 | 필수 | OEM-NFR-002 | SWR-012 | 초안 |
| SRS-Q-003 | 호환성 > 상호운용성 | SW는 외부와 OEM-IF-001~009가 정의한 데이터 이름, 형식, 단위, 범위, 값 집합(6.5)으로 입출력해야 한다. | 외부 인터페이스 계약 | 시험: 9개 인터페이스의 정상 값이 그대로 수용·출력되고 계약 밖 값은 거절 | 필수 | OEM-IF-001~009 | SWR-013 | 초안 |

해당 없는 ISO 25010 특성: 사용성(Web은 시험 입력용이며 HMI 승인 범위 밖), 유지보수성(품질 게이트는 프로젝트 규칙이며 OEM 요구 아님), 이식성(OEM 입력 없음). 성능 효율성은 제한 요구(SRS-L, 안전 시간 제한은 DOC-SAF-001)로 표현한다.

## 4. 제한 요구사항 (SRS-L)

| ID | ISO 25010 | 기술 (shall) | 수치/범위 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | 상태 |
|----|-----------|--------------|-----------|------------------------|----------|-----------|------|
| SRS-L-001 | 기능 적합성 > 기능 정확성 | 자동 잠금과 해제 허용 판정의 차속 임계값 | 3.0 km/h (이상=이동) | 시험: 2.9/3.0 km/h 경계값 | 필수 | OEM-FR-002 | 초안 |
| SRS-L-002 | 성능 효율성 > 시간 거동 | override 재입력 허용 시간 | 첫 억제 후 10.0초 이하 (1,000주기) | 시험: 1,000주기 경과 시 허용, 1,001주기 경과 시 불허 | 필수 | OEM-FR-003 | 초안 |
| SRS-L-003 | 성능 효율성 > 자원 활용 | 보존하는 결정 이력 건수 | 최근 100건 | 시험: 101건 입력 후 100건 | 필수 | OEM-NFR-002 | 초안 |
| SRS-L-004 | 기능 적합성 > 기능 정확성 | vehicle_speed_kph 허용 범위 | 0.0 ~ 300.0 km/h | 시험: 0.0, 300.0 수용 / -0.1, 300.1 거절 | 필수 | OEM-IF-001 | 초안 |

## 5. 제약 요구사항 (SRS-C)

| ID | 기술 (shall) | 근거 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | 상태 |
|----|--------------|------|------------------------|----------|-----------|------|
| SRS-C-001 | 제어 로직은 C17로 작성하고 Clang으로 빌드해야 한다. | 프로젝트 결정(A-11). OEM 사양은 Python 3.12이며 사용자가 편차를 승인함 | 검사: CMake 설정 C17, claudeclangbuild 이미지에서 빌드 성공 | 필수 | 프로젝트 결정(A-11), OEM-SWR-001 3장 | 초안 |
| SRS-C-002 | SW는 논리 출력(LOCK/RELEASE)까지만 생성하고 구동기 적용 feedback은 다루지 않아야 한다. | PC/SIL 경계 | 검사: 출력 인터페이스에 feedback 입력이 없음 | 필수 | OEM-IF-005 | 초안 |
| SRS-C-003 | 성인 존재 여부는 boolean 시험 신호로만 표현해야 한다. | 개인정보 제약 | 검사: adult_present의 타입이 boolean | 필수 | OEM-IF-007 | 초안 |
| SRS-C-004 | 시험 입력 주입과 결과 가시화를 위한 Web 시뮬레이터를 제공해야 한다. | OEM-SWR-001 3장 표시/조작 경계 | 시연: Web에서 입력 주입, 상태와 reason_code 확인 | 필수 | OEM-SWR-001 3장, OEM-FR-004 | 초안 |
| SRS-C-005 | 평가는 고정(논리) 시계의 10 ms 평가주기로 수행해야 한다. | 결정론, 시간 요구 검증 (A-05) | 검사+시험: 평가주기 상수 10 ms, 시계 주입 가능 | 필수 | OEM-NFR-001 | 초안 |

## 6. 정의 (요구사항의 일부)

### 6.1 용어

| 용어 | 정의 |
|------|------|
| 평가주기 | 10 ms 논리 시간 단위로 한 번 수행하는 입력 정규화와 출력 결정 (SRS-C-005) |
| VehicleSnapshot | 한 평가주기에 SW가 사용하는 정규화된 입력 묶음. 필드 timestamp_s(논리 시각, 초)를 가진다 |
| 누락 | 해당 주기 입력 메시지에 필드가 없거나 null인 경우 |
| 유효 | 누락이 아니고, 형식과 범위와 값 집합(6.5)을 만족하며, fresh한 입력. 아니면 INVALID |
| fresh | 현재 논리 시각 − 마지막 갱신 시각 ≤ 200 ms (갱신 시각 기준은 6.5) |
| 필수 입력 | OEM-IF-001(vehicle_speed_kph, gear, source_timestamp_s), -002, -003, -007, -008, -009의 모든 필드 (IF-004 명령은 이벤트이므로 제외) |
| 유효 차속 | 유효한 vehicle_speed_kph |
| 결정 | 출력, state, reason_code 중 하나가 직전 주기와 달라진 평가주기 (SRS-F-017) |
| 직전 출력 | 직전 평가주기에 확정된 좌, 우 출력. 초기값은 RELEASE |
| 공개 결과 | 평가주기마다 OEM-IF-005 출력(좌/우)과 OEM-IF-006 항목(state, priority_reason, reason_code, input_validity)을 고정 순서로 직렬화한 값 |

### 6.2 열거형

| 이름 | 값 |
|------|----|
| output | LOCK, RELEASE |
| state | OFF, NORMAL, DEGRADED, FAULT |
| reason_code (priority_reason 동일 집합) | NONE, CMD_APPLIED, CMD_REJECTED_SPEED, INPUT_INVALID, CRASH_CONFIRMED, CRASH_PENDING, FIRE, OVERTEMP, ADULT_PRESENT, IGNITION_OFF, SENSOR_FAULT, DEGRADED_STALE, APPROACH_RISK_LOCK, APPROACH_RISK_SUPPRESSED, OVERRIDE, ISOFIX_LOCK, AUTO_LOCK |
| 경고코드 | W-001 (SENSOR_FAULT) |
| input_validity | 6.1의 필수 입력별 VALID/INVALID 목록 |

### 6.3 우선순위 표 (낮은 번호가 높은 순위)

| 순위 | 규칙 | 효과 | 적용 조건 / 예외 |
|------|------|------|------------------|
| 1 | SSR-001 충돌 CONFIRMED | 좌, 우 RELEASE | crash_status가 유효한 CONFIRMED인 동안 항상 적용 (FAULT, DEGRADED 포함) |
| 2 | SRS-F-006 긴급 강제 해제 | 좌, 우 RELEASE | 해당 입력이 유효한 TRUE이면 항상 적용 (FAULT, DEGRADED 포함) |
| 3 | SRS-F-008 ignition off | 좌, 우 RELEASE, state OFF | ignition_on이 유효한 FALSE이면 항상 적용. 이 순위 이하의 모든 LOCK 규칙보다 우선 |
| 4 | SSR-011 sensor_fault | 직전 출력 유지 | sensor_fault가 유효한 TRUE인 동안 순위 5~10에 적용 |
| 5 | SSR-010 DEGRADED | 직전 출력 유지 | 필수 입력 중 하나라도 stale 또는 INVALID이면 순위 6~10에 적용 |
| 6 | SSR-004/005 접근위험 | 해당 도어 LOCK 및 unlock 억제 | 해당 도어의 접근위험이 유효한 TRUE. 예외: SRS-F-005 override |
| 7 | SRS-F-007 ISOFIX | 해당 쪽 LOCK | 해당 isofix가 유효한 TRUE |
| 8 | SRS-F-002/003 운전자 명령 | 선택 도어 LOCK/RELEASE | unlock은 유효 차속 < 3.0 km/h일 때만 |
| 9 | SRS-F-004 자동 잠금 | 좌, 우 LOCK | 유효 차속 ≥ 3.0 km/h |
| 10 | 해당 규칙 없음 | 직전 출력 유지 (reason_code NONE) | — |

- crash_status PENDING(SSR-003)은 규칙이 아니며 출력에 영향을 주지 않고 reason_code CRASH_PENDING만 기록한다 (다른 reason_code가 없을 때).
- state 결정: ignition_on FALSE → OFF, 아니면 sensor_fault TRUE → FAULT, 아니면 필수 입력 stale/INVALID → DEGRADED, 아니면 NORMAL.
- INVALID 입력은 평가에서 제외하여 그 입력을 조건으로 하는 규칙을 적용하지 않는다. INVALID IF-004 명령은 무시하고 reason_code INPUT_INVALID를 기록한다 (SSR-007).

### 6.4 상태 복귀 조건

| 상태 | 복귀 |
|------|------|
| OFF | ignition_on이 유효한 TRUE가 된 다음 평가주기 (SRS-F-009) |
| FAULT | sensor_fault가 유효한 FALSE가 된 다음 평가주기 (SSR-014) |
| DEGRADED | 모든 필수 입력이 유효해진 다음 평가주기 (SSR-014) |

### 6.5 인터페이스 데이터 정의

| IF | 데이터 | 형식/범위/값 집합 | 갱신 시각 기준 | 연관 요구 |
|----|--------|-------------------|----------------|-----------|
| OEM-IF-001 | vehicle_speed_kph, gear, source_timestamp_s | 0.0~300.0 실수; P/N/D/R; 초(실수, 0 이상) | source_timestamp_s | SRS-F-003/004, SRS-L-004 |
| OEM-IF-002 | crash_status | NONE/PENDING/CONFIRMED | 수신 논리 시각 | SSR-001/003 |
| OEM-IF-003 | rear_left/right_approach_risk | boolean | 수신 논리 시각 | SSR-004~006 |
| OEM-IF-004 | side, action, source | left/right/all; lock/unlock; physical_button/avn/voice/mobile_app | 요청 시각 = VehicleSnapshot.timestamp_s | SRS-F-001 |
| OEM-IF-005 | lock_left, lock_right | LOCK/RELEASE | — | SRS-F-002, C-002 |
| OEM-IF-006 | state, priority_reason, reason_code, input_validity | 6.2 열거형 | — | SRS-F-011~014 |
| OEM-IF-007 | fire_detected, overtemperature_detected, adult_present | boolean | 수신 논리 시각 | SRS-F-006, C-003 |
| OEM-IF-008 | isofix_left, isofix_right | boolean | 수신 논리 시각 | SRS-F-007 |
| OEM-IF-009 | ignition_on, sensor_fault | boolean | 수신 논리 시각 | SRS-F-008, SSR-011 |

gear는 형식(P/N/D/R) 검증만 하고 제어 판정에는 사용하지 않는다 (A-08).

## 7. 개수 요약

| 분류 | 개수 |
|------|------|
| 기능 (SRS-F) | 19 |
| 품질 (SRS-Q) | 3 |
| 제한 (SRS-L) | 4 |
| 제약 (SRS-C) | 5 |
| 합계 | 31 |
| 안전 요구 (SSR, 별도 문서) | 14 |
