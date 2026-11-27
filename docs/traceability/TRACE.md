# 추적성 매트릭스 (TRACE)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-TRC-001 |
| 버전 | 1.7 (Iter-1, Iter-2 설계 반영) |
| 상태 | scripts/docs/gen_trace.py로 생성 (리뷰 생략: Iter-1은 사용자 지시) |
| 입력 문서 버전 | SRS v1.6, SAFETY-REQ v1.2, ASSUMPTIONS v1.7, ARCH v0.7, ARCH-iter2 v0.5, DDS v0.8, DDS-iter2 v0.5 |

하향 추적은 문서와 코드의 추적 표기에서 스크립트로 생성한다. 빈 칸은 해당 이터레이션 범위 밖이거나 누락이다.
ID는 항상 접두사를 포함한 전체 표기를 쓴다.

## 1. 요구사항 추적 (상향 + 하향)

| 요구 ID | 문서 | 상위 요구 | 아키텍처 | 상세 설계 | 소스 | 단위 테스트 | 통합 테스트 | 시스템 테스트 |
|---------|------|-----------|----------|-----------|------|-------------|-------------|---------------|
| SRS-C-001 | SRS | 프로젝트 결정(A-11), OEM-SWR-001 3장 | ARCH-C-013 | DD-F-026 |  |  |  | 검사 |
| SRS-C-002 | SRS | OEM-IF-005 | ARCH-C-013 | DD-F-026 | control_cycle.c, control_cycle.h |  |  | 검사 |
| SRS-C-003 | SRS | OEM-IF-007 | ARCH-C-001 | DD-F-002 | input_validator.c, input_validator.h | UT-004 |  | ST-011 |
| SRS-C-004 | SRS | OEM-SWR-001 3장, OEM-FR-004 | ARCH-C-016 | DD-F-051, DD-F-052, DD-F-053, DD-F-054, DD-F-055, DD-F-056, DD-F-059, DD-F-060, DD-F-061, DD-F-063, DD-F-064, DD-F-065, DD-F-066, DD-F-067, DD-F-070 | web_handler.c, web_handler.h, web_main.c, web_page.c, web_page.h, web_request_parser.c, web_request_parser.h | UT-133 |  | 검사 |
| SRS-C-005 | SRS | OEM-NFR-001, OEM-SR-001, OEM-SR-003 | ARCH-C-013 | DD-F-025, DD-F-026 | control_cycle.c, control_cycle.h |  |  | ST-014 |
| SRS-F-001 | SRS | OEM-FR-001, OEM-IF-004 | ARCH-C-001, ARCH-C-009 | DD-F-001, DD-F-005, DD-F-015 | driver_command_rule.c, driver_command_rule.h, input_validator.c, input_validator.h | UT-011, UT-013 |  | ST-001 |
| SRS-F-002 | SRS | OEM-FR-001, OEM-IF-004, OEM-IF-005 | ARCH-C-009 | DD-F-015, DD-F-016 | driver_command_rule.c, driver_command_rule.h, input_validator.c | UT-013, UT-051, UT-053 | IT-006 | ST-001 |
| SRS-F-003 | SRS | OEM-FR-001, OEM-IF-001 | ARCH-C-009 | DD-F-015, DD-F-024, DD-F-029 | arbiter.c, driver_command_rule.c, driver_command_rule.h | UT-052, UT-078 | IT-004 | ST-002, ST-104 |
| SRS-F-004 | SRS | OEM-FR-002, OEM-IF-001 | ARCH-C-010 | DD-F-017 | auto_lock_rule.c, auto_lock_rule.h | UT-049 | IT-004 | ST-003 |
| SRS-F-007 | SRS | OEM-FR-003, OEM-SR-002 | ARCH-C-009, ARCH-C-011 | DD-F-018, DD-F-019, DD-F-020, DD-F-027 | override_manager.c, override_manager.h | UT-061, UT-062, UT-088 | IT-002, IT-020 | ST-004 |
| SRS-F-014 | SRS | OEM-FR-005, OEM-IF-007 | ARCH-C-005 | DD-F-011 | emergency_rule.c, emergency_rule.h | UT-043 | IT-007 | ST-005 |
| SRS-F-015 | SRS | OEM-FR-006, OEM-IF-008 | ARCH-C-008 | DD-F-014 | isofix_rule.c, isofix_rule.h | UT-048 |  | ST-006 |
| SRS-F-016 | SRS | OEM-FR-007, OEM-IF-009 | ARCH-C-003, ARCH-C-006 | DD-F-008, DD-F-012 | ignition_rule.c, ignition_rule.h, state_manager.c, state_manager.h | UT-044 | IT-005 | ST-007 |
| SRS-F-017 | SRS | OEM-SR-001, OEM-SR-002, OEM-FR-002, OEM-FR-005, OEM-FR-006, OEM-FR-007 | ARCH-C-012 | DD-F-021, DD-F-023 | arbiter.c, arbiter.h | UT-071, UT-073 | IT-004, IT-007 | ST-008 |
| SRS-F-018 | SRS | OEM-FR-004 | ARCH-C-014 | DD-F-040, DD-F-042, DD-F-043, DD-F-045, DD-F-052, DD-F-057 | status_presenter.c, status_presenter.h, web_handler.c, web_handler.h | UT-106, UT-134 |  | ST-101 |
| SRS-F-019 | SRS | OEM-FR-004, OEM-IF-006 | ARCH-C-014 | DD-F-040, DD-F-041, DD-F-043, DD-F-045 | control_cycle.c, status_presenter.c, status_presenter.h | UT-083, UT-101, UT-104 |  | ST-101 |
| SRS-F-020 | SRS | OEM-NFR-002 | ARCH-C-015 | DD-F-046, DD-F-047, DD-F-049 | event_recorder.c, event_recorder.h |  | IT-022 | ST-103 |
| SRS-F-021 | SRS | OEM-NFR-002 | ARCH-C-015 | DD-F-049, DD-F-050, DD-F-058 | event_recorder.c, event_recorder.h, web_handler.c |  |  | ST-103 |
| SRS-F-022 | SRS | OEM-IF-006 | ARCH-C-014 | DD-F-040, DD-F-043 | control_cycle.c, status_presenter.c, status_presenter.h |  |  | ST-101, ST-104 |
| SRS-F-023 | SRS | OEM-IF-006 | ARCH-C-014 | DD-F-042, DD-F-044, DD-F-057, DD-F-070 | status_presenter.c, status_presenter.h, web_handler.c, web_handler.h | UT-105, UT-107, UT-135, UT-138 | IT-024 | ST-102 |
| SRS-F-024 | SRS | OEM-NFR-002 | ARCH-C-015 | DD-F-047, DD-F-049, DD-F-058, DD-F-062 | event_recorder.c, event_recorder.h, web_handler.c | UT-112 |  | ST-103 |
| SRS-F-025 | SRS | OEM-NFR-002 | ARCH-C-015 | DD-F-046, DD-F-047, DD-F-048, DD-F-059, DD-F-064 | control_cycle.c, event_recorder.c, event_recorder.h, web_handler.c, web_handler.h | UT-111 | IT-022 | ST-103 |
| SRS-F-026 | SRS | OEM-NFR-002 | ARCH-C-015 | DD-F-046, DD-F-050 | event_recorder.c, event_recorder.h | UT-114, UT-140 | IT-022 | ST-103 |
| SRS-F-027 | SRS | OEM-FR-007, OEM-IF-009 | ARCH-C-003 | DD-F-008 | state_manager.c, state_manager.h |  |  | ST-007 |
| SRS-F-028 | SRS | OEM-FR-004, OEM-IF-006 | ARCH-C-012 | DD-F-021, DD-F-023 | arbiter.c, arbiter.h | UT-072, UT-073 |  | ST-008 |
| SRS-F-029 | SRS | OEM-FR-003, OEM-IF-006 | ARCH-C-009, ARCH-C-011, ARCH-C-012 | DD-F-019, DD-F-023 | arbiter.c, override_manager.c, override_manager.h | UT-077, UT-088 | IT-002, IT-020 | ST-004 |
| SRS-F-030 | SRS | OEM-FR-003, OEM-SR-002, OEM-FR-002, OEM-IF-004 | ARCH-C-011 | DD-F-020, DD-F-023 | arbiter.c, override_manager.c, override_manager.h | UT-046, UT-064, UT-077, UT-088, UT-089 | IT-002 | ST-004 |
| SRS-F-031 | SRS | OEM-FR-007, OEM-IF-009 | ARCH-C-012 | DD-F-021, DD-F-022, DD-F-024, DD-F-029 | arbiter.c, arbiter.h | UT-075 | IT-005 | ST-007 |
| SRS-F-032 | SRS | OEM-FR-001, OEM-IF-004, OEM-IF-006 | ARCH-C-012 | DD-F-021, DD-F-029 | arbiter.c | UT-079 | IT-023 | ST-104 |
| SRS-L-004 | SRS | OEM-FR-002 | ARCH-C-009, ARCH-C-010 | DD-F-015, DD-F-017 | auto_lock_rule.c, auto_lock_rule.h, driver_command_rule.c, driver_command_rule.h, params.h | UT-052 |  | ST-002, ST-003 |
| SRS-L-005 | SRS | OEM-FR-003 | ARCH-C-011 | DD-F-020 | override_manager.c, override_manager.h, params.h | UT-063 |  | ST-004 |
| SRS-L-006 | SRS | OEM-NFR-002 | ARCH-C-015 | DD-F-049 | event_recorder.c, event_recorder.h, params.h |  |  | ST-103 |
| SRS-L-007 | SRS | OEM-IF-001 | ARCH-C-001 | DD-F-003 | input_validator.c, input_validator.h, params.h | UT-005 |  | ST-011 |
| SRS-Q-001 | SRS | OEM-NFR-001 | ARCH-C-013 | DD-F-025, DD-F-026 | control_cycle.c, control_cycle.h | UT-084, UT-091 |  | ST-014 |
| SRS-Q-002 | SRS | OEM-NFR-002 | ARCH-C-014, ARCH-C-015 | DD-F-049, DD-F-058 | event_recorder.h |  |  | ST-105 |
| SRS-Q-003 | SRS | OEM-IF-001, OEM-IF-002, OEM-IF-003, OEM-IF-004, OEM-IF-005, OEM-IF-006, OEM-IF-007, OEM-IF-008, OEM-IF-009 | ARCH-C-001, ARCH-C-014 | DD-F-001, DD-F-043, DD-F-063 | input_validator.h, status_presenter.h, web_request_parser.c, web_request_parser.h | UT-002, UT-122 |  | ST-106 |
| SSR-001 | 안전(DOC-SAF-001) | OEM-SR-001, OEM-IF-002 | ARCH-C-004 | DD-F-010 | crash_rule.c, crash_rule.h | UT-042, UT-085 | IT-007 | ST-009 |
| SSR-002 | 안전(DOC-SAF-001) | OEM-SR-001 | ARCH-C-004, ARCH-C-013 | DD-F-010, DD-F-026 | control_cycle.c, control_cycle.h, crash_rule.c, crash_rule.h | UT-085 |  | ST-009 |
| SSR-003 | 안전(DOC-SAF-001) | OEM-SR-001, OEM-IF-002 | ARCH-C-004 | DD-F-010, DD-F-024 | crash_rule.c, crash_rule.h | UT-042, UT-078 |  | ST-009 |
| SSR-004 | 안전(DOC-SAF-001) | OEM-SR-002, OEM-IF-003 | ARCH-C-007 | DD-F-013 | approach_risk_rule.c, approach_risk_rule.h | UT-045 |  | ST-010 |
| SSR-005 | 안전(DOC-SAF-001) | OEM-SR-002, OEM-IF-003 | ARCH-C-007, ARCH-C-011 | DD-F-013, DD-F-020 | approach_risk_rule.c, approach_risk_rule.h | UT-047 | IT-002 | ST-004, ST-010 |
| SSR-006 | 안전(DOC-SAF-001) | OEM-SR-002, OEM-IF-003 | ARCH-C-007 | DD-F-013 | approach_risk_rule.c, approach_risk_rule.h | UT-045 |  | ST-010 |
| SSR-007 | 안전(DOC-SAF-001) | OEM-SR-003, OEM-IF-001, OEM-IF-002, OEM-IF-003, OEM-IF-004, OEM-IF-007, OEM-IF-008, OEM-IF-009 | ARCH-C-001 | DD-F-001, DD-F-002, DD-F-003, DD-F-004, DD-F-005 | arbiter.c, input_validator.c, input_validator.h, web_request_parser.c, web_request_parser.h | UT-003, UT-004, UT-006, UT-008, UT-012, UT-033, UT-050, UT-079, UT-123, UT-136 | IT-003, IT-021 | ST-011 |
| SSR-008 | 안전(DOC-SAF-001) | OEM-SR-003 | ARCH-C-002 | DD-F-007, DD-F-009 | freshness_monitor.c, freshness_monitor.h, params.h, state_manager.c | UT-024, UT-086 | IT-001 | ST-012 |
| SSR-009 | 안전(DOC-SAF-001) | OEM-SR-003 | ARCH-C-002, ARCH-C-003 | DD-F-008, DD-F-009, DD-F-024 | arbiter.c, state_manager.c, state_manager.h | UT-033, UT-086 | IT-001 | ST-012 |
| SSR-010 | 안전(DOC-SAF-001) | OEM-SR-003 | ARCH-C-012 | DD-F-021, DD-F-022 | arbiter.c, arbiter.h | UT-076, UT-086, UT-090 |  | ST-013 |
| SSR-011 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-IF-009 | ARCH-C-012 | DD-F-021, DD-F-022 | arbiter.c, arbiter.h | UT-076, UT-087 |  | ST-013 |
| SSR-012 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-IF-009 | ARCH-C-003 | DD-F-008, DD-F-024 | arbiter.c, state_manager.c, state_manager.h | UT-032, UT-087 | IT-023 | ST-013 |
| SSR-013 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-SR-001, OEM-SR-003 | ARCH-C-012 | DD-F-021, DD-F-023 | arbiter.c, arbiter.h |  |  | ST-009 |
| SSR-014 | 안전(DOC-SAF-001) | OEM-SR-003, OEM-SR-004 | ARCH-C-003 | DD-F-008 | state_manager.c, state_manager.h |  |  | ST-013 |
| SSR-015 | 안전(DOC-SAF-001) | OEM-SR-003, OEM-SR-004 | ARCH-C-003 | DD-F-008 | state_manager.c, state_manager.h | UT-031, UT-032 |  | ST-013 |
| SSR-016 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-SR-003, OEM-SR-002, OEM-FR-002, OEM-FR-006 | ARCH-C-012 | DD-F-021, DD-F-023 | arbiter.c, arbiter.h | UT-076, UT-087 |  | ST-013 |

## 2. OEM 요구 -> 도출 요구 역추적

| OEM 요구 | 도출한 요구 |
|----------|-------------|
| OEM-SR-001 | SRS-C-005, SRS-F-017, SSR-001, SSR-002, SSR-003, SSR-013 |
| OEM-SR-002 | SRS-F-007, SRS-F-017, SRS-F-030, SSR-004, SSR-005, SSR-006, SSR-016 |
| OEM-SR-003 | SRS-C-005, SSR-007, SSR-008, SSR-009, SSR-010, SSR-013, SSR-014, SSR-015, SSR-016 |
| OEM-SR-004 | SSR-011, SSR-012, SSR-013, SSR-014, SSR-015, SSR-016 |
| OEM-FR-001 | SRS-F-001, SRS-F-002, SRS-F-003, SRS-F-032 |
| OEM-FR-002 | SRS-F-004, SRS-F-017, SRS-F-030, SRS-L-004, SSR-016 |
| OEM-FR-003 | SRS-F-007, SRS-F-029, SRS-F-030, SRS-L-005 |
| OEM-FR-004 | SRS-C-004, SRS-F-018, SRS-F-019, SRS-F-028 |
| OEM-FR-005 | SRS-F-014, SRS-F-017 |
| OEM-FR-006 | SRS-F-015, SRS-F-017, SSR-016 |
| OEM-FR-007 | SRS-F-016, SRS-F-017, SRS-F-027, SRS-F-031 |
| OEM-NFR-001 | SRS-C-005, SRS-Q-001 |
| OEM-NFR-002 | SRS-F-020, SRS-F-021, SRS-F-024, SRS-F-025, SRS-F-026, SRS-L-006, SRS-Q-002 |
| OEM-IF-001 | SRS-F-003, SRS-F-004, SRS-L-007, SRS-Q-003, SSR-007 |
| OEM-IF-002 | SRS-Q-003, SSR-001, SSR-003, SSR-007 |
| OEM-IF-003 | SRS-Q-003, SSR-004, SSR-005, SSR-006, SSR-007 |
| OEM-IF-004 | SRS-F-001, SRS-F-002, SRS-F-030, SRS-F-032, SRS-Q-003, SSR-007 |
| OEM-IF-005 | SRS-C-002, SRS-F-002, SRS-Q-003 |
| OEM-IF-006 | SRS-F-019, SRS-F-022, SRS-F-023, SRS-F-028, SRS-F-029, SRS-F-032, SRS-Q-003 |
| OEM-IF-007 | SRS-C-003, SRS-F-014, SRS-Q-003, SSR-007 |
| OEM-IF-008 | SRS-F-015, SRS-Q-003, SSR-007 |
| OEM-IF-009 | SRS-F-016, SRS-F-027, SRS-F-031, SRS-Q-003, SSR-007, SSR-011, SSR-012 |

## 3. 편차 기록

| 편차 | 관련 요구 | 상태 |
|------|-----------|------|
| 구현 언어 C17/Clang (OEM-SWR-001 3장은 Python 3.12) | SRS-C-001 | 사용자 승인(OEM 대행), OEM 확인 대기 (A-11) |
| 모호성 해소 가정 | A-01, A-02, A-03, A-04, A-05, A-06, A-07, A-08, A-09 | 사용자 "제안안" 승인(OEM 대행), OEM 확인 대기 |
| SRS-F-003과 SRS-F-022 충돌 해소 | A-19 | 사용자 선택 "F-022에 예외 추가 (추천)" (2026-11-27), OEM 확인 대기 |
| 형식 오류 명령 reason과 상태 이유 충돌 해소 | A-25 | 사용자 선택 "상태 이유 우선 (추천)" (2026-11-27), 편차(SSR-007의 INPUT_INVALID 기록은 NORMAL에서만), OEM 확인 대기 |
| 리뷰 지적과 구현 중 추가된 가정 | A-13, A-14, A-15, A-16, A-17, A-18, A-20, A-21, A-22, A-23, A-24 | 작성자 결정(사용자 위임 "모두 너의 관장안으로"), 사용자 확인 대기, OEM 확인 대기 |

## 4. 추적 누락 목록

- 고아 요구(상위 없음): 없음 (SRS-F-017은 파생 요구로 상위 OEM 요구를 명시)
- OEM 요구 중 도출 요구 없음: 없음 (OEM-SR 4, FR 7, NFR 2, IF 9 모두 도출됨)
- 시스템 테스트 칸이 빈 요구 중 Iter-2(표시, 이력, Web) 설계 범위(구현 전): 없음
- 시스템 테스트 칸이 빈 요구 중 Iter-2 범위가 아닌 것(누락): 없음
- OEM SWR-019: 결번 (OEM 추적표에 없음)
