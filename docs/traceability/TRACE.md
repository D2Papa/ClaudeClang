# 추적성 매트릭스 (TRACE)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-TRC-001 |
| 버전 | 0.7 (Iter-0, 6차 리뷰 반려 조치본) |
| 상태 | 초안 (재리뷰 대기) |

하향 추적(아키텍처, 상세 설계, 소스, 테스트)은 후속 단계에서 채운다. 현재 단계: Iter-0 (SWE.1). ID는 항상 접두사를 포함한 전체 표기를 쓴다.

## 1. 요구사항 추적 (상향 + 하향)

| 요구 ID | 문서 | 상위 요구 | 아키텍처 | 상세 설계 | 소스 | 단위 테스트 | 통합 테스트 |
|---------|------|-----------|----------|-----------|------|-------------|-------------|
| SRS-C-001 | SRS | 프로젝트 결정(A-11), OEM-SWR-001 3장 |  |  |  |  |  |
| SRS-C-002 | SRS | OEM-IF-005 |  |  |  |  |  |
| SRS-C-003 | SRS | OEM-IF-007 |  |  |  |  |  |
| SRS-C-004 | SRS | OEM-SWR-001 3장, OEM-FR-004 |  |  |  |  |  |
| SRS-C-005 | SRS | OEM-NFR-001, OEM-SR-001, OEM-SR-003 |  |  |  |  |  |
| SRS-F-001 | SRS | OEM-FR-001, OEM-IF-004 |  |  |  |  |  |
| SRS-F-002 | SRS | OEM-FR-001, OEM-IF-005 |  |  |  |  |  |
| SRS-F-003 | SRS | OEM-FR-001, OEM-IF-001 |  |  |  |  |  |
| SRS-F-004 | SRS | OEM-FR-002, OEM-IF-001 |  |  |  |  |  |
| SRS-F-007 | SRS | OEM-FR-003, OEM-SR-002 |  |  |  |  |  |
| SRS-F-014 | SRS | OEM-FR-005, OEM-IF-007 |  |  |  |  |  |
| SRS-F-015 | SRS | OEM-FR-006, OEM-IF-008 |  |  |  |  |  |
| SRS-F-016 | SRS | OEM-FR-007, OEM-IF-009 |  |  |  |  |  |
| SRS-F-017 | SRS | OEM-SR-001, OEM-SR-002, OEM-FR-002, OEM-FR-005, OEM-FR-006, OEM-FR-007 |  |  |  |  |  |
| SRS-F-018 | SRS | OEM-FR-004 |  |  |  |  |  |
| SRS-F-019 | SRS | OEM-FR-004, OEM-IF-006 |  |  |  |  |  |
| SRS-F-020 | SRS | OEM-NFR-002 |  |  |  |  |  |
| SRS-F-021 | SRS | OEM-NFR-002 |  |  |  |  |  |
| SRS-F-022 | SRS | OEM-IF-006 |  |  |  |  |  |
| SRS-F-023 | SRS | OEM-IF-006 |  |  |  |  |  |
| SRS-F-024 | SRS | OEM-NFR-002 |  |  |  |  |  |
| SRS-F-025 | SRS | OEM-NFR-002 |  |  |  |  |  |
| SRS-F-026 | SRS | OEM-NFR-002 |  |  |  |  |  |
| SRS-F-027 | SRS | OEM-FR-007, OEM-IF-009 |  |  |  |  |  |
| SRS-F-028 | SRS | OEM-FR-004, OEM-IF-006 |  |  |  |  |  |
| SRS-F-029 | SRS | OEM-FR-003, OEM-IF-006 |  |  |  |  |  |
| SRS-F-030 | SRS | OEM-FR-003, OEM-SR-002, OEM-FR-002 |  |  |  |  |  |
| SRS-F-031 | SRS | OEM-FR-007, OEM-IF-009 |  |  |  |  |  |
| SRS-L-004 | SRS | OEM-FR-002 |  |  |  |  |  |
| SRS-L-005 | SRS | OEM-FR-003 |  |  |  |  |  |
| SRS-L-006 | SRS | OEM-NFR-002 |  |  |  |  |  |
| SRS-L-007 | SRS | OEM-IF-001 |  |  |  |  |  |
| SRS-Q-001 | SRS | OEM-NFR-001 |  |  |  |  |  |
| SRS-Q-002 | SRS | OEM-NFR-002 |  |  |  |  |  |
| SRS-Q-003 | SRS | OEM-IF-001, OEM-IF-002, OEM-IF-003, OEM-IF-004, OEM-IF-005, OEM-IF-006, OEM-IF-007, OEM-IF-008, OEM-IF-009 |  |  |  |  |  |
| SSR-001 | 안전(DOC-SAF-001) | OEM-SR-001, OEM-IF-002 |  |  |  |  |  |
| SSR-002 | 안전(DOC-SAF-001) | OEM-SR-001 |  |  |  |  |  |
| SSR-003 | 안전(DOC-SAF-001) | OEM-SR-001, OEM-IF-002 |  |  |  |  |  |
| SSR-004 | 안전(DOC-SAF-001) | OEM-SR-002, OEM-IF-003 |  |  |  |  |  |
| SSR-005 | 안전(DOC-SAF-001) | OEM-SR-002, OEM-IF-003 |  |  |  |  |  |
| SSR-006 | 안전(DOC-SAF-001) | OEM-SR-002, OEM-IF-003 |  |  |  |  |  |
| SSR-007 | 안전(DOC-SAF-001) | OEM-SR-003, OEM-IF-001, OEM-IF-002, OEM-IF-003, OEM-IF-004, OEM-IF-007, OEM-IF-008, OEM-IF-009 |  |  |  |  |  |
| SSR-008 | 안전(DOC-SAF-001) | OEM-SR-003 |  |  |  |  |  |
| SSR-009 | 안전(DOC-SAF-001) | OEM-SR-003 |  |  |  |  |  |
| SSR-010 | 안전(DOC-SAF-001) | OEM-SR-003 |  |  |  |  |  |
| SSR-011 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-IF-009 |  |  |  |  |  |
| SSR-012 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-IF-009 |  |  |  |  |  |
| SSR-013 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-SR-001, OEM-SR-003 |  |  |  |  |  |
| SSR-014 | 안전(DOC-SAF-001) | OEM-SR-003, OEM-SR-004 |  |  |  |  |  |
| SSR-015 | 안전(DOC-SAF-001) | OEM-SR-003, OEM-SR-004 |  |  |  |  |  |
| SSR-016 | 안전(DOC-SAF-001) | OEM-SR-004, OEM-SR-003, OEM-SR-002, OEM-FR-002, OEM-FR-006 |  |  |  |  |  |

## 2. OEM 요구 → 도출 요구 역추적

| OEM 요구 | 도출한 요구 |
|----------|-------------|
| OEM-SR-001 | SRS-C-005, SRS-F-017, SSR-001, SSR-002, SSR-003, SSR-013 |
| OEM-SR-002 | SRS-F-007, SRS-F-017, SRS-F-030, SSR-004, SSR-005, SSR-006, SSR-016 |
| OEM-SR-003 | SRS-C-005, SSR-007, SSR-008, SSR-009, SSR-010, SSR-013, SSR-014, SSR-015, SSR-016 |
| OEM-SR-004 | SSR-011, SSR-012, SSR-013, SSR-014, SSR-015, SSR-016 |
| OEM-FR-001 | SRS-F-001, SRS-F-002, SRS-F-003 |
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
| OEM-IF-004 | SRS-F-001, SRS-Q-003, SSR-007 |
| OEM-IF-005 | SRS-C-002, SRS-F-002, SRS-Q-003 |
| OEM-IF-006 | SRS-F-019, SRS-F-022, SRS-F-023, SRS-F-028, SRS-F-029, SRS-Q-003 |
| OEM-IF-007 | SRS-C-003, SRS-F-014, SRS-Q-003, SSR-007 |
| OEM-IF-008 | SRS-F-015, SRS-Q-003, SSR-007 |
| OEM-IF-009 | SRS-F-016, SRS-F-027, SRS-F-031, SRS-Q-003, SSR-007, SSR-011, SSR-012 |

## 3. 편차 기록

| 편차 | 관련 요구 | 상태 |
|------|-----------|------|
| 구현 언어 C17/Clang (OEM-SWR-001 3장은 Python 3.12) | SRS-C-001 | 사용자 승인(OEM 대행), OEM 확인 대기 (A-11) |
| 모호성 해소 가정 | A-01, A-02, A-03, A-04, A-05, A-06, A-07, A-08, A-09, A-13, A-14, A-15, A-16, A-17 | 사용자 승인(OEM 대행), OEM 확인 대기 |

## 4. 추적 누락 목록

- 고아 요구(상위 없음): 없음 (SRS-F-017은 파생 요구로 상위 OEM 요구를 명시)
- OEM 요구 중 도출 요구 없음: 없음 (OEM-SR 4, FR 7, NFR 2, IF 9 모두 도출됨)
- 하향 추적 미작성: 전체 요구 (아키텍처 설계 이후 단계에서 작성)
- OEM SWR-019: 결번 (OEM 추적표에 없음)
