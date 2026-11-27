/**
 * @file    control_system_test.cpp
 * @brief   제어 코어 시스템 테스트 (SWE.6): SRS/SSR 합격 기준을 블랙박스(원시 입력 → 최종 결과)로 검증한다.
 * @trace   SRS-F-001~004, 007, 014~017, 027~031, SRS-Q-001, SRS-L-004, 005, 007, SRS-C-003, 005, SSR-001~016
 *
 * 각 테스트는 RecordProperty("requirements", ...)로 검증하는 요구사항 ID를 기록하고,
 * scripts/ci/req_coverage.py가 JUnit XML에서 읽어 요구사항 커버리지를 계산한다.
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

#include <cstdint>
#include <cstring>
#include <string>

extern "C" {
#include "common/params.h"
#include "control/control_cycle.h"
}

namespace
{

// ------------------------------------------------------------------ 입력 도우미
RawValue_t Num(double v) { RawValue_t r{}; r.kind = RAW_NUMBER; r.number = v; return r; }
RawValue_t Bool(int v) { RawValue_t r{}; r.kind = RAW_BOOLEAN; r.boolean = v; return r; }
RawValue_t Enum(int v) { RawValue_t r{}; r.kind = RAW_ENUM; r.enum_id = v; return r; }
RawValue_t Bad() { RawValue_t r{}; r.kind = RAW_OTHER; return r; }

/** 주기 번호에 해당하는 논리 시각(10 ms 단위)의 정상 입력 */
RawInput_t Raw(int cycle)
{
    RawInput_t in{};
    in.timestamp_s = cycle * CYCLE_PERIOD_S;
    in.speed = Num(0.0);
    in.gear = Enum(GEAR_P);
    in.source_timestamp = Num(in.timestamp_s);
    in.crash = Enum(CRASH_NONE);
    in.approach[0] = Bool(0);
    in.approach[1] = Bool(0);
    in.fire = Bool(0);
    in.overtemp = Bool(0);
    in.adult = Bool(0);
    in.isofix[0] = Bool(0);
    in.isofix[1] = Bool(0);
    in.ignition = Bool(1);
    in.sensor_fault = Bool(0);
    return in;
}

void Cmd(RawInput_t *in, int side, int action, int source = SOURCE_VOICE)
{
    in->cmd_side = Enum(side);
    in->cmd_action = Enum(action);
    in->cmd_source = Enum(source);
}

class System : public ::testing::Test
{
protected:
    void SetUp() override { ASSERT_EQ(STATUS_OK, Control_Init(&ctx_)); }

    CycleResult_t Step(const RawInput_t &in)
    {
        CycleResult_t out{};
        EXPECT_EQ(STATUS_OK, Control_Step(&ctx_, &in, &out));
        return out;
    }

    /** 속도 0에서 lock 명령 한 주기로 양쪽을 LOCK 상태로 만든다 */
    void PrimeLock(int cycle)
    {
        RawInput_t in = Raw(cycle);
        Cmd(&in, SIDE_ALL, ACTION_LOCK);
        CycleResult_t r = Step(in);
        ASSERT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
        ASSERT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    }

    Control_t ctx_{};
};

// ------------------------------------------------------------------ SHA-256 (SRS-Q-001 공개 결과 해시)
struct Sha256
{
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    uint8_t buf[64];
    size_t len = 0;
    uint64_t total = 0;

    static uint32_t Rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

    void Block(const uint8_t *p)
    {
        static const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
        uint32_t w[64];
        for (int i = 0; i < 16; i++)
        {
            w[i] = (uint32_t(p[4 * i]) << 24) | (uint32_t(p[4 * i + 1]) << 16) | (uint32_t(p[4 * i + 2]) << 8) |
                   uint32_t(p[4 * i + 3]);
        }
        for (int i = 16; i < 64; i++)
        {
            uint32_t s0 = Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; i++)
        {
            uint32_t t1 = hh + (Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25)) + ((e & f) ^ (~e & g)) + k[i] + w[i];
            uint32_t t2 = (Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    void Update(const void *data, size_t n)
    {
        const uint8_t *p = static_cast<const uint8_t *>(data);
        total += n;
        while (n > 0)
        {
            size_t take = std::min(n, sizeof(buf) - len);
            std::memcpy(buf + len, p, take);
            len += take;
            p += take;
            n -= take;
            if (len == 64)
            {
                Block(buf);
                len = 0;
            }
        }
    }

    std::string Final()
    {
        uint64_t bits = total * 8;
        uint8_t pad = 0x80;
        Update(&pad, 1);
        uint8_t zero = 0;
        while (len != 56)
        {
            Update(&zero, 1);
        }
        uint8_t be[8];
        for (int i = 0; i < 8; i++)
        {
            be[i] = static_cast<uint8_t>(bits >> (56 - 8 * i));
        }
        Update(be, 8);
        char out[65];
        for (int i = 0; i < 8; i++)
        {
            std::snprintf(out + 8 * i, 9, "%08x", h[i]);
        }
        return std::string(out);
    }
};

std::string PublicResultHash(const CycleResult_t &r, Sha256 *sha)
{
    sha->Update(&r.output[0], sizeof(r.output));
    sha->Update(&r.state, sizeof(r.state));
    sha->Update(&r.priority_reason[0], sizeof(r.priority_reason));
    sha->Update(&r.reason_code[0], sizeof(r.reason_code));
    sha->Update(&r.input_validity, sizeof(r.input_validity));
    return std::string();
}

// ================================================================== 테스트

/** ST-001 SRS-F-001/002: 4 source x 3 side x 2 action = 24 조합이 선택 도어에만 적용되고 CMD_APPLIED를 기록한다 (속도 0). */
TEST_F(System, DriverCommands_AllCombinations_AppliedToSelectedDoorsOnly)
{
    RecordProperty("requirements", "SRS-F-001,SRS-F-002");
    int count = 0;
    for (int source = 0; source <= SOURCE_MAX; source++)
    {
        for (int side = 0; side <= SIDE_MAX; side++)
        {
            for (int action = 0; action <= ACTION_MAX; action++)
            {
                ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
                if (action == ACTION_UNLOCK)
                {
                    PrimeLock(0);
                }
                RawInput_t in = Raw(1);
                Cmd(&in, side, action, source);
                CycleResult_t r = Step(in);
                for (int d = 0; d < DOOR_COUNT; d++)
                {
                    bool targeted = (side == SIDE_ALL) || (side == d);
                    Output_t before = (action == ACTION_UNLOCK) ? OUTPUT_LOCK : OUTPUT_RELEASE;
                    Output_t after = (action == ACTION_LOCK) ? OUTPUT_LOCK : OUTPUT_RELEASE;
                    EXPECT_EQ(targeted ? after : before, r.output[d]) << source << side << action << d;
                    if (targeted)
                    {
                        EXPECT_EQ(REASON_CMD_APPLIED, r.reason_code[d]);
                    }
                }
                count++;
            }
        }
    }
    EXPECT_EQ(24, count);
}

/** ST-002 SRS-F-003/L-004: unlock은 2.9 km/h에서 적용되고 3.0 km/h에서는 적용되지 않으며 CMD_REJECTED_SPEED가 기록된다. */
TEST_F(System, Unlock_SpeedBoundary_AppliedBelowThresholdOnly)
{
    RecordProperty("requirements", "SRS-F-003,SRS-L-004");
    PrimeLock(0);
    RawInput_t slow = Raw(1);
    slow.speed = Num(2.9);
    Cmd(&slow, SIDE_ALL, ACTION_UNLOCK);
    CycleResult_t r = Step(slow);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);

    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    PrimeLock(0);
    RawInput_t edge = Raw(1);
    edge.speed = Num(3.0);
    Cmd(&edge, SIDE_ALL, ACTION_UNLOCK);
    r = Step(edge);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_CMD_REJECTED_SPEED, r.reason_code[DOOR_LEFT]);
    EXPECT_EQ(REASON_AUTO_LOCK, r.priority_reason[DOOR_LEFT]);
}

/** ST-003 SRS-F-004/L-004: 2.9 km/h는 동작하지 않고 3.0 km/h부터 100주기 동안 양쪽 LOCK+AUTO_LOCK. */
TEST_F(System, AutoLock_FromThreshold_LocksBothDoors)
{
    RecordProperty("requirements", "SRS-F-004,SRS-L-004");
    RawInput_t slow = Raw(0);
    slow.speed = Num(2.9);
    CycleResult_t r = Step(slow);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    for (int c = 1; c <= 100; c++)
    {
        RawInput_t in = Raw(c);
        in.speed = Num(3.0);
        r = Step(in);
        EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]) << c;
        EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]) << c;
        EXPECT_EQ(REASON_AUTO_LOCK, r.reason_code[DOOR_LEFT]) << c;
    }
}

/** ST-004 SRS-F-007/029/030, SRS-L-005, SSR-005: override 성립, 허용 주기 경계, 지속, 종료 조건. */
TEST_F(System, Override_Lifecycle_WindowAndEndConditions)
{
    RecordProperty("requirements", "SRS-F-007,SRS-F-029,SRS-F-030,SRS-L-005,SSR-005");
    auto first = [&]() {
        RawInput_t in = Raw(0);
        in.approach[DOOR_LEFT] = Bool(1);
        Cmd(&in, SIDE_LEFT, ACTION_UNLOCK);
        CycleResult_t r = Step(in);
        EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
        EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, r.reason_code[DOOR_LEFT]);
    };
    auto hold = [&](int from, int to) {
        for (int c = from; c < to; c++)
        {
            RawInput_t in = Raw(c);
            in.approach[DOOR_LEFT] = Bool(1);
            CycleResult_t r = Step(in);
            ASSERT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]) << c;
        }
    };

    // 1,000주기 이하: override 성립
    first();
    hold(1, 1000);
    RawInput_t re = Raw(1000);
    re.approach[DOOR_LEFT] = Bool(1);
    Cmd(&re, SIDE_LEFT, ACTION_UNLOCK);
    CycleResult_t r = Step(re);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, r.priority_reason[DOOR_LEFT]);
    EXPECT_EQ(REASON_OVERRIDE, r.reason_code[DOOR_LEFT]);

    // 지속 100주기 + 반대쪽 도어는 영향 없음
    for (int c = 1001; c < 1101; c++)
    {
        RawInput_t in = Raw(c);
        in.approach[DOOR_LEFT] = Bool(1);
        r = Step(in);
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]) << c;
        EXPECT_EQ(REASON_OVERRIDE, r.reason_code[DOOR_LEFT]) << c;
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]) << c;
    }

    // 접근위험 해소: 종료 (출력 유지, NONE), 재상승 시 LOCK
    RawInput_t gone = Raw(1101);
    r = Step(gone);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_NONE, r.reason_code[DOOR_LEFT]);
    RawInput_t back = Raw(1102);
    back.approach[DOOR_LEFT] = Bool(1);
    r = Step(back);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);

    // 1,001주기 재입력: override 불가
    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    first();
    hold(1, 1001);
    RawInput_t late = Raw(1001);
    late.approach[DOOR_LEFT] = Bool(1);
    Cmd(&late, SIDE_LEFT, ACTION_UNLOCK);
    r = Step(late);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, r.reason_code[DOOR_LEFT]);

    // 속도 3.0 km/h: 종료되고 6.3에 따라 LOCK
    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    first();
    RawInput_t again = Raw(1);
    again.approach[DOOR_LEFT] = Bool(1);
    Cmd(&again, SIDE_LEFT, ACTION_UNLOCK);
    r = Step(again);
    ASSERT_EQ(REASON_OVERRIDE, r.reason_code[DOOR_LEFT]);
    RawInput_t fast = Raw(2);
    fast.approach[DOOR_LEFT] = Bool(1);
    fast.speed = Num(3.0);
    r = Step(fast);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);

    // 같은 도어 lock 명령: 종료하고 LOCK, 이후 접근위험이 있으면 LOCK 유지
    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    first();
    r = Step(again);
    ASSERT_EQ(REASON_OVERRIDE, r.reason_code[DOOR_LEFT]);
    RawInput_t lock = Raw(2);
    lock.approach[DOOR_LEFT] = Bool(1);
    Cmd(&lock, SIDE_LEFT, ACTION_LOCK);
    r = Step(lock);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_LOCK, r.reason_code[DOOR_LEFT]); // override 종료 후 접근위험(순위 4)이 명령(순위 6)보다 높다
    RawInput_t next = Raw(3);
    next.approach[DOOR_LEFT] = Bool(1);
    EXPECT_EQ(OUTPUT_LOCK, Step(next).output[DOOR_LEFT]);

    // ISOFIX가 있는 도어는 override로 풀리지 않는다
    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    RawInput_t iso = Raw(0);
    iso.approach[DOOR_LEFT] = Bool(1);
    iso.isofix[DOOR_LEFT] = Bool(1);
    Cmd(&iso, SIDE_LEFT, ACTION_UNLOCK);
    Step(iso);
    iso = Raw(1);
    iso.approach[DOOR_LEFT] = Bool(1);
    iso.isofix[DOOR_LEFT] = Bool(1);
    Cmd(&iso, SIDE_LEFT, ACTION_UNLOCK);
    r = Step(iso);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_ISOFIX_LOCK, r.priority_reason[DOOR_LEFT]);
}

/** ST-005 SRS-F-014: 화재/과온/성인 단독 입력은 다음 주기부터 입력이 TRUE인 동안 RELEASE (고속에서도), 해제되면 규칙대로 평가. */
TEST_F(System, EmergencyRelease_PerInput_HeldWhileTrueThenReevaluated)
{
    RecordProperty("requirements", "SRS-F-014");
    const ReasonCode_t reasons[] = {REASON_FIRE, REASON_OVERTEMP, REASON_ADULT_PRESENT};
    for (int which = 0; which < 3; which++)
    {
        ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
        PrimeLock(0);
        for (int c = 1; c <= 100; c++)
        {
            RawInput_t in = Raw(c);
            in.speed = Num(c <= 50 ? 0.0 : 50.0);
            (which == 0 ? in.fire : which == 1 ? in.overtemp : in.adult) = Bool(1);
            CycleResult_t r = Step(in);
            EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]) << which << " " << c;
            EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]) << which << " " << c;
            EXPECT_EQ(reasons[which], r.reason_code[DOOR_LEFT]) << which << " " << c;
        }
        RawInput_t end = Raw(101);
        end.speed = Num(50.0);
        CycleResult_t r = Step(end);
        EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]); // 속도 3 km/h 이상이면 자동 잠금
    }
}

/** ST-006 SRS-F-015: 한쪽 ISOFIX만 TRUE이면 그 쪽만 LOCK+ISOFIX_LOCK, 반대쪽은 유지. */
TEST_F(System, Isofix_OneSide_LocksOnlyThatSide)
{
    RecordProperty("requirements", "SRS-F-015");
    RawInput_t in = Raw(0);
    in.isofix[DOOR_RIGHT] = Bool(1);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_ISOFIX_LOCK, r.reason_code[DOOR_RIGHT]);
    in = Raw(1);
    in.isofix[DOOR_LEFT] = Bool(1);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]); // ISOFIX가 해제되어도 반대쪽 출력은 직전 값을 유지한다
}

/** ST-007 SRS-F-016/027/031: ignition off 동안 매 주기 RELEASE+OFF+IGNITION_OFF (다른 LOCK 입력과 명령에도), 복귀 다음 주기에 OFF 해제. */
TEST_F(System, IgnitionOff_HoldsReleaseAndRecovers)
{
    RecordProperty("requirements", "SRS-F-016,SRS-F-027,SRS-F-031");
    for (int c = 0; c <= 100; c++)
    {
        RawInput_t in = Raw(c);
        in.ignition = Bool(0);
        in.isofix[DOOR_LEFT] = Bool(1);
        in.approach[DOOR_RIGHT] = Bool(1);
        in.speed = Num(3.0);
        Cmd(&in, SIDE_ALL, ACTION_LOCK);
        CycleResult_t r = Step(in);
        EXPECT_EQ(STATE_OFF, r.state) << c;
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]) << c;
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]) << c;
        EXPECT_EQ(REASON_IGNITION_OFF, r.reason_code[DOOR_LEFT]) << c;
    }
    CycleResult_t on = Step(Raw(101));
    EXPECT_EQ(STATE_NORMAL, on.state);
    EXPECT_EQ(OUTPUT_RELEASE, on.output[DOOR_LEFT]);
}

// ---- 우선순위 쌍 시험 (SRS-F-017/028)
struct RuleCase
{
    const char *name;
    ReasonCode_t reason;
    Output_t effect;
};

const RuleCase kRules[RULE_COUNT] = {
    {"crash", REASON_CRASH_CONFIRMED, OUTPUT_RELEASE},
    {"fire", REASON_FIRE, OUTPUT_RELEASE},
    {"ignition", REASON_IGNITION_OFF, OUTPUT_RELEASE},
    {"approach", REASON_APPROACH_RISK_LOCK, OUTPUT_LOCK},
    {"isofix", REASON_ISOFIX_LOCK, OUTPUT_LOCK},
    {"driver", REASON_CMD_APPLIED, OUTPUT_RELEASE},
    {"autolock", REASON_AUTO_LOCK, OUTPUT_LOCK},
};

void Apply(RawInput_t *in, int rank, bool lock_command)
{
    switch (rank)
    {
    case 0: in->crash = Enum(CRASH_CONFIRMED); break;
    case 1: in->fire = Bool(1); break;
    case 2: in->ignition = Bool(0); break;
    case 3: in->approach[DOOR_LEFT] = Bool(1); break;
    case 4: in->isofix[DOOR_LEFT] = Bool(1); break;
    case 5: Cmd(in, SIDE_LEFT, lock_command ? ACTION_LOCK : ACTION_UNLOCK); break;
    default: in->speed = Num(3.0); break;
    }
}

/** ST-008 SRS-F-017/028: 규칙 7개 중 2개를 동시에 활성화한 21쌍 중 OFF와 순위 4~7의 4쌍을 제외한 17쌍에서 상위 규칙의 효과와 이유가 적용된다. */
TEST_F(System, PriorityPairs_Seventeen_HigherRuleWins)
{
    RecordProperty("requirements", "SRS-F-017,SRS-F-028");
    int tested = 0;
    for (int i = 0; i < RULE_COUNT; i++)
    {
        for (int j = i + 1; j < RULE_COUNT; j++)
        {
            if ((i == 2) && (j >= 3))
            {
                continue; // OFF 중에는 순위 4~7을 적용하지 않는다 (SRS-F-031 시험에서 검증)
            }
            ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
            bool lock_command = (i == 5) && (j == 6);
            Output_t winner_effect = (i == 5 && lock_command) ? OUTPUT_LOCK : kRules[i].effect;
            ReasonCode_t winner_reason = kRules[i].reason;
            if (i == 5)
            {
                winner_effect = lock_command ? OUTPUT_LOCK : OUTPUT_RELEASE;
            }
            // 직전 출력을 승자의 효과와 반대로 만든다
            if (winner_effect == OUTPUT_RELEASE)
            {
                PrimeLock(0);
            }
            RawInput_t in = Raw(1);
            Apply(&in, i, lock_command);
            Apply(&in, j, lock_command);
            CycleResult_t r = Step(in);
            EXPECT_EQ(winner_effect, r.output[DOOR_LEFT]) << kRules[i].name << "," << kRules[j].name;
            if ((i == 3) && (j == 5))
            {
                EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, r.priority_reason[DOOR_LEFT]);
            }
            else
            {
                EXPECT_EQ(winner_reason, r.priority_reason[DOOR_LEFT]) << kRules[i].name << "," << kRules[j].name;
            }
            tested++;
        }
    }
    EXPECT_EQ(17, tested);
}

/** ST-009 SSR-001/002/003/013: 충돌 CONFIRMED는 같은 주기에 RELEASE(30주기 이내)하고 유지되며 FAULT/DEGRADED에서도 적용, PENDING은 출력을 바꾸지 않는다. */
TEST_F(System, Crash_ConfirmedReleasesAndPendingDoesNot)
{
    RecordProperty("requirements", "SSR-001,SSR-002,SSR-003,SSR-013");
    PrimeLock(0);
    for (int c = 1; c <= 100; c++)
    {
        RawInput_t in = Raw(c);
        in.crash = Enum(CRASH_CONFIRMED);
        in.isofix[0] = Bool(1);
        Cmd(&in, SIDE_ALL, ACTION_LOCK);
        CycleResult_t r = Step(in);
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]) << c;
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]) << c;
        EXPECT_EQ(REASON_CRASH_CONFIRMED, r.reason_code[DOOR_RIGHT]) << c;
    }
    const int variants[2] = {0, 1};
    for (int v : variants)
    {
        ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
        PrimeLock(0);
        RawInput_t in = Raw(1);
        in.crash = Enum(CRASH_CONFIRMED);
        if (v == 0)
        {
            in.sensor_fault = Bool(1);
        }
        else
        {
            in.source_timestamp = Num(0.0);
            in.timestamp_s = 10.0;
        }
        CycleResult_t r = Step(in);
        EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]) << v;
    }
    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    PrimeLock(0);
    RawInput_t pending = Raw(1);
    pending.crash = Enum(CRASH_PENDING);
    CycleResult_t r = Step(pending);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_CRASH_PENDING, r.reason_code[DOOR_LEFT]);
}

/** ST-010 SSR-004/005/006: 접근위험은 해당 도어만 LOCK, unlock 억제, 좌우 독립. */
TEST_F(System, ApproachRisk_PerDoorLockAndSuppression)
{
    RecordProperty("requirements", "SSR-004,SSR-005,SSR-006");
    RawInput_t in = Raw(0);
    in.approach[DOOR_LEFT] = Bool(1);
    CycleResult_t r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
    EXPECT_EQ(REASON_APPROACH_RISK_LOCK, r.reason_code[DOOR_LEFT]);

    in = Raw(1);
    in.approach[DOOR_LEFT] = Bool(1);
    Cmd(&in, SIDE_ALL, ACTION_UNLOCK);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_APPROACH_RISK_SUPPRESSED, r.reason_code[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]); // 우측 unlock은 적용 (이미 RELEASE)

    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    PrimeLock(0);
    in = Raw(1);
    in.approach[DOOR_LEFT] = Bool(1);
    Cmd(&in, SIDE_ALL, ACTION_UNLOCK);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_RIGHT]);
}

/** ST-011 SSR-007/SRS-L-007/SRS-C-003: 범위, 형식, 누락, 값 집합 오류 입력은 평가에 쓰이지 않고 DEGRADED(또는 INPUT_INVALID)로 처리되며 gear 오류는 DEGRADED를 일으키지 않는다. */
TEST_F(System, InputValidation_BadInputs_RejectedBeforeEvaluation)
{
    RecordProperty("requirements", "SSR-007,SRS-L-007,SRS-C-003");
    struct Case
    {
        const char *name;
        RawValue_t RawInput_t::*field;
        RawValue_t value;
        bool degraded;
    };
    const Case cases[] = {
        {"speed -0.1", &RawInput_t::speed, Num(-0.1), true},
        {"speed 300.1", &RawInput_t::speed, Num(300.1), true},
        {"speed text", &RawInput_t::speed, Bad(), true},
        {"speed absent", &RawInput_t::speed, RawValue_t{}, true},
        {"speed 0.0 ok", &RawInput_t::speed, Num(0.0), false},
        {"speed 300.0 ok", &RawInput_t::speed, Num(300.0), false},
        {"crash 3", &RawInput_t::crash, Enum(3), true},
        {"fire 2", &RawInput_t::fire, Bool(2), true},
        {"adult number", &RawInput_t::adult, Num(1.0), true},
        {"ignition absent", &RawInput_t::ignition, RawValue_t{}, true},
        {"gear X", &RawInput_t::gear, Enum(9), false},
    };
    for (const Case &c : cases)
    {
        ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
        RawInput_t in = Raw(0);
        in.*(c.field) = c.value;
        CycleResult_t r = Step(in);
        EXPECT_EQ(c.degraded ? STATE_DEGRADED : STATE_NORMAL, r.state) << c.name;
    }
    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    RawInput_t badcmd = Raw(0);
    badcmd.cmd_side = Enum(7);
    badcmd.cmd_action = Enum(ACTION_LOCK);
    badcmd.cmd_source = Enum(SOURCE_AVN);
    CycleResult_t r = Step(badcmd);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    EXPECT_EQ(REASON_INPUT_INVALID, r.reason_code[DOOR_LEFT]);
}

/** ST-012 SSR-008/009: 차속 원천 시각 차이 200 ms(20주기)는 fresh, 210 ms(21주기)는 stale이고 같은 주기에 DEGRADED(100 ms 이내)이다. */
TEST_F(System, Freshness_Boundary_StaleAfter200ms)
{
    RecordProperty("requirements", "SSR-008,SSR-009");
    RawInput_t fresh = Raw(100);
    fresh.source_timestamp = Num(fresh.timestamp_s - 20 * CYCLE_PERIOD_S);
    EXPECT_EQ(STATE_NORMAL, Step(fresh).state);
    RawInput_t stale = Raw(101);
    stale.source_timestamp = Num(stale.timestamp_s - 21 * CYCLE_PERIOD_S);
    CycleResult_t r = Step(stale);
    EXPECT_EQ(STATE_DEGRADED, r.state);
    EXPECT_EQ(REASON_DEGRADED_STALE, r.reason_code[DOOR_LEFT]);
}

/** ST-013 SSR-010/011/012/014/015/016: FAULT/DEGRADED 상태의 명령 미적용, LOCK 규칙 적용, 경고코드, 복귀, 상태 우선순위 표. */
TEST_F(System, FaultAndDegraded_BehaviorAndRecovery)
{
    RecordProperty("requirements", "SSR-010,SSR-011,SSR-012,SSR-014,SSR-015,SSR-016");
    // FAULT 첫 주기
    RawInput_t in = Raw(0);
    in.sensor_fault = Bool(1);
    Cmd(&in, SIDE_ALL, ACTION_LOCK);
    CycleResult_t r = Step(in);
    EXPECT_EQ(STATE_FAULT, r.state);
    EXPECT_EQ(1, r.warning_code);
    EXPECT_EQ(REASON_SENSOR_FAULT, r.reason_code[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    // FAULT 지속 중 LOCK 규칙 적용
    in = Raw(1);
    in.sensor_fault = Bool(1);
    in.isofix[DOOR_LEFT] = Bool(1);
    in.approach[DOOR_RIGHT] = Bool(1);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_RIGHT]);
    in = Raw(2);
    in.sensor_fault = Bool(1);
    in.speed = Num(10.0);
    r = Step(in);
    EXPECT_EQ(OUTPUT_LOCK, r.output[DOOR_LEFT]);
    // 복귀: 조건 해소 다음 주기에 NORMAL
    r = Step(Raw(3));
    EXPECT_EQ(STATE_NORMAL, r.state);
    EXPECT_EQ(0, r.warning_code);

    // DEGRADED: 명령 미적용, 규칙 적용, 복귀
    ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
    in = Raw(10);
    in.fire = Bad();
    Cmd(&in, SIDE_ALL, ACTION_LOCK);
    r = Step(in);
    EXPECT_EQ(STATE_DEGRADED, r.state);
    EXPECT_EQ(OUTPUT_RELEASE, r.output[DOOR_LEFT]);
    in = Raw(11);
    in.fire = Bad();
    in.isofix[DOOR_RIGHT] = Bool(1);
    EXPECT_EQ(OUTPUT_LOCK, Step(in).output[DOOR_RIGHT]);
    EXPECT_EQ(STATE_NORMAL, Step(Raw(12)).state);

    // SSR-015: OFF > FAULT > DEGRADED 전수
    for (int mask = 0; mask < 8; mask++)
    {
        ASSERT_EQ(STATUS_OK, Control_Init(&ctx_));
        RawInput_t t = Raw(0);
        t.ignition = Bool((mask & 1) ? 0 : 1);
        t.sensor_fault = Bool((mask >> 1) & 1);
        if ((mask >> 2) & 1)
        {
            t.adult = Bad();
        }
        State_t expect = (mask & 1) ? STATE_OFF : ((mask >> 1) & 1) ? STATE_FAULT : ((mask >> 2) & 1) ? STATE_DEGRADED : STATE_NORMAL;
        EXPECT_EQ(expect, Step(t).state) << mask;
    }
}

/** ST-014 SRS-Q-001/C-005: 같은 입력 벡터 1,000회 재생 시 공개 결과의 SHA-256 해시가 모두 같다. */
TEST_F(System, Determinism_ThousandReplays_SameSha256)
{
    RecordProperty("requirements", "SRS-Q-001,SRS-C-005");
    std::string first;
    for (int run = 0; run < 1000; run++)
    {
        Control_t c{};
        ASSERT_EQ(STATUS_OK, Control_Init(&c));
        Sha256 sha;
        for (int cycle = 0; cycle < 120; cycle++)
        {
            RawInput_t in = Raw(cycle);
            in.speed = Num((cycle % 9) * 1.0);
            in.approach[cycle % 2] = Bool(cycle % 3 == 0 ? 1 : 0);
            if (cycle % 7 == 0)
            {
                Cmd(&in, cycle % 3, cycle % 2);
            }
            CycleResult_t r{};
            ASSERT_EQ(STATUS_OK, Control_Step(&c, &in, &r));
            PublicResultHash(r, &sha);
        }
        std::string digest = sha.Final();
        if (run == 0)
        {
            first = digest;
        }
        EXPECT_EQ(first, digest) << run;
    }
    EXPECT_EQ(64U, first.size());
}

} // namespace
