# ClaudeClang

범용 Claude Code 프로젝트 템플릿입니다. 새 프로젝트에 복사해 `CLAUDE.md`의 TODO만 채우면
프로젝트 지침, 권한 설정, 서브에이전트, 스킬을 바로 쓸 수 있습니다.

## 구성

| 경로 | 설명 |
|------|------|
| `CLAUDE.md` | Claude가 매 세션 읽는 프로젝트 지침 |
| `.claude/settings.json` | 팀 공용 권한 설정 (읽기 전용 git 명령 허용, `.env` 읽기·force push 차단) |
| `.claude/agents/code-reviewer.md` | 변경 코드 리뷰 (읽기 전용) |
| `.claude/agents/test-runner.md` | 테스트 실행·실패 분석·수정 |
| `.claude/agents/doc-writer.md` | 문서 작성·갱신 |
| `.claude/agents/aspice-reviewer.md` | A-SPICE 산출물 엄격 리뷰 (`review` 스킬 적용, 부적합 전부 조치 전에는 다음 단계 진행 불가) |
| `.claude/skills/commit/` | Conventional Commits 커밋 |
| `.claude/skills/changelog/` | git 이력으로 CHANGELOG 생성 |
| `.claude/skills/new-agent/` | 새 서브에이전트 생성 |
| `.claude/skills/new-skill/` | 새 스킬 생성 |
| `.claude/skills/requirements-analysis/` | 요구사항 분석 (A-SPICE SWE.1, ISO 25010) |
| `.claude/skills/architecture-design/` | 아키텍처 설계 (SWE.2, 인터페이스·SOLID) |
| `.claude/skills/detailed-design/` | 상세 설계 (SWE.3, 클린코드·단일 책임) |
| `.claude/skills/implementation/` | 구현 (SWE.3, 단일 반환·반환값 규칙) |
| `.claude/skills/unit-test/` | 단위 테스트 (SWE.4, 커버리지·MC/DC, `scripts/llvm_coverage.py`) |
| `.claude/skills/integration-test/` | 통합 테스트 (SWE.5, 인터페이스 중심) |
| `.claude/skills/review/` | 산출물별 체크리스트 기반 리뷰 (`checklists.md`) |

> A-SPICE 개발 스킬은 요구사항 → 아키텍처 → 상세 설계 → 구현 → 단위 테스트 → 통합 테스트 순으로 쓰고,
> 매 단계 산출물을 `review`로 리뷰합니다. 요구사항과 이후 산출물의 양방향 추적성은 `docs/traceability/TRACE.md`에서 관리합니다.

## 사용법

```text
# 스킬: 슬래시 명령으로 직접 실행하거나, 요청 내용에 맞으면 Claude가 자동으로 사용
/commit
/changelog v1.0.0..HEAD
/new-agent API 설계를 검토하는 에이전트
/new-skill 릴리스 태그를 만드는 스킬

# 서브에이전트: 이름으로 요청하거나, 작업에 맞으면 Claude가 자동으로 위임
code-reviewer로 방금 변경한 코드 리뷰해줘
test-runner로 테스트 돌리고 실패 고쳐줘
```

## 템플릿 적용 순서

1. 이 저장소의 `CLAUDE.md`, `.claude/`, `.gitignore`를 대상 프로젝트로 복사합니다.
2. `CLAUDE.md`의 `<!-- TODO -->` 부분(개요, 빌드·테스트 명령)을 채웁니다.
3. `.claude/settings.json`의 `allow`에 자주 쓰는 빌드·테스트 명령을 추가합니다.
4. 개인 설정은 `.claude/settings.local.json`에 둡니다 (gitignore 처리됨).

## agent vs skill

| | 서브에이전트 (agent) | 스킬 (skill) |
|---|---|---|
| 실행 위치 | 별도 맥락 (메인 대화를 어지럽히지 않음) | 메인 대화 안 |
| 적합한 작업 | 독립적이고 결과만 필요한 작업 (리뷰, 조사, 테스트) | 정해진 절차를 따르는 작업 (커밋, 릴리스) |
| 호출 | 자동 위임 또는 이름으로 요청 | 자동 사용 또는 `/이름` |
| 정의 파일 | `.claude/agents/<이름>.md` | `.claude/skills/<이름>/SKILL.md` |
