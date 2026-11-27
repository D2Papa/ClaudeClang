# 통합 테스트 계획과 인터페이스 커버리지

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-ITP-001 |
| 버전 | 1.0 (Iter-1, Iter-2) |
| 상태 | 초안 (리뷰 대기) |
| 작성 | Claude Sonnet 5.5, 2026-11-27, 요청자 KYLee |

## 1. 목적과 전략

- 대상: 아키텍처 인터페이스 ARCH-I-001~010로 연결된 컴포넌트 조합 (DOC-ARC-001, DOC-ARC-002).
- 전략: 실제 컴포넌트를 모두 연결하고(스텁 없음) 요청·입력 한 건이 인터페이스를 차례로 거쳐 최종 응답까지 이어지는 시나리오를 검증한다. 인터페이스마다 정상 경로와 오류 경로를 하나 이상 포함한다.
- 합격 기준 (CLAUDE.md 2.1): 통합 테스트 **함수 커버리지 100%**, **성공률 100%**. 측정은 `scripts/ci/coverage.sh`(스위트별 분리)이고 기준 검사는 `scripts/ci/coverage_gate.py`이다.
- 실행: `claudeclangbuild` 컨테이너에서 `scripts/ci/build.sh`, `test.sh`, `coverage.sh` 순서.
- 파일: `test/integration/control_integration_test.cpp`(Iter-1, 제어 코어), `test/integration/web_integration_test.cpp`(Iter-2, Web 조합).

## 2. 인터페이스 커버리지 표

| 인터페이스 | 제공자 → 사용자 | 정상 경로 | 오류 경로 | 통합 테스트 |
|------------|-----------------|-----------|-----------|-------------|
| ARCH-I-001 입력 검증 | C-001 → C-013 | 정상 입력 정규화 | 무효 입력 INVALID 표시 | IT-001, IT-003, IT-021 |
| ARCH-I-002 신선도 | C-002 → C-013 | 신선한 차속 | 오래된 차속, 시각 역행 ERR_STATE | IT-001, IT-008 |
| ARCH-I-003 상태 결정 | C-003 → C-013 | NORMAL | DEGRADED, OFF, FAULT | IT-001, IT-005, IT-023 |
| ARCH-I-004 규칙 평가 | C-004~010 → C-013 | 규칙 7개 중 단독 활성 | 입력 무효 시 규칙 비활성 | IT-002, IT-004, IT-007 |
| ARCH-I-005 override | C-011 → C-013 | 성립·지속 | 접근위험 해소 시 종료 | IT-002, IT-020 |
| ARCH-I-006 중재 | C-012 → C-013 | 우선순위 적용 | 상태에 의한 규칙 제외, 이유 덮어쓰기 | IT-005, IT-007, IT-023 |
| ARCH-I-007 제어 주기 | C-013 → C-016, 테스트 | 한 주기 실행 | 시각 역행 오류 전파 | IT-006, IT-008, IT-020 |
| ARCH-I-008 상태 표시 | C-014 → C-016 | JSON 직렬화 | 본문 상한 초과 HTTP 500 | IT-020, IT-021, IT-024 |
| ARCH-I-009 이벤트 기록 | C-015 → C-016 | 변화 시 기록, 조회 | reset 후 0건 | IT-022 |
| ARCH-I-010 Web 요청 처리 | C-016 → 어댑터 | 200 응답 5개 경로, 실제 TCP 연결(소켓 어댑터 경유) | 404, 405, 400, 500 | IT-020~IT-025 |

## 3. 절차와 결과 기록

1. 컨테이너에서 전체 게이트를 실행한다 (`scripts/ci/run_all.sh`).
2. 통합 테스트 결과는 `reports/test/integration.xml`, 커버리지는 `reports/coverage/integration/`에 남는다.
3. 최신 결과: 통합 테스트 14건 모두 통과, 함수 커버리지 100% (44/44 제어 코어 + Iter-2 모듈 포함).
