# DDS — 제어 코어 상세 설계서 (Iter-1)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-DDS-001 |
| 버전 | 0.1 |
| 상태 | 리뷰 생략 (사용자 지시 2026-11-27), aspice-reviewer 미검토 |
| 작성 | Claude Sonnet 5.5, 2026-11-27, 요청자 KYLee |
| 입력 | DOC-ARC-001 v0.1 (ARCH-C-001~013), DOC-SRS-001, DOC-SAF-001 |

- **인터페이스 정의 기준 수준**: 다른 모듈이 호출하는 함수가 하나라도 있는 모듈은 인터페이스(DD-I)를 정의한다. 이 설계에서는 모든 모듈이 ControlCycle에서 호출되므로 전부 대상이다. 각 DD-I는 ARCH-I와 1:1로 대응한다.
- 모든 함수는 `Status_t`를 반환하고 `void`가 없으며 `return`은 함수 끝에 한 번이다. 함수당 CCN 10 이하, 80라인 이하다.
- 모든 모듈은 공개 헤더(`.h`)의 함수로만 서로를 호출한다. 전역 가변 데이터는 없다.
- ARCH 보완: 한 주기의 호출 순서를 ARCH 5장과 다음처럼 정한다. Validator → Freshness → State → **DriverCommandRule** → Override_Update → 나머지 6개 규칙 → Arbiter. 이유: Override_Update가 이번 주기의 명령 요청 결과를 입력으로 쓰고, ApproachRiskRule이 override 상태를 입력으로 쓰기 때문이다. ARCH-I-004의 RuleInput_t의 OverrideStatus_t는 DriverCommandRule에서는 사용하지 않는다.

## 1. 공용 정의 (`src/common/`)

| 파일 | 내용 |
|------|------|
| status.h | Status_t: STATUS_OK=0, STATUS_ERR_PARAM=1, STATUS_ERR_STATE=2 |
| params.h | `CYCLE_PERIOD_S=0.01`, `SPEED_THRESHOLD_KPH=3.0`, `SPEED_MAX_KPH=300.0`, `FRESHNESS_LIMIT_S=0.2`, `OVERRIDE_WINDOW_CYCLES=1000`, `HISTORY_CAPACITY=100` (읽기 전용 상수) |
| types.h | Output_t, Door_t, Validity_t, RawKind_t, RawValue_t, RawInput_t, Num_t, Enum_t, Flag_t, Command_t, Snapshot_t, State_t, ReasonCode_t, RuleInput_t, RuleResult_t, OverrideStatus_t, ArbiterInput_t, Decision_t, CycleResult_t (ARCH 4장 정의 구체화) |

구체화한 타입: `Flag_t{int value; Validity_t validity;}`, `Num_t{double value; Validity_t validity;}`, `Enum_t{int value; Validity_t validity;}`, `Command_t{int present; Validity_t validity; int side; int action; int source;}`.
열거: Gear(P=0,N,D,R), Crash(NONE=0,PENDING,CONFIRMED), Side(LEFT=0,RIGHT,ALL), Action(LOCK=0,UNLOCK), Source(PHYSICAL=0,AVN,VOICE,MOBILE).
`RuleResult_t`: active[2], effect[2], reason[2](활성 규칙의 reason_code), info_reason[2](비활성이지만 기록할 코드: CRASH_PENDING, CMD_REJECTED_SPEED, INPUT_INVALID), unlock_allowed[2], lock_requested[2], suppressed[2].

## 2. 모듈 정의

| ID | 모듈(파일, `src/control/`) | 컴포넌트 | 책임 | 인터페이스 | 의존 | 데이터 |
|----|---------------------------|----------|------|-----------|------|--------|
| DD-M-001 | input_validator.c/.h | ARCH-C-001 | 원시 입력을 검증해 Snapshot_t를 만든다 | DD-I-001 Validator_Normalize | common | 없음(비보유) |
| DD-M-002 | freshness_monitor.c/.h | ARCH-C-002 | timestamp 단조성과 IF-001 시각 차이를 검사한다 | DD-I-002 Freshness_Init, Freshness_Apply | common | Freshness_t{initialized, prev_timestamp_s}. 불변: initialized이면 prev는 직전 주기 시각 |
| DD-M-003 | state_manager.c/.h | ARCH-C-003 | state와 경고코드를 정한다 | DD-I-003 State_Determine | common | 없음 |
| DD-M-004 | crash_rule.c/.h | ARCH-C-004 | 충돌 규칙을 평가한다 | DD-I-004 Crash_Evaluate | common | 없음 |
| DD-M-005 | emergency_rule.c/.h | ARCH-C-005 | 긴급 강제 해제 규칙을 평가한다 | DD-I-004 Emergency_Evaluate | common | 없음 |
| DD-M-006 | ignition_rule.c/.h | ARCH-C-006 | ignition off 규칙을 평가한다 | DD-I-004 Ignition_Evaluate | common | 없음 |
| DD-M-007 | approach_risk_rule.c/.h | ARCH-C-007 | 접근위험 규칙을 평가한다 | DD-I-004 ApproachRisk_Evaluate | common | 없음 |
| DD-M-008 | isofix_rule.c/.h | ARCH-C-008 | ISOFIX 규칙을 평가한다 | DD-I-004 Isofix_Evaluate | common | 없음 |
| DD-M-009 | driver_command_rule.c/.h | ARCH-C-009 | 운전자 명령 규칙을 평가한다 | DD-I-004 DriverCommand_Evaluate | common | 없음 |
| DD-M-010 | auto_lock_rule.c/.h | ARCH-C-010 | 자동 잠금 규칙을 평가한다 | DD-I-004 AutoLock_Evaluate | common | 없음 |
| DD-M-011 | override_manager.c/.h | ARCH-C-011 | override 성립, 지속, 종료를 추적한다 | DD-I-005 Override_Init, Override_Update | common | Override_t{cycle_valid, suppressed_cycle[2], suppressed_valid[2], active[2]}. 불변: active이면 suppressed_valid 해제 |
| DD-M-012 | arbiter.c/.h | ARCH-C-012 | 우선순위 중재와 상태 억제를 적용한다 | DD-I-006 Arbiter_Resolve | common | 없음 |
| DD-M-013 | control_cycle.c/.h | ARCH-C-013 | 한 주기의 호출 순서와 컨텍스트를 관리한다 | DD-I-007 Control_Init, Control_Step | DD-M-001~012의 공개 헤더 | Control_t{Freshness_t, Override_t, prev_output[2], prev_suppressed[2], cycle, initialized} |

## 3. 함수 정의

공통: 모든 함수의 반환은 `STATUS_OK`(정상), `STATUS_ERR_PARAM`(널 포인터 등 인자 오류), `STATUS_ERR_STATE`(컨텍스트 오류). 동작은 "검증 → 계산 → 단일 반환" 구조다.

### DD-M-001 input_validator

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-001 | `Status_t Validator_Normalize(const RawInput_t *raw, Snapshot_t *snap)` | 널이면 ERR_PARAM. 그렇지 않으면 timestamp 복사, 필드별 DD-F-002~005 호출, 명령은 DD-F-006 호출 | SSR-007, SRS-F-001, SRS-Q-003 |
| DD-F-002 | `static Status_t Validate_Flag(const RawValue_t *in, Flag_t *out)` | kind==BOOLEAN이고 boolean이 0 또는 1이면 VALID, 아니면 INVALID(값 0) | SSR-007, SRS-C-003 |
| DD-F-003 | `static Status_t Validate_Number(const RawValue_t *in, double min, double max, Num_t *out)` | kind==NUMBER이고 값이 유한하며 min 이상 max 이하이면 VALID, 아니면 INVALID | SSR-007, SRS-L-007 |
| DD-F-004 | `static Status_t Validate_Enum(const RawValue_t *in, int max_id, Enum_t *out)` | kind==ENUM이고 0 이상 max_id 이하이면 VALID, 아니면 INVALID | SSR-007 |
| DD-F-005 | `static Status_t Validate_Command(const RawInput_t *raw, Command_t *out)` | 명령 필드 3개가 모두 ABSENT이면 present=0. 하나라도 있으면 present=1이고 side, action, source가 모두 유효할 때만 VALID, 아니면 INVALID | SRS-F-001, SSR-007 |

### DD-M-002 freshness_monitor

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-006 | `Status_t Freshness_Init(Freshness_t *ctx)` | 널이면 ERR_PARAM, 아니면 initialized=1, prev=-1.0 | SSR-008 |
| DD-F-007 | `Status_t Freshness_Apply(Freshness_t *ctx, Snapshot_t *snap)` | 널이면 ERR_PARAM, 미초기화면 ERR_STATE. snap->timestamp가 prev보다 작으면 ERR_STATE. 그렇지 않으면 speed와 source_ts가 VALID일 때 (timestamp − source_ts)가 FRESHNESS_LIMIT_S를 초과하거나 0 미만이면 둘 다 INVALID로 표시하고 prev 갱신 | SSR-008 |

### DD-M-003 state_manager

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-008 | `Status_t State_Determine(const Snapshot_t *snap, State_t *state, int *warning_code)` | 널이면 ERR_PARAM. off = ignition이 VALID이고 값 0, fault = sensor_fault가 VALID이고 값 1, degraded = DD-F-009 > 0. state = off ? OFF : fault ? FAULT : degraded ? DEGRADED : NORMAL. warning_code = fault ? 1 : 0 | SSR-009, 012, 014, 015, SRS-F-016, 027 |
| DD-F-009 | `static int State_CountInvalidRequired(const Snapshot_t *snap)` | 필수 입력(speed, source_ts, crash, approach 2, fire, overtemp, adult, isofix 2, ignition, sensor_fault)의 INVALID 개수. gear 제외 | SSR-008, 009 |

### DD-M-004~010 규칙 (공통 구조)

공통: 널이면 ERR_PARAM(out 미수정). out을 먼저 `RuleResult_Clear`(common 인라인이 아닌 각 모듈 static 초기화)로 비활성 초기화한 뒤 조건을 평가한다.

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-010 | `Status_t Crash_Evaluate(const RuleInput_t *in, RuleResult_t *out)` | crash VALID이고 CONFIRMED면 두 도어 active, RELEASE, reason CRASH_CONFIRMED. PENDING이면 info_reason=CRASH_PENDING | SSR-001, 002, 003 |
| DD-F-011 | `Status_t Emergency_Evaluate(...)` | fire VALID TRUE → FIRE, else overtemp VALID TRUE → OVERTEMP, else adult VALID TRUE → ADULT_PRESENT. 해당하면 두 도어 active, RELEASE | SRS-F-014 |
| DD-F-012 | `Status_t Ignition_Evaluate(...)` | ignition VALID이고 값 0이면 두 도어 active, RELEASE, IGNITION_OFF | SRS-F-016 |
| DD-F-013 | `Status_t ApproachRisk_Evaluate(...)` | 도어별: approach VALID TRUE이고 in->override.active[d]==0이면 active, LOCK, reason=APPROACH_RISK_LOCK. 같은 도어를 지정한 유효한 unlock 명령이 있으면 suppressed[d]=1, reason=APPROACH_RISK_SUPPRESSED | SSR-004, 005, 006 |
| DD-F-014 | `Status_t Isofix_Evaluate(...)` | 쪽별: isofix VALID TRUE이면 active, LOCK, ISOFIX_LOCK | SRS-F-015 |
| DD-F-015 | `Status_t DriverCommand_Evaluate(...)` | 명령 present이고 INVALID면 info_reason=INPUT_INVALID. VALID면 지정 도어(DD-F-016)별로: lock → active, LOCK, CMD_APPLIED, lock_requested=1. unlock → 유효 차속이 SPEED_THRESHOLD_KPH 미만이면 active, RELEASE, CMD_APPLIED, unlock_allowed=1, 아니면 info_reason=CMD_REJECTED_SPEED | SRS-F-001, 002, 003, SRS-L-004 |
| DD-F-016 | `static int Command_TargetsDoor(const Command_t *cmd, Door_t door)` | side가 ALL이거나 door와 같으면 1 | SRS-F-002 |
| DD-F-017 | `Status_t AutoLock_Evaluate(...)` | 유효 차속이 SPEED_THRESHOLD_KPH 이상이면 두 도어 active, LOCK, AUTO_LOCK | SRS-F-004 |

### DD-M-011 override_manager

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-018 | `Status_t Override_Init(Override_t *ctx)` | 전부 0으로 초기화 | SRS-F-007 |
| DD-F-019 | `Status_t Override_Update(Override_t *ctx, const OverrideInput_t *in, OverrideStatus_t *out)` | 널이면 ERR_PARAM. 도어별로 DD-F-020 호출 후 out->active 복사 | SRS-F-007, 029, 030 |
| DD-F-020 | `static Status_t Override_UpdateDoor(Override_t *ctx, const OverrideInput_t *in, int d)` | OverrideInput_t = cycle, state, 도어별 risk_true / speed_ge(차속 3.0 이상) / unlock_allowed / lock_requested / prev_suppressed. (1) active이면 risk_true가 0이거나 speed_ge이거나 (lock_requested이고 state가 NORMAL)이면 active=0, 기록 해제. (2) active가 아니고 state가 NORMAL이며 unlock_allowed이고 risk_true이며 기록이 유효하고 cycle − 기록 ≤ OVERRIDE_WINDOW_CYCLES이면 active=1, 기록 해제. (3) active가 아니고 prev_suppressed이며 state NORMAL이고 (기록 없음 또는 만료)이면 기록=cycle−1. (4) risk_true가 0이면 기록 해제 | SRS-F-007, 030, SRS-L-005 |

### DD-M-012 arbiter

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-021 | `Status_t Arbiter_Resolve(const ArbiterInput_t *in, Decision_t *out)` | 널이면 ERR_PARAM. 도어별로 DD-F-022 호출, 상태 기반 reason(DD-F-024)과 priority_reason 설정 | SRS-F-017, 028, 031, SSR-010, 011, 013, 016 |
| DD-F-022 | `static int Arbiter_RankGated(State_t state, int rank)` | state OFF이고 rank ≥ 4이면 1, state DEGRADED 또는 FAULT이고 rank == 6이면 1, 아니면 0 | SRS-F-031, SSR-010, 011 |
| DD-F-023 | `static int Arbiter_PickRank(const ArbiterInput_t *in, int d, Output_t *effect, ReasonCode_t *reason)` | 순위 1~7을 순서대로 보며 gated가 아니고 active[d]인 첫 규칙을 선택. 순위 6은 규칙 결과가 비활성이고 override.active[d]이면 RELEASE, OVERRIDE로 대체. 선택한 순위(0이면 없음)를 반환 | SRS-F-017, 029, 030 |
| DD-F-024 | `static ReasonCode_t Arbiter_FallbackReason(const ArbiterInput_t *in, int d)` | 선택된 규칙이 없을 때: state FAULT → SENSOR_FAULT, DEGRADED → DEGRADED_STALE, 아니면 info_reason이 있는 가장 높은 순위의 값, 없으면 NONE | SSR-009, 012, SRS-F-003 |

### DD-M-013 control_cycle

| ID | 시그니처 | 동작 | 추적 |
|----|----------|------|------|
| DD-F-025 | `Status_t Control_Init(Control_t *ctx)` | 널이면 ERR_PARAM. Freshness_Init, Override_Init, prev_output=RELEASE, cycle=0, initialized=1 | SRS-C-005, Q-001 |
| DD-F-026 | `Status_t Control_Step(Control_t *ctx, const RawInput_t *raw, CycleResult_t *out)` | 널이면 ERR_PARAM, 미초기화면 ERR_STATE. 순서대로 Validator_Normalize, Freshness_Apply, State_Determine, DriverCommand_Evaluate, Override_Update(입력은 DD-F-027로 구성), 나머지 6개 규칙 평가, Arbiter_Resolve. 한 단계라도 오류가 나면 이후 단계를 건너뛰고 오류를 반환하며 ctx의 직전 출력은 갱신하지 않는다. 성공하면 DD-F-028로 결과를 채우고 ctx 갱신 | SRS-Q-001, SRS-C-002, SRS-C-005, SSR-002 |
| DD-F-027 | `static Status_t Control_BuildOverrideInput(const Control_t *ctx, const Snapshot_t *snap, State_t state, const RuleResult_t *cmd, OverrideInput_t *out)` | 스냅샷의 접근위험 VALID TRUE, 차속 유효 3.0 이상 여부, cmd의 unlock_allowed, lock_requested, ctx->prev_suppressed, ctx->cycle, state를 채운다 | SRS-F-007 |
| DD-F-028 | `static Status_t Control_FillResult(const Decision_t *dec, State_t state, int warning, const Snapshot_t *snap, CycleResult_t *out)` | 출력, state, priority_reason, reason_code, warning_code, input_validity(필수 입력과 gear의 VALID/INVALID 비트)를 채운다 | SRS-F-019 (Iter-2 표시용 데이터) |

## 4. 점검표 결과

| 항목 | 결과 |
|------|------|
| 인터페이스 정의 | 13개 모듈 전부 DD-I 대상이며 ARCH-I-001~007과 1:1 |
| 의존 | 모듈 간 의존은 ControlCycle → 하위 모듈 공개 헤더뿐이며 하위 모듈끼리 의존 없음 |
| 이름 | 역할 동사(Normalize, Determine, Evaluate, Resolve, Update) 사용 |
| 단일 책임 | 규칙 7개 분리, 검증 보조 함수 분리. 함수 설명에 "그리고" 없음 |
| 크기 | 가장 복잡한 DD-F-020(Override 도어 갱신)과 DD-F-015(운전자 명령)는 보조 함수로 분리해 CCN 10 이하가 되도록 구현한다 (구현 시 Lizard로 확인) |
| 반환 | 모든 함수가 Status_t 또는 정수 반환, 단일 반환 |
| 추적 | 3장의 추적 열과 TRACE.md |

## 5. 요구 → 설계 배정 (상세 설계 ID)

| 요구 ID | 상세 설계 |
|---------|-----------|
| SRS-F-001 | DD-F-001, DD-F-005, DD-F-015 |
| SRS-F-002 | DD-F-015, DD-F-016 |
| SRS-F-003 | DD-F-015, DD-F-024 |
| SRS-F-004 | DD-F-017 |
| SRS-F-007 | DD-F-018, DD-F-019, DD-F-020, DD-F-027 |
| SRS-F-014 | DD-F-011 |
| SRS-F-015 | DD-F-014 |
| SRS-F-016 | DD-F-008, DD-F-012 |
| SRS-F-017 | DD-F-021, DD-F-023 |
| SRS-F-027 | DD-F-008 |
| SRS-F-028 | DD-F-021, DD-F-023 |
| SRS-F-029 | DD-F-019, DD-F-023 |
| SRS-F-030 | DD-F-020, DD-F-023 |
| SRS-F-031 | DD-F-021, DD-F-022 |
| SRS-Q-001 | DD-F-025, DD-F-026 |
| SRS-Q-003 | DD-F-001 |
| SRS-L-004 | DD-F-015, DD-F-017 |
| SRS-L-005 | DD-F-020 |
| SRS-L-007 | DD-F-003 |
| SRS-C-001 | DD-F-026 |
| SRS-C-002 | DD-F-026 |
| SRS-C-003 | DD-F-002 |
| SRS-C-005 | DD-F-025, DD-F-026 |
| SSR-001 | DD-F-010 |
| SSR-002 | DD-F-010, DD-F-026 |
| SSR-003 | DD-F-010, DD-F-024 |
| SSR-004 | DD-F-013 |
| SSR-005 | DD-F-013, DD-F-020 |
| SSR-006 | DD-F-013 |
| SSR-007 | DD-F-001, DD-F-002, DD-F-003, DD-F-004, DD-F-005 |
| SSR-008 | DD-F-007, DD-F-009 |
| SSR-009 | DD-F-008, DD-F-009, DD-F-024 |
| SSR-010 | DD-F-021, DD-F-022 |
| SSR-011 | DD-F-021, DD-F-022 |
| SSR-012 | DD-F-008, DD-F-024 |
| SSR-013 | DD-F-021, DD-F-023 |
| SSR-014 | DD-F-008 |
| SSR-015 | DD-F-008 |
| SSR-016 | DD-F-021, DD-F-023 |

Iter-1 범위 밖(표시·이력·Web, ARCH-C-014~016)은 Iter-2의 상세 설계에서 다룬다.
