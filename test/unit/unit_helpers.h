/**
 * @file    unit_helpers.h
 * @brief   단위 테스트 공용 입력 생성 도우미 (제품 코드가 아님).
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
#ifndef UNIT_HELPERS_H
#define UNIT_HELPERS_H

extern "C" {
#include "common/params.h"
#include "common/types.h"
#include "control/approach_risk_rule.h"
#include "control/arbiter.h"
#include "control/auto_lock_rule.h"
#include "control/control_cycle.h"
#include "control/crash_rule.h"
#include "control/driver_command_rule.h"
#include "control/emergency_rule.h"
#include "control/freshness_monitor.h"
#include "control/ignition_rule.h"
#include "control/input_validator.h"
#include "control/isofix_rule.h"
#include "control/override_manager.h"
#include "control/state_manager.h"
}

namespace uh
{

inline RawValue_t Num(double v)
{
    RawValue_t r{};
    r.kind = RAW_NUMBER;
    r.number = v;
    return r;
}

inline RawValue_t Bool(int v)
{
    RawValue_t r{};
    r.kind = RAW_BOOLEAN;
    r.boolean = v;
    return r;
}

inline RawValue_t Enum(int v)
{
    RawValue_t r{};
    r.kind = RAW_ENUM;
    r.enum_id = v;
    return r;
}

inline RawValue_t Kind(RawKind_t k)
{
    RawValue_t r{};
    r.kind = k;
    return r;
}

/** 모든 필드가 유효한 정상 원시 입력 (명령 없음) */
inline RawInput_t NormalRaw(double t)
{
    RawInput_t in{};
    in.timestamp_s = t;
    in.speed = Num(0.0);
    in.gear = Enum(GEAR_P);
    in.source_timestamp = Num(t);
    in.crash = Enum(CRASH_NONE);
    in.approach[DOOR_LEFT] = Bool(0);
    in.approach[DOOR_RIGHT] = Bool(0);
    in.fire = Bool(0);
    in.overtemp = Bool(0);
    in.adult = Bool(0);
    in.isofix[DOOR_LEFT] = Bool(0);
    in.isofix[DOOR_RIGHT] = Bool(0);
    in.ignition = Bool(1);
    in.sensor_fault = Bool(0);
    return in;
}

inline void SetCommand(RawInput_t *in, int side, int action, int source = SOURCE_AVN)
{
    in->cmd_side = Enum(side);
    in->cmd_action = Enum(action);
    in->cmd_source = Enum(source);
}

inline Flag_t F(int value, Validity_t v = VALIDITY_VALID)
{
    Flag_t f{};
    f.value = value;
    f.validity = v;
    return f;
}

/** 모든 필드가 유효한 정상 스냅샷 (명령 없음: present 0, INVALID) */
inline Snapshot_t NormalSnapshot()
{
    Snapshot_t s{};
    s.timestamp_s = 1.0;
    s.speed = {0.0, VALIDITY_VALID};
    s.gear = {GEAR_P, VALIDITY_VALID};
    s.source_timestamp = {1.0, VALIDITY_VALID};
    s.crash = {CRASH_NONE, VALIDITY_VALID};
    s.approach[DOOR_LEFT] = F(0);
    s.approach[DOOR_RIGHT] = F(0);
    s.fire = F(0);
    s.overtemp = F(0);
    s.adult = F(0);
    s.isofix[DOOR_LEFT] = F(0);
    s.isofix[DOOR_RIGHT] = F(0);
    s.ignition = F(1);
    s.sensor_fault = F(0);
    return s;
}

/** 유효한 명령을 스냅샷에 설정한다 */
inline void SetValidCommand(Snapshot_t *s, int side, int action)
{
    s->cmd.present = 1;
    s->cmd.validity = VALIDITY_VALID;
    s->cmd.side = side;
    s->cmd.action = action;
    s->cmd.source = SOURCE_AVN;
    s->cmd.target[DOOR_LEFT] = ((side == SIDE_ALL) || (side == SIDE_LEFT)) ? 1 : 0;
    s->cmd.target[DOOR_RIGHT] = ((side == SIDE_ALL) || (side == SIDE_RIGHT)) ? 1 : 0;
}

inline RuleInput_t MakeRuleInput(const Snapshot_t *s, State_t st = STATE_NORMAL)
{
    RuleInput_t in{};
    in.snapshot = s;
    in.state = st;
    in.previous[DOOR_LEFT] = OUTPUT_RELEASE;
    in.previous[DOOR_RIGHT] = OUTPUT_RELEASE;
    return in;
}

/** 쓰레기 값으로 채운 결과 (규칙이 먼저 초기화하는지 확인하기 위함) */
inline RuleResult_t DirtyResult()
{
    RuleResult_t r{};
    for (int d = 0; d < DOOR_COUNT; d++)
    {
        r.active[d] = 1;
        r.effect[d] = OUTPUT_LOCK;
        r.reason[d] = REASON_AUTO_LOCK;
        r.info_reason[d] = REASON_FIRE;
        r.unlock_allowed[d] = 1;
        r.lock_requested[d] = 1;
        r.suppressed[d] = 1;
    }
    return r;
}

} // namespace uh

#endif // UNIT_HELPERS_H
