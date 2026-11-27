/**
 * @file    input_validator_unit_test.cpp
 * @brief   InputValidator(DD-M-001) 단위 테스트.
 * @trace   DD-F-001~005, SSR-007, SRS-F-001, SRS-L-007, SRS-C-003
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
#include <gtest/gtest.h>

#include <cmath>

#include "unit_helpers.h"

using namespace uh;

namespace
{

Snapshot_t Normalize(const RawInput_t &raw)
{
    Snapshot_t snap{};
    EXPECT_EQ(STATUS_OK, Validator_Normalize(&raw, &snap));
    return snap;
}

/** 목적: 널 포인터 거절. 입력: raw 또는 snap이 NULL. 기대: STATUS_ERR_PARAM. 추적: UT-001 -> DD-F-001. 기법: 오류 추정 */
TEST(InputValidatorUnit, Normalize_NullPointers_ReturnsParamError)
{
    RawInput_t raw = NormalRaw(0.0);
    Snapshot_t snap{};
    EXPECT_EQ(STATUS_ERR_PARAM, Validator_Normalize(nullptr, &snap));
    EXPECT_EQ(STATUS_ERR_PARAM, Validator_Normalize(&raw, nullptr));
}

/** 목적: 정상 입력이 모두 VALID로 정규화되고 시각이 복사된다. 입력: NormalRaw(2.5). 기대: 모든 validity VALID, timestamp 2.5. 추적: UT-002 -> DD-F-001, SRS-Q-003. 기법: 동치 클래스 */
TEST(InputValidatorUnit, Normalize_NormalInput_AllValid)
{
    Snapshot_t s = Normalize(NormalRaw(2.5));
    EXPECT_DOUBLE_EQ(2.5, s.timestamp_s);
    EXPECT_EQ(VALIDITY_VALID, s.speed.validity);
    EXPECT_EQ(VALIDITY_VALID, s.gear.validity);
    EXPECT_EQ(VALIDITY_VALID, s.source_timestamp.validity);
    EXPECT_EQ(VALIDITY_VALID, s.crash.validity);
    EXPECT_EQ(VALIDITY_VALID, s.approach[DOOR_LEFT].validity);
    EXPECT_EQ(VALIDITY_VALID, s.approach[DOOR_RIGHT].validity);
    EXPECT_EQ(VALIDITY_VALID, s.fire.validity);
    EXPECT_EQ(VALIDITY_VALID, s.overtemp.validity);
    EXPECT_EQ(VALIDITY_VALID, s.adult.validity);
    EXPECT_EQ(VALIDITY_VALID, s.isofix[DOOR_LEFT].validity);
    EXPECT_EQ(VALIDITY_VALID, s.isofix[DOOR_RIGHT].validity);
    EXPECT_EQ(VALIDITY_VALID, s.ignition.validity);
    EXPECT_EQ(VALIDITY_VALID, s.sensor_fault.validity);
    EXPECT_EQ(0, s.cmd.present);
    EXPECT_EQ(VALIDITY_INVALID, s.cmd.validity);
}

/** 목적: 불리언 값 집합 검사. 입력: fire=0, 1(유효), 2, -1(무효). 기대: 0/1만 VALID, 무효는 값 0. 추적: UT-003 -> DD-F-002, SSR-007. 기법: 경계값, 동치 클래스 */
TEST(InputValidatorUnit, Flag_BooleanValues_OnlyZeroAndOneValid)
{
    RawInput_t raw = NormalRaw(0.0);
    raw.fire = Bool(0);
    EXPECT_EQ(VALIDITY_VALID, Normalize(raw).fire.validity);
    raw.fire = Bool(1);
    Snapshot_t s1 = Normalize(raw);
    EXPECT_EQ(VALIDITY_VALID, s1.fire.validity);
    EXPECT_EQ(1, s1.fire.value);
    raw.fire = Bool(2);
    Snapshot_t s2 = Normalize(raw);
    EXPECT_EQ(VALIDITY_INVALID, s2.fire.validity);
    EXPECT_EQ(0, s2.fire.value);
    raw.fire = Bool(-1);
    EXPECT_EQ(VALIDITY_INVALID, Normalize(raw).fire.validity);
}

/** 목적: 불리언 필드에 다른 종류의 값이 오면 무효. 입력: 누락, 실수, 열거, 형식 오류. 기대: INVALID. 추적: UT-004 -> DD-F-002, SSR-007, SRS-C-003. 기법: 동치 클래스 */
TEST(InputValidatorUnit, Flag_WrongKind_Invalid)
{
    RawInput_t raw = NormalRaw(0.0);
    const RawKind_t kinds[] = {RAW_ABSENT, RAW_NUMBER, RAW_ENUM, RAW_OTHER};
    for (RawKind_t k : kinds)
    {
        raw.adult = Kind(k);
        raw.adult.boolean = 1; // 종류가 다르면 값이 맞아도 무효여야 한다
        EXPECT_EQ(VALIDITY_INVALID, Normalize(raw).adult.validity) << k;
    }
}

/** 목적: 차속 범위 경계(SRS-L-007). 입력: -0.1, 0.0, 300.0, 300.1, NaN, 무한대. 기대: 0.0과 300.0만 VALID. 추적: UT-005 -> DD-F-003, SRS-L-007. 기법: 경계값 분석 */
TEST(InputValidatorUnit, Number_SpeedBoundaries_ValidOnlyInRange)
{
    RawInput_t raw = NormalRaw(0.0);
    const struct
    {
        double v;
        Validity_t expect;
    } cases[] = {{-0.1, VALIDITY_INVALID}, {0.0, VALIDITY_VALID},   {150.0, VALIDITY_VALID},
                 {300.0, VALIDITY_VALID},  {300.1, VALIDITY_INVALID}, {std::nan(""), VALIDITY_INVALID},
                 {INFINITY, VALIDITY_INVALID}};
    for (const auto &c : cases)
    {
        raw.speed = Num(c.v);
        Snapshot_t s = Normalize(raw);
        EXPECT_EQ(c.expect, s.speed.validity) << c.v;
        if (c.expect == VALIDITY_VALID)
        {
            EXPECT_DOUBLE_EQ(c.v, s.speed.value);
        }
    }
}

/** 목적: 차속 필드의 종류 오류. 입력: 누락, 불리언, 열거, 형식 오류. 기대: INVALID. 추적: UT-006 -> DD-F-003, SSR-007. 기법: 동치 클래스 */
TEST(InputValidatorUnit, Number_WrongKind_Invalid)
{
    RawInput_t raw = NormalRaw(0.0);
    const RawKind_t kinds[] = {RAW_ABSENT, RAW_BOOLEAN, RAW_ENUM, RAW_OTHER};
    for (RawKind_t k : kinds)
    {
        raw.speed = Kind(k);
        raw.speed.number = 10.0;
        EXPECT_EQ(VALIDITY_INVALID, Normalize(raw).speed.validity) << k;
    }
}

/** 목적: 원천 시각의 범위. 입력: -0.001(무효), 0.0, 1e12(유효), 1e12+1e6(무효). 기대: 범위 안만 VALID. 추적: UT-007 -> DD-F-003. 기법: 경계값 분석 */
TEST(InputValidatorUnit, Number_SourceTimestampRange_ValidOnlyNonNegative)
{
    RawInput_t raw = NormalRaw(0.0);
    raw.source_timestamp = Num(-0.001);
    EXPECT_EQ(VALIDITY_INVALID, Normalize(raw).source_timestamp.validity);
    raw.source_timestamp = Num(0.0);
    EXPECT_EQ(VALIDITY_VALID, Normalize(raw).source_timestamp.validity);
    raw.source_timestamp = Num(1.0e12);
    EXPECT_EQ(VALIDITY_VALID, Normalize(raw).source_timestamp.validity);
    raw.source_timestamp = Num(1.0e12 + 1.0e6);
    EXPECT_EQ(VALIDITY_INVALID, Normalize(raw).source_timestamp.validity);
}

/** 목적: gear와 crash 값 집합. 입력: gear -1, 0, 3, 4 / crash -1, 0, 2, 3. 기대: 집합 안만 VALID. 추적: UT-008 -> DD-F-004, SSR-007. 기법: 경계값 분석 */
TEST(InputValidatorUnit, Enum_GearAndCrashBoundaries_ValidOnlyInSet)
{
    RawInput_t raw = NormalRaw(0.0);
    const struct
    {
        int id;
        Validity_t gear;
        Validity_t crash;
    } cases[] = {{-1, VALIDITY_INVALID, VALIDITY_INVALID}, {0, VALIDITY_VALID, VALIDITY_VALID},
                 {2, VALIDITY_VALID, VALIDITY_VALID},      {3, VALIDITY_VALID, VALIDITY_INVALID},
                 {4, VALIDITY_INVALID, VALIDITY_INVALID}};
    for (const auto &c : cases)
    {
        raw.gear = Enum(c.id);
        raw.crash = Enum(c.id);
        Snapshot_t s = Normalize(raw);
        EXPECT_EQ(c.gear, s.gear.validity) << c.id;
        EXPECT_EQ(c.crash, s.crash.validity) << c.id;
    }
}

/** 목적: 열거 필드의 종류 오류. 입력: 숫자/불리언/누락/형식 오류. 기대: INVALID. 추적: UT-009 -> DD-F-004. 기법: 동치 클래스 */
TEST(InputValidatorUnit, Enum_WrongKind_Invalid)
{
    RawInput_t raw = NormalRaw(0.0);
    const RawKind_t kinds[] = {RAW_ABSENT, RAW_NUMBER, RAW_BOOLEAN, RAW_OTHER};
    for (RawKind_t k : kinds)
    {
        raw.gear = Kind(k);
        EXPECT_EQ(VALIDITY_INVALID, Normalize(raw).gear.validity) << k;
    }
}

/** 목적: 명령이 없으면 present 0. 입력: 세 명령 필드 누락. 기대: present 0, INVALID, 지정 도어 없음. 추적: UT-010 -> DD-F-005. 기법: 동치 클래스 */
TEST(InputValidatorUnit, Command_AllAbsent_NotPresent)
{
    Snapshot_t s = Normalize(NormalRaw(0.0));
    EXPECT_EQ(0, s.cmd.present);
    EXPECT_EQ(VALIDITY_INVALID, s.cmd.validity);
    EXPECT_EQ(0, s.cmd.target[DOOR_LEFT]);
    EXPECT_EQ(0, s.cmd.target[DOOR_RIGHT]);
}

/** 목적: 명령 필드가 하나만 있어도 present 1 (side/action/source 각각). 입력: 한 필드만 형식 오류로 존재. 기대: present 1, INVALID. 추적: UT-011 -> DD-F-005, SRS-F-001. 기법: 조건 독립성(MC/DC) */
TEST(InputValidatorUnit, Command_SingleFieldPresent_PresentButInvalid)
{
    RawInput_t raw = NormalRaw(0.0);
    raw.cmd_side = Kind(RAW_OTHER);
    EXPECT_EQ(1, Normalize(raw).cmd.present);
    EXPECT_EQ(VALIDITY_INVALID, Normalize(raw).cmd.validity);

    raw = NormalRaw(0.0);
    raw.cmd_action = Kind(RAW_OTHER);
    EXPECT_EQ(1, Normalize(raw).cmd.present);

    raw = NormalRaw(0.0);
    raw.cmd_source = Kind(RAW_OTHER);
    EXPECT_EQ(1, Normalize(raw).cmd.present);
}

/** 목적: 명령 세 필드 중 하나만 무효여도 명령 전체가 무효. 입력: 두 필드 유효 + 한 필드 범위 밖. 기대: present 1, INVALID, 지정 도어 없음. 추적: UT-012 -> DD-F-005, SSR-007. 기법: 조건 독립성(MC/DC) */
TEST(InputValidatorUnit, Command_OneFieldOutOfRange_Invalid)
{
    for (int bad = 0; bad < 3; bad++)
    {
        RawInput_t raw = NormalRaw(0.0);
        SetCommand(&raw, SIDE_ALL, ACTION_LOCK, SOURCE_AVN);
        if (bad == 0)
        {
            raw.cmd_side = Enum(SIDE_MAX + 1);
        }
        else if (bad == 1)
        {
            raw.cmd_action = Enum(ACTION_MAX + 1);
        }
        else
        {
            raw.cmd_source = Enum(SOURCE_MAX + 1);
        }
        Snapshot_t s = Normalize(raw);
        EXPECT_EQ(1, s.cmd.present) << bad;
        EXPECT_EQ(VALIDITY_INVALID, s.cmd.validity) << bad;
        EXPECT_EQ(0, s.cmd.target[DOOR_LEFT]) << bad;
    }
}

/** 목적: side에 따른 지정 도어. 입력: side=ALL/LEFT/RIGHT, 4개 source, 2개 action 전체 조합. 기대: ALL은 양쪽, LEFT는 좌만, RIGHT는 우만 지정. 추적: UT-013 -> DD-F-005, SRS-F-001, SRS-F-002. 기법: 전수 조합 (4x3x2=24) */
TEST(InputValidatorUnit, Command_AllCombinations_TargetsMatchSide)
{
    int count = 0;
    for (int source = 0; source <= SOURCE_MAX; source++)
    {
        for (int side = 0; side <= SIDE_MAX; side++)
        {
            for (int action = 0; action <= ACTION_MAX; action++)
            {
                RawInput_t raw = NormalRaw(0.0);
                SetCommand(&raw, side, action, source);
                Snapshot_t s = Normalize(raw);
                EXPECT_EQ(VALIDITY_VALID, s.cmd.validity);
                EXPECT_EQ(side, s.cmd.side);
                EXPECT_EQ(action, s.cmd.action);
                EXPECT_EQ(source, s.cmd.source);
                EXPECT_EQ((side == SIDE_ALL) || (side == SIDE_LEFT) ? 1 : 0, s.cmd.target[DOOR_LEFT]);
                EXPECT_EQ((side == SIDE_ALL) || (side == SIDE_RIGHT) ? 1 : 0, s.cmd.target[DOOR_RIGHT]);
                count++;
            }
        }
    }
    EXPECT_EQ(24, count);
}

} // namespace
