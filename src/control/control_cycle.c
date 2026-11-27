/**
 * @file    control_cycle.c
 * @brief   평가주기 실행 구현.
 * @trace   SRS-Q-001, SRS-C-002, SRS-C-005, SSR-002
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
#include "control/control_cycle.h"
#include "common/params.h"
#include "control/input_validator.h"
#include "control/state_manager.h"
#include "control/crash_rule.h"
#include "control/emergency_rule.h"
#include "control/ignition_rule.h"
#include "control/approach_risk_rule.h"
#include "control/isofix_rule.h"
#include "control/driver_command_rule.h"
#include "control/auto_lock_rule.h"
#include "control/arbiter.h"
#include <stddef.h>

/** 규칙 평가 함수 포인터 (ARCH-I-004) */
typedef Status_t (*RuleFunction_t)(const RuleInput_t *in, RuleResult_t *out);

/** 순위 순서의 규칙 평가 함수 표 */
static const RuleFunction_t RULE_TABLE[RULE_COUNT] = {
    Crash_Evaluate,        Emergency_Evaluate, Ignition_Evaluate, ApproachRisk_Evaluate,
    Isofix_Evaluate,       DriverCommand_Evaluate, AutoLock_Evaluate};

/**
 * @brief override 갱신 입력을 만든다.
 * @param[in]  ctx    컨텍스트
 * @param[in]  snap   검증된 입력
 * @param[in]  state  이번 주기 state
 * @param[in]  cmd    운전자 명령 규칙 결과
 * @param[out] out    갱신 입력
 * @return STATUS_OK
 * @trace DD-F-027
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
static Status_t Control_BuildOverrideInput(const Control_t *ctx, const Snapshot_t *snap, State_t state,
                                           const RuleResult_t *cmd, OverrideInput_t *out)
{
    int d;

    out->cycle = ctx->cycle;
    out->state = state;
    for (d = 0; d < DOOR_COUNT; d++)
    {
        out->risk_true[d] = ((snap->approach[d].validity == VALIDITY_VALID) && (snap->approach[d].value == 1)) ? 1 : 0;
        out->speed_ge[d] = ((snap->speed.validity == VALIDITY_VALID) && (snap->speed.value >= SPEED_THRESHOLD_KPH)) ? 1 : 0;
        out->unlock_allowed[d] = cmd->unlock_allowed[d];
        out->lock_requested[d] = cmd->lock_requested[d];
        out->prev_suppressed[d] = ctx->prev_suppressed[d];
    }
    return STATUS_OK;
}

/**
 * @brief 유효하면 지정한 비트를, 아니면 0을 돌려준다.
 * @param[in] validity  유효성
 * @param[in] bit       비트 값
 * @return 비트 또는 0
 * @trace SRS-F-019
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
static unsigned int Control_ValidBit(Validity_t validity, unsigned int bit)
{
    unsigned int result = 0U;

    if (validity == VALIDITY_VALID)
    {
        result = bit;
    }
    return result;
}

/**
 * @brief 필수 입력과 gear의 유효성 비트 마스크를 만든다 (비트 0~12).
 * @param[in] snap  검증된 입력
 * @return VALID이면 1인 비트 마스크
 * @trace SRS-F-019
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
static unsigned int Control_InputValidity(const Snapshot_t *snap)
{
    unsigned int mask = 0U;

    mask |= Control_ValidBit(snap->speed.validity, 1U);
    mask |= Control_ValidBit(snap->gear.validity, 2U);
    mask |= Control_ValidBit(snap->source_timestamp.validity, 4U);
    mask |= Control_ValidBit(snap->crash.validity, 8U);
    mask |= Control_ValidBit(snap->approach[DOOR_LEFT].validity, 16U);
    mask |= Control_ValidBit(snap->approach[DOOR_RIGHT].validity, 32U);
    mask |= Control_ValidBit(snap->fire.validity, 64U);
    mask |= Control_ValidBit(snap->overtemp.validity, 128U);
    mask |= Control_ValidBit(snap->adult.validity, 256U);
    mask |= Control_ValidBit(snap->isofix[DOOR_LEFT].validity, 512U);
    mask |= Control_ValidBit(snap->isofix[DOOR_RIGHT].validity, 1024U);
    mask |= Control_ValidBit(snap->ignition.validity, 2048U);
    mask |= Control_ValidBit(snap->sensor_fault.validity, 4096U);
    return mask;
}

/**
 * @brief 운전자 명령 규칙을 먼저 평가한 뒤 override를 갱신하고 나머지 규칙을 평가한다.
 * @param[inout] ctx    컨텍스트
 * @param[in]    snap   검증된 입력
 * @param[in]    state  이번 주기 state
 * @param[out]   arb    중재 입력
 * @return STATUS_OK
 * @trace DD-F-026
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
static Status_t Control_EvaluateRules(Control_t *ctx, const Snapshot_t *snap, State_t state, ArbiterInput_t *arb)
{
    RuleInput_t rule_in = {NULL, STATE_NORMAL, {OUTPUT_RELEASE, OUTPUT_RELEASE}, {{0, 0}}};
    OverrideInput_t ov_in;
    int i;

    rule_in.snapshot = snap;
    rule_in.state = state;
    rule_in.previous[DOOR_LEFT] = ctx->previous[DOOR_LEFT];
    rule_in.previous[DOOR_RIGHT] = ctx->previous[DOOR_RIGHT];
    /* 아래 호출들은 인자가 널이 아님이 이 함수에서 보장되므로 오류를 반환하지 않는다. */
    DriverCommand_Evaluate(&rule_in, &arb->rules[RANK_DRIVER]);
    Control_BuildOverrideInput(ctx, snap, state, &arb->rules[RANK_DRIVER], &ov_in);
    Override_Update(&ctx->override_ctx, &ov_in, &rule_in.override_status);
    for (i = 0; i < RULE_COUNT; i++)
    {
        if (i != (int)RANK_DRIVER)
        {
            RULE_TABLE[i](&rule_in, &arb->rules[i]);
        }
    }
    arb->state = state;
    arb->previous[DOOR_LEFT] = ctx->previous[DOOR_LEFT];
    arb->previous[DOOR_RIGHT] = ctx->previous[DOOR_RIGHT];
    arb->override_status = rule_in.override_status;
    return STATUS_OK;
}

/**
 * @brief 좌 값이 NONE이 아니면 좌, 아니면 우 값을 요약 값으로 돌려준다.
 * @param[in] left   좌 도어 이유
 * @param[in] right  우 도어 이유
 * @return 요약 이유
 * @trace SRS-F-022, SRS-F-025
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
static ReasonCode_t Control_Summary(ReasonCode_t left, ReasonCode_t right)
{
    ReasonCode_t summary = right;

    if (left != REASON_NONE)
    {
        summary = left;
    }
    return summary;
}

/**
 * @brief 성공한 주기의 결과를 출력에 채우고 컨텍스트를 갱신한다.
 * @param[inout] ctx    컨텍스트
 * @param[in]    dec    중재 결과
 * @param[in]    arb    중재 입력
 * @param[in]    state  state
 * @param[in]    warn   경고코드
 * @param[in]    snap   검증된 입력
 * @param[out]   out    최종 결과
 * @return STATUS_OK
 * @trace DD-F-028
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
static Status_t Control_Commit(Control_t *ctx, const Decision_t *dec, const ArbiterInput_t *arb, State_t state,
                                int warn, const Snapshot_t *snap, CycleResult_t *out)
{
    int d;

    for (d = 0; d < DOOR_COUNT; d++)
    {
        out->output[d] = dec->output[d];
        out->priority_reason[d] = dec->priority_reason[d];
        out->reason_code[d] = dec->reason_code[d];
        ctx->previous[d] = dec->output[d];
        ctx->prev_suppressed[d] = arb->rules[RANK_APPROACH].suppressed[d];
    }
    out->priority_reason_summary = Control_Summary(dec->priority_reason[DOOR_LEFT], dec->priority_reason[DOOR_RIGHT]);
    out->reason_code_summary = Control_Summary(dec->reason_code[DOOR_LEFT], dec->reason_code[DOOR_RIGHT]);
    out->state = state;
    out->warning_code = warn;
    out->input_validity = Control_InputValidity(snap);
    ctx->cycle += 1U;
    return STATUS_OK;
}

/**
 * @brief 검증, 신선도, state, 규칙, 중재를 순서대로 실행한다.
 * @param[inout] ctx  컨텍스트
 * @param[in]    raw  원시 입력
 * @param[out]   out  최종 결과
 * @return STATUS_OK 또는 하위 오류
 * @trace DD-F-026
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
static Status_t Control_Run(Control_t *ctx, const RawInput_t *raw, CycleResult_t *out)
{
    Snapshot_t snap;
    ArbiterInput_t arb;
    Decision_t dec;
    State_t state = STATE_NORMAL;
    int warn = 0;
    Status_t status;

    /* 널이 아닌 인자로 호출하는 하위 함수는 오류를 반환하지 않으므로 신선도 검사만 오류를 전파한다. */
    Validator_Normalize(raw, &snap);
    status = Freshness_Apply(&ctx->freshness, &snap);
    if (status == STATUS_OK)
    {
        State_Determine(&snap, &state, &warn);
        Control_EvaluateRules(ctx, &snap, state, &arb);
        Arbiter_Resolve(&arb, &dec);
        status = Control_Commit(ctx, &dec, &arb, state, warn, &snap, out);
    }
    return status;
}

Status_t Control_Init(Control_t *ctx)
{
    Status_t result = STATUS_ERR_PARAM;

    if (ctx != NULL)
    {
        Control_t cleared = {0};

        *ctx = cleared;
        Freshness_Init(&ctx->freshness);
        Override_Init(&ctx->override_ctx);
        ctx->previous[DOOR_LEFT] = OUTPUT_RELEASE;
        ctx->previous[DOOR_RIGHT] = OUTPUT_RELEASE;
        ctx->initialized = 1;
        result = STATUS_OK;
    }
    return result;
}

Status_t Control_Step(Control_t *ctx, const RawInput_t *raw, CycleResult_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((ctx != NULL) && (raw != NULL) && (out != NULL))
    {
        result = (ctx->initialized == 1) ? Control_Run(ctx, raw, out) : STATUS_ERR_STATE;
    }
    return result;
}
