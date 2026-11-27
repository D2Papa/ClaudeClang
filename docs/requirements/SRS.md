# SRS — 전자식 차일드락 제어 SW 요구사항 명세서

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-SRS-001 |
| 버전 | 0.8 (Iter-0, 7차 리뷰 반려 조치본) |
| 상태 | 초안 (재리뷰 대기) |
| 작성 | Claude Sonnet 5.5, 2026-11-27, 요청자 KYLee |
| 입력 | OEM-SWR-001 Rev 1.0 (OEM-A 공급자 입력, 교육용 가상 사양) |

- 기능 안전 요구사항은 이 문서에 쓰지 않는다. `docs/safety/SAFETY-REQ.md`(DOC-SAF-001)에서 관리하고, 이 문서는 ID(`SSR-nnn`)만 참조한다 (2장). 안전 규칙의 효과·조건·상태 복귀는 DOC-SAF-001에만 서술한다.
- 상위 추적 ID: `OEM-SR-*`(안전 입력), `OEM-FR-*`, `OEM-NFR-*`, `OEM-IF-*`. OEM의 `SWR-nnn`은 OEM 추적표의 참조 번호이며 `OEM SWR` 열로 교차 참조한다. SWR-019는 결번이다.
- 가정(A-nn)은 `ASSUMPTIONS.md`(DOC-ASM-001)를 참조한다.
- ID는 영구이며 재사용하지 않는다. 폐기 ID는 8장에 기록한다.
- 용어, 열거형, 우선순위 표, 인터페이스 데이터는 6장에서 정의하며 6장은 요구사항의 일부다.

## 1. 기능 요구사항 (SRS-F)

| ID | 제목 | 기술 (shall) | 근거 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | OEM SWR | 상태 |
|----|------|--------------|------|-----------------------|----------|-----------|---------|------|
| SRS-F-001 | 운전자 명령 수신 | SW는 physical_button, avn, voice, mobile_app 네 source로 수신한 side(left/right/all)와 action(lock/unlock) 명령을 수신하고, 요청 시각으로 VehicleSnapshot.timestamp_s를 사용해야 한다. | 운전자 조작 경로, 요청 시각 계약 | 시험: 4 source × 3 side × 2 action = 24개 조합이 수신되고 요청 시각이 해당 평가주기의 VehicleSnapshot.timestamp_s와 같다 | 필수 | OEM-FR-001, OEM-IF-004 | SWR-001 | 초안 |
| SRS-F-002 | 선택 도어 출력 적용 | SW는 적용이 허용된 운전자 명령을 side가 지정한 도어 출력에만 적용하고 reason_code CMD_APPLIED를 기록해야 한다. | 좌우 독립 제어 | 시험: 차속 0 km/h, 정상 입력에서 source 4종 각각에 대해 left→좌만, right→우만, all→양쪽이 LOCK/RELEASE로 변경되고 선택되지 않은 출력은 변화 없음, reason_code CMD_APPLIED | 필수 | OEM-FR-001, OEM-IF-005 | SWR-002 | 초안 |
| SRS-F-003 | 일반 해제 허용 조건 | SW는 운전자 unlock 명령을 유효 차속이 SRS-L-004 미만일 때만 적용하고, 적용하지 않은 명령은 reason_code CMD_REJECTED_SPEED로 기록해야 한다. | 주행 중 의도치 않은 해제 방지 (A-06) | 시험: 2.9 km/h 적용, 3.0 km/h 미적용+CMD_REJECTED_SPEED | 필수 | OEM-FR-001, OEM-IF-001 | SWR-004 | 초안 |
| SRS-F-004 | 속도 자동 잠금 | SW는 유효 차속이 SRS-L-004 이상이 된 첫 평가주기부터 그 상태가 유지되는 동안 매 평가주기에 좌, 우 출력을 LOCK으로 하는 규칙을 활성으로 하고 reason_code AUTO_LOCK을 기록해야 한다. 히스테리시스는 없다(A-08). | 이동 시작 시 자동 보호. NORMAL, FAULT, DEGRADED에서 같은 지속 기간으로 적용(SSR-016) | 시험: 2.9 km/h 미동작, 3.0 km/h 첫 평가주기부터 100주기 동안 두 출력 LOCK+AUTO_LOCK | 필수 | OEM-FR-002, OEM-IF-001 | SWR-003 | 초안 |
| SRS-F-007 | 해제 override | SW는 접근위험으로 해제가 억제(SSR-005)된 도어에 대해 억제 시점부터 SRS-L-005 이하의 평가주기 안에 같은 도어의 같은 unlock 명령이 재입력되고 유효 차속이 SRS-L-004 미만이면 해당 도어를 RELEASE해야 한다. override는 SSR-004, SSR-005의 LOCK과 억제에 대한 예외이며 6.3의 다른 순위(ISOFIX 등)와 SSR-010, SSR-011의 명령 미적용은 바꾸지 않는다. | 운전자의 명시적 의도 (A-03) | 시험(접근위험 TRUE, state NORMAL, 속도 < 3.0 km/h): 억제 후 1,000주기 이하 재입력 → RELEASE, 1,001주기 재입력 → 미적용(LOCK 유지), 속도 3.0 km/h → 미적용, 같은 도어에 ISOFIX TRUE → LOCK, sensor_fault TRUE 또는 DEGRADED → 명령 미적용 | 필수 | OEM-FR-003, OEM-SR-002 | SWR-006 | 초안 |
| SRS-F-014 | 긴급 강제 해제 | SW는 fire_detected, overtemperature_detected, adult_present 중 하나가 유효한 TRUE이면 다음 평가주기부터 그 입력이 TRUE인 동안 매 평가주기에 좌, 우 출력을 RELEASE로 하고 입력별 reason_code(FIRE, OVERTEMP, ADULT_PRESENT)를 기록해야 한다. 입력이 모두 FALSE가 된 주기부터는 6.3에 따라 평가하며 활성 규칙이 없으면 직전 출력을 유지한다 (A-17). | 화재, 과온, 성인 탑승 시 탈출 확보 | 시험: 3개 입력 단독 주입 시 다음 주기부터 입력이 TRUE인 100주기 동안 RELEASE+해당 이유코드(속도 3.0 km/h 이상에서도 동일), 입력이 FALSE가 되면 활성 규칙이 없을 때 출력 유지, 속도 3.0 km/h 이상이면 자동 잠금으로 LOCK | 필수 | OEM-FR-005, OEM-IF-007 | SWR-017 | 초안 |
| SRS-F-015 | ISOFIX 강제 잠금 | SW는 isofix_left 또는 isofix_right가 유효한 TRUE이면 다음 평가주기에 해당 쪽 출력을 LOCK으로 하고 반대쪽 출력은 유지하며 reason_code ISOFIX_LOCK을 기록해야 한다. | 어린이 카시트 장착 도어 보호. FAULT/DEGRADED 중 적용은 SSR-016 | 시험: 한쪽만 TRUE → 해당 출력만 LOCK+ISOFIX_LOCK, 반대쪽 불변 | 필수 | OEM-FR-006, OEM-IF-008 | SWR-018 | 초안 |
| SRS-F-016 | ignition off 해제 | SW는 ignition_on이 유효한 FALSE인 동안 매 평가주기에 좌, 우 출력을 RELEASE로 하고 state를 OFF, reason_code를 IGNITION_OFF로 해야 한다. | 시동 꺼짐 시 초기 상태 (A-14) | 시험: ignition_on=FALSE 첫 주기와 이후 100주기에서 RELEASE, state=OFF, IGNITION_OFF (ISOFIX TRUE, 접근위험 TRUE, 속도 3.0 km/h 동시 입력에서도 동일) | 필수 | OEM-FR-007, OEM-IF-009 | SWR-020 | 초안 |
| SRS-F-017 | 우선순위 중재 | SW는 둘 이상의 규칙이 서로 다른 출력을 요구하면 6.3 우선순위 표에서 순위가 높은 규칙의 효과를 적용해야 한다. | 결정론적 충돌 해소 (A-01) | 시험: 6.3의 순위 1~7 규칙 중 2개를 동시에 활성화하는 21쌍(i<j) 중 순위 3과 순위 4~7의 4쌍((3,4), (3,5), (3,6), (3,7))은 OFF 중 순위 4~7을 적용하지 않으므로(SRS-F-031) 이 시험에서 제외하고 SRS-F-031 시험으로 검증한다. 나머지 17쌍 전부에서 출력이 순위 i 규칙의 효과이다. 순위 3이 포함된 쌍((1,3), (2,3))은 state OFF, 그 외 쌍은 state NORMAL에서 시험한다. 순위 6(운전자 명령)은 unlock 명령을 쓰되, 순위 7과의 쌍은 순위 7이 속도 3.0 km/h 이상을 요구해 unlock이 허용되지 않으므로 lock 명령을 쓴다. 효과가 같은 쌍은 직전 출력을 반대 값으로 두고 SRS-F-028로 구분한다 | 필수 | OEM-SR-001, OEM-SR-002, OEM-FR-002, OEM-FR-005, OEM-FR-006, OEM-FR-007 | (파생) | 초안 |
| SRS-F-028 | priority_reason 기록 | SW는 출력을 결정한 규칙(6.3 중 가장 높은 순위의 활성 규칙)의 reason_code를 priority_reason으로 기록해야 한다. | 결정 이유 표시 (A-01) | 시험: SRS-F-017의 17쌍 모두 priority_reason이 순위 i 규칙의 reason_code(CRASH_CONFIRMED, 긴급 입력별 코드, IGNITION_OFF, APPROACH_RISK_LOCK 또는 APPROACH_RISK_SUPPRESSED, ISOFIX_LOCK, CMD_APPLIED 또는 OVERRIDE, AUTO_LOCK)이다 | 필수 | OEM-FR-004, OEM-IF-006 | (파생) | 초안 |
| SRS-F-030 | override 지속 | SW는 override가 성립한 도어에 대해 SSR-004의 LOCK을 적용하지 않고 6.3 순위 6 규칙으로 RELEASE를 유지해야 한다. 이 지속은 해당 도어의 접근위험이 FALSE가 되거나, 유효 차속이 SRS-L-004 이상이 되거나, 해당 도어에 적용되는 운전자 lock 명령이 입력되면 끝난다. 끝난 주기에는 운전자 lock 명령이면 SRS-F-002에 따라 LOCK하고, 그 외에는 6.3에 따라 평가하며 활성 규칙이 없으면 직전 출력을 유지한다. | override 효과의 지속과 종료 정의 (A-03, A-18). 자동 잠금, 운전자 명령과의 충돌 방지 | 시험: override 성립 후 접근위험 TRUE인 100주기 동안 해당 도어 RELEASE(같은 도어에 ISOFIX TRUE이면 순위 5에 따라 LOCK), 접근위험 FALSE가 되면 종료(출력 유지, reason_code NONE), 이후 접근위험 TRUE 재상승 시 LOCK, 지속 중 속도 3.0 km/h가 되면 종료하고 자동 잠금으로 LOCK, 지속 중 같은 도어 lock 명령 시 LOCK+CMD_APPLIED로 종료하고 이후 접근위험이 TRUE이면 LOCK 유지 | 필수 | OEM-FR-003, OEM-SR-002, OEM-FR-002 | SWR-006 | 초안 |
| SRS-F-031 | OFF 중 명령·규칙 미적용 | SW는 state가 OFF인 동안 운전자 명령(OEM-IF-004)과 6.3 순위 4~7의 규칙을 적용하지 않고, 운전자 명령은 reason_code IGNITION_OFF로 기록해야 한다. | OFF = 기능 비활성 (A-14) | 시험: OFF 중 lock 명령, 접근위험, ISOFIX, 속도 3.0 km/h 입력에도 출력 RELEASE 유지, 명령 주기의 reason_code IGNITION_OFF | 필수 | OEM-FR-007, OEM-IF-009 | SWR-020 | 초안 |
| SRS-F-029 | override 표시 | SW는 override가 성립한 주기와 SRS-F-030에 따라 override가 출력을 결정하는 모든 주기에 priority_reason과 reason_code를 OVERRIDE로 해야 한다. | OEM의 "override 상태" 표시 (A-03) | 시험: SRS-F-007의 RELEASE 주기와 SRS-F-030의 지속 주기 100주기 모두 priority_reason, reason_code가 OVERRIDE | 필수 | OEM-FR-003, OEM-IF-006 | SWR-006 | 초안 |
| SRS-F-018 | 상태 조회 응답 | SW는 상태 조회 요청에 OEM-IF-005의 좌, 우 출력과 SRS-F-019의 표시 항목을 응답해야 한다. | 표시 및 진단 | 통합 시험: 응답에 좌 출력, 우 출력, state, priority_reason, reason_code, input_validity가 있고 값이 마지막 평가 결과와 같다 | 필수 | OEM-FR-004 | SWR-014 | 초안 |
| SRS-F-019 | 표시 항목 제공 | SW는 표시 인터페이스에 state, priority_reason, reason_code, input_validity를 6.2 열거형 값으로 제공해야 한다. | 표시 계약 | 통합 시험: 4개 항목이 응답에 있고 6.2 열거형 값 집합에 속한다 | 필수 | OEM-FR-004, OEM-IF-006 | SWR-015 | 초안 |
| SRS-F-020 | 이벤트 ID 부여 | SW는 제어 결정마다 이전에 부여한 적이 없는 고유 이벤트 ID를 부여해야 한다. | 결정 추적 | 시험: 1,000건 기록에서 이벤트 ID 중복 0건 | 필수 | OEM-NFR-002 | SWR-010 | 초안 |
| SRS-F-021 | 결정 이력 보존 | SW는 최근 SRS-L-006 건의 결정 이벤트를 오래된 순서대로 조회할 수 있게 보존해야 한다. | 이력 조회 | 시험: 101건 입력 후 최신 100건만 시간순 조회 | 필수 | OEM-NFR-002 | SWR-012 | 초안 |
| SRS-F-022 | priority_reason 매핑 | SW는 priority_reason 값을 같은 값의 reason_code로도 제공해야 한다. | OEM-IF-006 매핑 요구 | 통합 시험: 6.2의 모든 reason_code 값에서 priority_reason 설정 시 reason_code가 같은 값 | 필수 | OEM-IF-006 | SWR-015 | 초안 |
| SRS-F-023 | 직렬화 실패 응답 | SW는 표시 인터페이스 응답을 6.4의 OEM-IF-006 형식으로 변환하는 직렬화가 오류를 반환하면 HTTP 500으로 응답해야 한다. | OEM-IF-006 오류 처리 | 통합 시험: 직렬화 오류 주입 시 HTTP 상태 500, 정상 시 200 | 필수 | OEM-IF-006 | SWR-015 | 초안 |
| SRS-F-024 | 이벤트 필드 한정 | SW는 이벤트에 이벤트 ID, 논리 시각, 좌 출력, 우 출력, state, reason_code 필드만 기록해야 한다. (개인정보 비저장은 SRS-Q-002) | 허용 필드 한정 (A-09) | 시험: 모든 이벤트의 필드 집합이 정의된 6개와 정확히 일치 | 필수 | OEM-NFR-002 | SWR-011 | 초안 |
| SRS-F-025 | 결정 기록 조건 | SW는 출력, state, reason_code 중 하나라도 직전 평가주기와 달라진 평가주기를 하나의 결정으로 보고 이벤트를 기록해야 한다. | "결정" 단위 정의 (A-15) | 시험: 변화 없는 100주기에 이벤트 0건, 변화가 있는 주기마다 이벤트 1건 | 필수 | OEM-NFR-002 | SWR-010 | 초안 |
| SRS-F-026 | 이력 휘발성 | SW는 결정 이력을 메모리에만 두고 프로세스 재기동 후에는 이력이 남지 않게 해야 한다. | 휘발성 요구 | 시험: 재기동 후 조회 건수 0 | 필수 | OEM-NFR-002 | SWR-012 | 초안 |
| SRS-F-027 | OFF 복귀 | SW는 ignition_on이 유효한 TRUE로 바뀐 다음 평가주기에 state OFF를 해제하고 6.3의 우선순위에 따라 평가해야 한다. | OFF 복귀 조건 정의 | 시험: FALSE→TRUE 후 다음 주기에 state가 OFF가 아니고(SSR-015에 따름) 출력이 규칙대로 갱신 | 필수 | OEM-FR-007, OEM-IF-009 | SWR-020 | 초안 |

## 2. 안전 요구사항 참조 (ID만, 본문은 DOC-SAF-001)

| 안전 요구 ID | 상위 | 이 SRS와의 관계 |
|--------------|------|-----------------|
| SSR-001, SSR-002, SSR-003 | OEM-SR-001 | 6.3 순위 1, SRS-F-017 |
| SSR-004, SSR-005, SSR-006 | OEM-SR-002 | 6.3 순위 4, SRS-F-007, SRS-F-030 |
| SSR-007, SSR-008, SSR-009, SSR-010, SSR-014, SSR-015 | OEM-SR-003 | 입력 검증, 6.3 순위 6, 상태 결정 |
| SSR-011, SSR-012 | OEM-SR-004 | 6.3 순위 6 |
| SSR-013 | OEM-SR-004, OEM-SR-001, OEM-SR-003 | 6.3 순위 1~3, SRS-F-014, SRS-F-016 |
| SSR-016 | OEM-SR-004, OEM-SR-003, OEM-SR-002, OEM-FR-002, OEM-FR-006 | 6.3 순위 4, 5, 7, SRS-F-004, SRS-F-015 |

## 3. 품질 요구사항 (SRS-Q, ISO 25010)

| ID | 특성 > 하위 특성 | 기술 (shall) | 근거 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | OEM SWR | 상태 |
|----|------------------|--------------|------|-----------------------|----------|-----------|---------|------|
| SRS-Q-001 | 신뢰성 > 성숙성(결정론) | SW는 같은 입력 순서와 초기 상태에서 같은 제어 결과를 생성해야 한다. | 재현 가능한 검증 | 시험: 고정 시계로 같은 입력 벡터를 1,000회 재생했을 때 공개 결과(6.1)의 SHA-256 해시가 모두 같다 | 필수 | OEM-NFR-001 | SWR-016 | 초안 |
| SRS-Q-002 | 보안성 > 기밀성 | SW는 영상, 음성 원본과 개인 식별자를 입력받거나 저장하지 않아야 한다. | 개인정보 보호 | 검사: 입력 스키마(6.4)와 이벤트 스키마(SRS-F-024)에 해당 필드가 없음 | 필수 | OEM-NFR-002 | SWR-012 | 초안 |
| SRS-Q-003 | 호환성 > 상호운용성 | SW는 외부와 6.4의 OEM-IF 9건이 정의한 데이터 이름, 형식, 단위, 범위, 값 집합으로 입출력해야 한다. | 외부 인터페이스 계약 | 시험: 9개 인터페이스의 정상 값이 그대로 수용·출력됨 (계약 밖 값의 거절은 SSR-007) | 필수 | OEM-IF-001, OEM-IF-002, OEM-IF-003, OEM-IF-004, OEM-IF-005, OEM-IF-006, OEM-IF-007, OEM-IF-008, OEM-IF-009 | SWR-013 | 초안 |

해당 없는 ISO 25010 특성: 사용성(Web은 시험 입력용이며 HMI 승인 범위 밖), 유지보수성(품질 게이트는 프로젝트 규칙이며 OEM 요구 아님), 이식성(OEM 입력 없음). 성능 효율성은 제한 요구(SRS-L, 안전 시간 제한은 DOC-SAF-001)로 표현한다.

## 4. 제한 요구사항 (SRS-L)

| ID | ISO 25010 | 기술 (shall) | 수치/범위 | 근거 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | 상태 |
|----|-----------|--------------|-----------|------|------------------------|----------|-----------|------|
| SRS-L-004 | 기능 적합성 > 기능 정확성 | 자동 잠금과 해제 허용 판정의 차속 임계값 | 3.0 km/h (이상=이동) | OEM-FR-002 수치 | 시험: 2.9/3.0 km/h 경계값 | 필수 | OEM-FR-002 | 초안 |
| SRS-L-005 | 성능 효율성 > 시간 거동 | override 재입력 허용 시간 | 억제 시점부터 1,000 평가주기(10.0초) 이하 | OEM-FR-003 수치, 주기 10 ms | 시험: 1,000주기 허용, 1,001주기 불허 | 필수 | OEM-FR-003 | 초안 |
| SRS-L-006 | 성능 효율성 > 용량 | 보존하는 결정 이력 건수 | 최근 100건 | OEM-NFR-002 수치 | 시험: 101건 입력 후 100건 | 필수 | OEM-NFR-002 | 초안 |
| SRS-L-007 | 기능 적합성 > 기능 정확성 | vehicle_speed_kph 허용 범위 | 0.0 ~ 300.0 km/h | OEM-IF-001 범위 | 시험: 0.0, 300.0 수용 / -0.1, 300.1 거절 | 필수 | OEM-IF-001 | 초안 |

## 5. 제약 요구사항 (SRS-C)

| ID | 기술 (shall) | 근거 | 검증 방법 / 합격 기준 | 우선순위 | 상위 추적 | 상태 |
|----|--------------|------|------------------------|----------|-----------|------|
| SRS-C-001 | 제어 로직은 C17로 작성하고 Clang으로 빌드해야 한다. | 프로젝트 결정(A-11). OEM 사양은 Python 3.12이며 사용자가 편차를 승인함 | 검사: CMake 설정 C17, claudeclangbuild 이미지에서 빌드 성공 | 필수 | 프로젝트 결정(A-11), OEM-SWR-001 3장 | 초안 |
| SRS-C-002 | SW는 논리 출력(LOCK/RELEASE)까지만 생성하고 구동기 적용 feedback은 다루지 않아야 한다. | PC/SIL 경계 | 검사: 출력 인터페이스에 feedback 입력이 없음 | 필수 | OEM-IF-005 | 초안 |
| SRS-C-003 | 성인 존재 여부는 boolean 시험 신호로만 표현해야 한다. (값 집합 검증은 SSR-007) | 개인정보 제약 | 검사: adult_present의 타입이 boolean | 필수 | OEM-IF-007 | 초안 |
| SRS-C-004 | 시험 입력 주입과 결과 가시화를 위한 Web 시뮬레이터를 제공해야 한다. | OEM-SWR-001 3장 표시/조작 경계 | 시연: Web에서 입력 주입, 상태와 reason_code 확인 | 필수 | OEM-SWR-001 3장, OEM-FR-004 | 초안 |
| SRS-C-005 | 평가는 고정(논리) 시계의 10 ms 평가주기로 수행해야 한다. | 결정론, 시간 요구 검증 (A-05) | 검사+시험: 평가주기 상수 10 ms, 시계 주입 가능 | 필수 | OEM-NFR-001, OEM-SR-001, OEM-SR-003 | 초안 |

## 6. 정의 (요구사항의 일부)

### 6.1 용어

| 용어 | 정의 |
|------|------|
| 평가주기 | 10 ms 논리 시간 단위로 한 번 수행하는 입력 정규화와 출력 결정 (SRS-C-005) |
| 논리 시각 | 시험에서 주입하는 단일 시간축의 값. 모든 시각(VehicleSnapshot.timestamp_s, source_timestamp_s, 수신 시각, 이벤트 논리 시각)은 이 시간축이다 |
| VehicleSnapshot | 한 평가주기에 SW가 사용하는 정규화된 입력 묶음. timestamp_s는 그 주기 시작의 논리 시각이다 |
| 누락 | 해당 주기 입력 메시지에 필드가 없거나 null인 경우 |
| 유효 | 누락이 아니고, 형식·범위·값 집합(6.4)을 만족하며, 필수 입력이면 fresh한 입력. 아니면 INVALID |
| fresh | IF-001: timestamp_s − source_timestamp_s ≤ 200 ms. 그 외 필수 입력: 현재 논리 시각 − 마지막 수신 논리 시각 ≤ 200 ms (DOC-SAF-001 SSR-008) |
| 필수 입력 | OEM-IF-001의 vehicle_speed_kph와 source_timestamp_s, OEM-IF-002, 003, 007, 008, 009의 모든 필드. gear와 IF-004 명령은 제외한다 |
| 유효 차속 | 유효한 vehicle_speed_kph |
| 활성 규칙 | 해당 평가주기에 조건이 성립해 출력 효과를 요구하는 6.3의 규칙(순위 1~7). 활성 규칙이 없으면 순위 8 |
| 결정 | 출력, state, reason_code 중 하나가 직전 주기와 달라진 평가주기 (SRS-F-025) |
| 직전 출력 | 직전 평가주기에 확정된 좌, 우 출력. 초기값은 RELEASE |
| 공개 결과 | 평가주기마다 OEM-IF-005 출력과 OEM-IF-006 항목(state, priority_reason, reason_code, input_validity)을 고정 순서로 직렬화한 값 |

### 6.2 열거형

| 이름 | 값 |
|------|----|
| output | LOCK, RELEASE |
| state | OFF, NORMAL, DEGRADED, FAULT |
| reason_code (priority_reason 동일 집합) | NONE, CMD_APPLIED, CMD_REJECTED_SPEED, INPUT_INVALID, CRASH_CONFIRMED, CRASH_PENDING, FIRE, OVERTEMP, ADULT_PRESENT, IGNITION_OFF, SENSOR_FAULT, DEGRADED_STALE, APPROACH_RISK_LOCK, APPROACH_RISK_SUPPRESSED, OVERRIDE, ISOFIX_LOCK, AUTO_LOCK |
| 경고코드 | W-001 |
| input_validity | 6.1 필수 입력과 gear의 항목별 VALID/INVALID 목록 |

### 6.3 우선순위 표 (낮은 번호가 높은 순위)

안전 규칙(SSR)의 효과와 조건은 DOC-SAF-001에만 기술하며 이 표는 순위와 참조만 정한다.

| 순위 | 규칙 | 효과·조건 |
|------|------|-----------|
| 1 | SSR-001 충돌 CONFIRMED | DOC-SAF-001 SSR-001, SSR-013 |
| 2 | SRS-F-014 긴급 강제 해제 | SRS-F-014, SSR-013 |
| 3 | SRS-F-016 ignition off | SRS-F-016, SRS-F-031, SSR-013 |
| 4 | SSR-004, SSR-005 접근위험 | DOC-SAF-001 SSR-004, SSR-005, SSR-016, 예외 SRS-F-007, SRS-F-030 |
| 5 | SRS-F-015 ISOFIX | SRS-F-015, SSR-016 |
| 6 | SRS-F-002, SRS-F-003, SRS-F-007 운전자 명령, SRS-F-030 override 지속 | 각 요구(override 지속의 reason_code는 OVERRIDE), 명령 미적용은 SSR-010, SSR-011 |
| 7 | SRS-F-004 자동 잠금 | SRS-F-004, SSR-016 |
| 8 | 해당 규칙 없음 | 직전 출력 유지, reason_code NONE |

- state DEGRADED 또는 FAULT의 영향은 순위 6(운전자 명령)에만 있으며 SSR-010, SSR-011이 정한다. 나머지 규칙은 SSR-013, SSR-016에 따라 상태와 무관하게 적용한다.
- state OFF인 동안 순위 4~7은 적용하지 않는다(SRS-F-031).
- crash_status PENDING은 규칙이 아니다(SSR-003). INVALID 입력 처리는 SSR-007, state 결정은 SSR-015, FAULT/DEGRADED 복귀는 SSR-014, OFF 복귀는 SRS-F-027을 따른다.

### 6.4 인터페이스 데이터 정의

| IF | 데이터 | 형식/범위/값 집합 | 갱신 시각 기준 | 연관 요구 |
|----|--------|-------------------|----------------|-----------|
| OEM-IF-001 | vehicle_speed_kph, gear, source_timestamp_s | 0.0~300.0 실수; P/N/D/R; 논리 시각(초, 0 이상) | source_timestamp_s | SRS-F-003, SRS-F-004, SRS-L-007, SSR-008 |
| OEM-IF-002 | crash_status | NONE/PENDING/CONFIRMED | 수신 논리 시각 | SSR-001, SSR-003 |
| OEM-IF-003 | rear_left_approach_risk, rear_right_approach_risk | boolean | 수신 논리 시각 | SSR-004, SSR-005, SSR-006 |
| OEM-IF-004 | side, action, source | left/right/all; lock/unlock; physical_button/avn/voice/mobile_app | 요청 시각 = VehicleSnapshot.timestamp_s | SRS-F-001 |
| OEM-IF-005 | lock_left, lock_right | LOCK/RELEASE | — | SRS-F-002, SRS-C-002 |
| OEM-IF-006 | state, priority_reason, reason_code, input_validity | 6.2 열거형 | — | SRS-F-019, SRS-F-022, SRS-F-023 |
| OEM-IF-007 | fire_detected, overtemperature_detected, adult_present | boolean | 수신 논리 시각 | SRS-F-014, SRS-C-003 |
| OEM-IF-008 | isofix_left, isofix_right | boolean | 수신 논리 시각 | SRS-F-015 |
| OEM-IF-009 | ignition_on, sensor_fault | boolean | 수신 논리 시각 | SRS-F-016, SSR-011 |

gear는 형식(P/N/D/R) 검증만 하고 제어 판정과 freshness·DEGRADED 판정에는 사용하지 않는다. gear가 INVALID이면 input_validity에 표시만 한다 (A-08, SSR-007).

## 7. 개수 요약

| 분류 | 개수 |
|------|------|
| 기능 (SRS-F) | 23 |
| 품질 (SRS-Q) | 3 |
| 제한 (SRS-L) | 4 |
| 제약 (SRS-C) | 5 |
| 합계 | 35 |
| 안전 요구 (SSR, 별도 문서) | 16 |

## 8. ID 이력 (1차 초안 v0.1 → 현재)

| 폐기(결번) ID | 사유 | 이관 |
|---------------|------|------|
| SRS-F-005, SRS-F-006, SRS-F-008, SRS-F-009, SRS-F-010, SRS-F-011, SRS-F-012, SRS-F-013 | 안전 요구로 분리 | SSR-004, 005, 006, 001, 003, 007, 008·009·010, 011·012 (DOC-SAF-001) |
| SRS-L-001, SRS-L-002, SRS-L-003 | 안전 요구로 분리 | SSR-002, SSR-008, SSR-009 |

- 위 ID는 재사용하지 않는다.
- 의미가 유지된 ID(SRS-F-001~003, 014~016, 018, 021, SRS-Q-001, 002, SRS-L-004~007, SRS-C-001~005)는 1차 초안과 같은 ID를 쓴다.
- 내용이 변경된 ID: SRS-F-004(첫 평가주기 LOCK에서 지속 규칙으로, 5차 리뷰 조치. 효과는 같고 지속 기간만 명확화), SRS-F-014(입력 지속 중 매 주기 적용과 종료 후 동작 추가, 6차 리뷰 조치), SRS-F-016(첫 평가주기에서 OFF 지속 중 매 주기로, 4차 리뷰 조치).
- 범위가 축소된 ID: SRS-F-007(override 출력만 남김, 표시는 SRS-F-029, 지속은 SRS-F-030), SRS-F-017(우선순위 적용만 남김, 이유 기록은 SRS-F-028), SRS-F-019(표시 항목만 남김, 매핑·직렬화는 SRS-F-022, 023), SRS-F-020(이벤트 ID 부여만 남김, 필드·기록 조건은 SRS-F-024, 025). 축소 부분은 신규 ID로 이관했다.
- 이후 신규 요구는 SRS-F-032, SRS-L-008 등 미사용 번호를 쓴다.
