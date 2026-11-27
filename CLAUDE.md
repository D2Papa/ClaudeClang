# CLAUDE.md

이 파일은 Claude Code가 이 저장소에서 작업할 때 항상 읽는 프로젝트 지침입니다.
템플릿을 실제 프로젝트에 적용할 때 `<!-- TODO -->` 표시된 부분을 채워 주세요.

## 프로젝트 개요

<!-- TODO: 프로젝트가 무엇을 하는지 2~3문장으로 설명 -->
범용 Claude Code 프로젝트 템플릿입니다. 프로젝트 지침(CLAUDE.md), 설정(`.claude/settings.json`),
서브에이전트(`.claude/agents/`), 스킬(`.claude/skills/`)의 기본 구성을 담고 있습니다.

## 자주 쓰는 명령

<!-- TODO: 실제 프로젝트의 명령으로 교체 -->
| 작업 | 명령 |
|------|------|
| 의존성 설치 | `TODO` |
| 빌드 | `TODO` |
| 테스트 | `TODO` |
| 린트/포맷 | `TODO` |

## 디렉터리 구조

```
.
├── CLAUDE.md                 # 프로젝트 지침 (이 파일)
├── .claude/
│   ├── settings.json         # 팀 공용 권한·설정 (커밋 대상)
│   ├── settings.local.json   # 개인 설정 (gitignore, 직접 생성)
│   ├── agents/               # 서브에이전트 정의 (*.md)
│   └── skills/               # 스킬 (<이름>/SKILL.md)
└── README.md
```

## 작업 규칙

- 응답과 주석은 한국어로 작성하고, 코드 식별자와 기술 용어는 원문을 유지합니다.
- 변경은 요청 범위 안에서 최소한으로 하고, 관련 없는 리팩터링은 하지 않습니다.
- 코드를 수정한 뒤에는 위 표의 테스트·린트 명령을 실행해 결과를 확인합니다.
- 커밋 메시지는 `commit` 스킬의 규칙(Conventional Commits)을 따릅니다.
- 비밀값(.env, 키, 토큰)은 커밋하지 않습니다.

## 서브에이전트

| 이름 | 용도 |
|------|------|
| `code-reviewer` | 변경된 코드의 버그·보안·가독성 리뷰 (읽기 전용) |
| `test-runner` | 테스트 실행, 실패 원인 분석 및 최소 수정 |
| `doc-writer` | README·주석·변경 이력 등 문서 작성 |

## 스킬

| 이름 | 용도 |
|------|------|
| `commit` | 변경 사항을 분석해 Conventional Commits 형식으로 커밋 |
| `changelog` | git 이력으로 CHANGELOG 항목 생성 |
| `new-agent` | 새 서브에이전트 파일 생성 |
| `new-skill` | 새 스킬 디렉터리와 SKILL.md 생성 |

### A-SPICE 개발 스킬

V-모델 순서대로 수행하며, 모든 산출물은 작성 후 `review` 스킬로 리뷰한다.
모든 요구사항은 이후 산출물(아키텍처 → 상세 설계 → 소스 → 테스트)과 양방향 추적성을 유지한다 (`docs/traceability/TRACE.md`).

| 단계 | 스킬 | A-SPICE | 핵심 규칙 |
|------|------|---------|-----------|
| 1 | `requirements-analysis` | SWE.1 | 기능/비기능(품질·제한·제약) 분류, 품질은 ISO 25010, 기능 안전은 별도 문서 |
| 2 | `architecture-design` | SWE.2 | 컴포넌트·인터페이스 필수 정의, 인터페이스로만 통신, 고응집·저결합, SOLID |
| 3 | `detailed-design` | SWE.3 | 모듈 정의, 기준 이상 모듈은 인터페이스 필수, 클린코드, 단일 책임 |
| 4 | `implementation` | SWE.3 | `void` 금지(상태 코드 반환), `return`은 함수 마지막에 한 번만 |
| 5 | `unit-test` | SWE.4 | 목표 커버리지 100%, 의미 없는 케이스 금지, Doxygen 주석, MC/DC는 LLVM 스크립트 |
| 6 | `integration-test` | SWE.5 | 인터페이스 중심 테스트, 목표 커버리지 100%, Doxygen 주석 |
| 전 단계 | `review` | — | 산출물별 체크리스트(`.claude/skills/review/checklists.md`) 기반 리뷰 |

## 개발 규칙 (C/C++)

- 구현 언어는 C/C++를 기본으로 한다 (Clang 사용). 문서는 `docs/` 아래에 둔다.
- 기능 안전 요구사항은 이 저장소의 SRS가 아니라 별도 안전 문서에서 관리한다.
- LLVM 도구(`clang`, `llvm-profdata`, `llvm-cov`; MC/DC는 clang 18 이상)와 PICT가 PATH에 있어야 테스트 스킬을 완전히 쓸 수 있다.
