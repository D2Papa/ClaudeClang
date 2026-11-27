/**
 * @file    types.h
 * @brief   제어 코어가 공유하는 데이터 타입 정의 (함수 없음).
 * @trace   DOC-ARC-001 4장, DOC-DDS-001 1장
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
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>

#include "common/status.h"

/** 도어 출력 값 */
typedef enum
{
    OUTPUT_RELEASE = 0, /**< 해제 */
    OUTPUT_LOCK = 1     /**< 잠금 */
} Output_t;

/** 도어 인덱스 (배열 첨자) */
typedef enum
{
    DOOR_LEFT = 0,  /**< 후석 좌 */
    DOOR_RIGHT = 1, /**< 후석 우 */
    DOOR_COUNT = 2  /**< 도어 수 */
} Door_t;

/** 입력 유효성 */
typedef enum
{
    VALIDITY_INVALID = 0, /**< 무효 */
    VALIDITY_VALID = 1    /**< 유효 */
} Validity_t;

/** 원시 입력 값의 종류 */
typedef enum
{
    RAW_ABSENT = 0, /**< 누락 */
    RAW_NUMBER = 1, /**< 실수 */
    RAW_BOOLEAN = 2, /**< 불리언 */
    RAW_ENUM = 3,   /**< 열거 식별자 */
    RAW_OTHER = 4   /**< 형식 오류 (문자열 등) */
} RawKind_t;

/** 원시 입력 필드 하나 */
typedef struct
{
    RawKind_t kind; /**< 값의 종류 */
    double number;  /**< kind가 RAW_NUMBER일 때의 값 */
    int boolean;    /**< kind가 RAW_BOOLEAN일 때의 값 (0 또는 1) */
    int enum_id;    /**< kind가 RAW_ENUM일 때의 식별자 */
} RawValue_t;

/** gear 열거 */
typedef enum
{
    GEAR_P = 0, /**< P */
    GEAR_N = 1, /**< N */
    GEAR_D = 2, /**< D */
    GEAR_R = 3, /**< R */
    GEAR_MAX = 3 /**< 최대 식별자 */
} Gear_t;

/** crash_status 열거 */
typedef enum
{
    CRASH_NONE = 0,      /**< 없음 */
    CRASH_PENDING = 1,   /**< 판정 중 */
    CRASH_CONFIRMED = 2, /**< 확정 */
    CRASH_MAX = 2        /**< 최대 식별자 */
} CrashStatus_t;

/** 명령 side 열거 */
typedef enum
{
    SIDE_LEFT = 0,  /**< 좌 */
    SIDE_RIGHT = 1, /**< 우 */
    SIDE_ALL = 2,   /**< 전체 */
    SIDE_MAX = 2    /**< 최대 식별자 */
} Side_t;

/** 명령 action 열거 */
typedef enum
{
    ACTION_LOCK = 0,   /**< 잠금 */
    ACTION_UNLOCK = 1, /**< 해제 */
    ACTION_MAX = 1     /**< 최대 식별자 */
} Action_t;

/** 명령 source 열거 */
typedef enum
{
    SOURCE_PHYSICAL_BUTTON = 0, /**< 물리 버튼 */
    SOURCE_AVN = 1,             /**< AVN */
    SOURCE_VOICE = 2,           /**< 음성 */
    SOURCE_MOBILE_APP = 3,      /**< 모바일 앱 */
    SOURCE_MAX = 3              /**< 최대 식별자 */
} Source_t;

/** 한 평가주기의 원시 입력 */
typedef struct
{
    double timestamp_s;          /**< 이 주기의 논리 시각 (초) */
    RawValue_t speed;            /**< OEM-IF-001 vehicle_speed_kph */
    RawValue_t gear;             /**< OEM-IF-001 gear */
    RawValue_t source_timestamp; /**< OEM-IF-001 source_timestamp_s */
    RawValue_t crash;            /**< OEM-IF-002 crash_status */
    RawValue_t approach[DOOR_COUNT]; /**< OEM-IF-003 접근위험 (좌, 우) */
    RawValue_t fire;             /**< OEM-IF-007 fire_detected */
    RawValue_t overtemp;         /**< OEM-IF-007 overtemperature_detected */
    RawValue_t adult;            /**< OEM-IF-007 adult_present */
    RawValue_t isofix[DOOR_COUNT]; /**< OEM-IF-008 ISOFIX (좌, 우) */
    RawValue_t ignition;         /**< OEM-IF-009 ignition_on */
    RawValue_t sensor_fault;     /**< OEM-IF-009 sensor_fault */
    RawValue_t cmd_side;         /**< OEM-IF-004 side */
    RawValue_t cmd_action;       /**< OEM-IF-004 action */
    RawValue_t cmd_source;       /**< OEM-IF-004 source */
} RawInput_t;

/** 검증된 실수 입력 */
typedef struct
{
    double value;        /**< 값 (INVALID이면 0.0) */
    Validity_t validity; /**< 유효성 */
} Num_t;

/** 검증된 열거 입력 */
typedef struct
{
    int value;           /**< 값 (INVALID이면 0) */
    Validity_t validity; /**< 유효성 */
} Enum_t;

/** 검증된 불리언 입력 */
typedef struct
{
    int value;           /**< 값 (INVALID이면 0) */
    Validity_t validity; /**< 유효성 */
} Flag_t;

/** 검증된 운전자 명령 */
typedef struct
{
    int present;         /**< 명령 필드가 하나라도 있으면 1 */
    Validity_t validity; /**< present일 때의 유효성 */
    int side;            /**< Side_t */
    int action;          /**< Action_t */
    int source;          /**< Source_t */
    int target[DOOR_COUNT]; /**< 명령이 지정한 도어 (좌, 우) */
} Command_t;

/** 검증·정규화된 입력 묶음 (VehicleSnapshot) */
typedef struct
{
    double timestamp_s;            /**< 이 주기의 논리 시각 */
    Num_t speed;                   /**< 차속 (km/h) */
    Enum_t gear;                   /**< Gear_t */
    Num_t source_timestamp;        /**< 차속 원천 시각 (초) */
    Enum_t crash;                  /**< CrashStatus_t */
    Flag_t approach[DOOR_COUNT];   /**< 접근위험 */
    Flag_t fire;                   /**< 화재 */
    Flag_t overtemp;               /**< 과온 */
    Flag_t adult;                  /**< 성인 존재 */
    Flag_t isofix[DOOR_COUNT];     /**< ISOFIX */
    Flag_t ignition;               /**< ignition_on */
    Flag_t sensor_fault;           /**< sensor_fault */
    Command_t cmd;                 /**< 운전자 명령 */
} Snapshot_t;

/** 시스템 상태 */
typedef enum
{
    STATE_OFF = 0,      /**< 기능 비활성 */
    STATE_NORMAL = 1,   /**< 정상 */
    STATE_DEGRADED = 2, /**< 입력 이상 */
    STATE_FAULT = 3     /**< 센서 고장 */
} State_t;

/** 결정 이유 코드 (priority_reason과 같은 값 집합) */
typedef enum
{
    REASON_NONE = 0,
    REASON_CMD_APPLIED,
    REASON_CMD_REJECTED_SPEED,
    REASON_INPUT_INVALID,
    REASON_CRASH_CONFIRMED,
    REASON_CRASH_PENDING,
    REASON_FIRE,
    REASON_OVERTEMP,
    REASON_ADULT_PRESENT,
    REASON_IGNITION_OFF,
    REASON_SENSOR_FAULT,
    REASON_DEGRADED_STALE,
    REASON_APPROACH_RISK_LOCK,
    REASON_APPROACH_RISK_SUPPRESSED,
    REASON_OVERRIDE,
    REASON_ISOFIX_LOCK,
    REASON_AUTO_LOCK
} ReasonCode_t;

/** override 상태 (도어별) */
typedef struct
{
    int active[DOOR_COUNT]; /**< override가 지속 중이면 1 */
} OverrideStatus_t;

/** 규칙 평가 입력 */
typedef struct
{
    const Snapshot_t *snapshot;        /**< 검증된 입력 */
    State_t state;                     /**< 이번 주기의 state */
    Output_t previous[DOOR_COUNT];     /**< 직전 출력 */
    OverrideStatus_t override_status;  /**< override 상태 */
} RuleInput_t;

/** 규칙 평가 결과 */
typedef struct
{
    int active[DOOR_COUNT];               /**< 규칙이 출력을 요구하면 1 */
    Output_t effect[DOOR_COUNT];          /**< 요구하는 출력 */
    ReasonCode_t reason[DOOR_COUNT];      /**< 활성 규칙의 reason_code */
    ReasonCode_t info_reason[DOOR_COUNT]; /**< 비활성이지만 기록할 코드 */
    int unlock_allowed[DOOR_COUNT];       /**< (명령 규칙) 허용된 unlock 요청 */
    int lock_requested[DOOR_COUNT];       /**< (명령 규칙) lock 요청 */
    int suppressed[DOOR_COUNT];           /**< (접근위험 규칙) unlock 억제 */
} RuleResult_t;

/** 우선순위 순위 수 (순위 1~7) */
#define RULE_COUNT 7

/** 순위 인덱스 (배열 첨자, 순위-1) */
typedef enum
{
    RANK_CRASH = 0,
    RANK_EMERGENCY = 1,
    RANK_IGNITION = 2,
    RANK_APPROACH = 3,
    RANK_ISOFIX = 4,
    RANK_DRIVER = 5,
    RANK_AUTO_LOCK = 6
} Rank_t;

/** 중재 입력 */
typedef struct
{
    RuleResult_t rules[RULE_COUNT];    /**< 순위 순서의 규칙 결과 */
    State_t state;                     /**< state */
    Output_t previous[DOOR_COUNT];     /**< 직전 출력 */
    OverrideStatus_t override_status;  /**< override 상태 */
} ArbiterInput_t;

/** 중재 결과 */
typedef struct
{
    Output_t output[DOOR_COUNT];            /**< 도어별 출력 */
    ReasonCode_t priority_reason[DOOR_COUNT]; /**< 도어별 결정 규칙의 이유 (없으면 NONE) */
    ReasonCode_t reason_code[DOOR_COUNT];     /**< 도어별 기록 이유 */
} Decision_t;

/** 한 평가주기의 최종 결과 */
typedef struct
{
    Output_t output[DOOR_COUNT];              /**< OEM-IF-005 출력 */
    State_t state;                            /**< state */
    ReasonCode_t priority_reason[DOOR_COUNT]; /**< 도어별 priority_reason */
    ReasonCode_t reason_code[DOOR_COUNT];     /**< 도어별 reason_code */
    ReasonCode_t priority_reason_summary;     /**< 요약 priority_reason (좌가 NONE이 아니면 좌, 아니면 우) */
    ReasonCode_t reason_code_summary;         /**< 요약 reason_code (좌가 NONE이 아니면 좌, 아니면 우) */
    int warning_code;                         /**< 0 또는 1(W-001) */
    unsigned int input_validity;              /**< 필수 입력과 gear의 VALID 비트 마스크 */
} CycleResult_t;

#endif /* COMMON_TYPES_H */
