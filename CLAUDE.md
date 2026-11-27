# CLAUDE.md

이 파일은 Claude Code가 이 저장소에서 작업할 때 항상 읽는 프로젝트 지침입니다.

## 프로젝트 개요

<!-- TODO: 프로젝트가 무엇을 하는지 2~3문장으로 설명 -->
A-SPICE 기준의 C/C++(Clang) 소프트웨어 개발 프로젝트입니다. Claude Code의 서브에이전트(`.claude/agents/`)와
스킬(`.claude/skills/`)로 개발 절차를 수행하고, Jenkins CI로 품질 기준을 검증합니다.

## 자주 쓰는 명령

모든 빌드·검사 명령은 `claudeclangbuild` 컨테이너 안에서 실행한다 (PowerShell 기준, 저장소 루트에서).

| 작업 | 명령 |
|------|------|
| 빌드 이미지 생성 | `docker build -t claudeclangbuild .` |
| 컨테이너 셸 | `docker run --rm -it -v "${PWD}:/work" claudeclangbuild` |
| 전체 품질 게이트 (CI와 동일) | `docker run --rm -v "${PWD}:/work" claudeclangbuild bash scripts/ci/run_all.sh` |
| 빌드 | `... claudeclangbuild bash scripts/ci/build.sh` |
| 테스트 (tdd/unit/integration/system 분리 실행) | `... claudeclangbuild bash scripts/ci/test.sh` |
| 커버리지 (스위트별 분리, MC/DC 포함) | `... claudeclangbuild bash scripts/ci/coverage.sh` |
| 시스템 테스트 요구사항 커버리지 | `... claudeclangbuild python3 scripts/ci/req_coverage.py` (범위: `test/system/requirements_in_scope.txt`) |
| 정적 분석 (Cppcheck) | `... claudeclangbuild bash scripts/ci/static.sh` |
| 소스 메트릭 (Lizard, CLOC) | `... claudeclangbuild bash scripts/ci/metrics.sh` |
| 주석 규칙 / Doxygen | `... claudeclangbuild python3 scripts/ci/check_comments.py` / `bash scripts/ci/docs.sh` |
| Jenkins 시작 (내부 정기 점검) | `docker compose -f ci/jenkins/docker-compose.yml up -d` → http://localhost:8080 |

(`...` 는 `docker run --rm -v "${PWD}:/work"` 를 뜻한다. 결과물은 `reports/`, 빌드 산출물은 `build/`에 생성된다.)

## 디렉터리 구조

```
.
├── CLAUDE.md                 # 프로젝트 지침 (이 파일)
├── .claude/
│   ├── settings.json         # 팀 공용 권한·설정·Hook (커밋 대상)
│   ├── settings.local.json   # 개인 설정 (gitignore, 직접 생성)
│   ├── agents/               # 서브에이전트 정의 (*.md)
│   └── skills/               # 스킬 (<이름>/SKILL.md)
├── docs/                     # 산출물 (requirements, architecture, detailed-design, reviews, traceability, test), Doxyfile
├── src/                      # 소스 코드 (clamp/ 는 파이프라인 검증용 샘플 — 실제 개발 시작 시 삭제)
├── test/
│   ├── tdd/                  # TDD 테스트 (구현 단계 산출, 단위 테스트와 분리)
│   ├── unit/                 # 단위 테스트 (SWE.4)
│   ├── integration/          # 통합 테스트 (SWE.5)
│   └── system/               # 시스템 테스트 (SWE.6)
├── scripts/
│   ├── ci/                   # 품질 게이트 스크립트 (빌드/테스트/커버리지/정적분석/메트릭/주석)
│   └── hooks/                # Claude Code Hook (lizard_check.py)
├── .github/workflows/ci.yml  # GitHub Actions CI (PR 머지 조건)
├── ci/jenkins/               # Jenkins 이미지·플러그인·JCasC·docker-compose
├── Dockerfile                # claudeclangbuild 이미지 (개발·CI 공용)
├── CMakeLists.txt
├── Jenkinsfile
└── README.md
```

## 작업 규칙

- 응답과 주석은 한국어로 작성하고, 코드 식별자와 기술 용어는 원문을 유지합니다.
- 변경은 요청 범위 안에서 최소한으로 하고, 관련 없는 리팩터링은 하지 않습니다.
- 코드를 수정한 뒤에는 빌드·테스트·정적 분석 명령을 실행해 결과를 확인합니다.
- 커밋 메시지는 `commit` 스킬의 규칙(Conventional Commits)을 따릅니다.
- 비밀값(.env, 키, 토큰)은 커밋하지 않습니다.

---

## 1. 개발 절차

### 1.1 기본 원칙

- 개발은 **A-SPICE**를 기준으로 한다.
- 개발 생명주기는 **애자일**이다. **분석 → 아키텍처 설계 → 상세 설계 → 구현 → 단위/통합/시스템 테스트를 하나의 이터레이션**으로 한다.
- 개발은 기본적으로 **메인 에이전트**가 수행하며, 각 단계에 맞는 스킬을 사용한다.
- 모든 요구사항은 이후 산출물(아키텍처 → 상세 설계 → 소스 → 테스트)과 **양방향 추적성**을 유지한다 (`docs/traceability/TRACE.md`).

### 1.2 이터레이션 단계와 스킬

| 단계 | 스킬 | A-SPICE | 핵심 규칙 |
|------|------|---------|-----------|
| 1. 분석 | `requirements-analysis` | SWE.1 | 기능/비기능(품질·제한·제약) 분류, 품질은 ISO 25010, 기능 안전은 별도 문서 |
| 2. 아키텍처 설계 | `architecture-design` | SWE.2 | 컴포넌트·인터페이스 필수 정의, 인터페이스로만 통신, 고응집·저결합, SOLID |
| 3. 상세 설계 | `detailed-design` | SWE.3 | 모듈 정의, 기준 이상 모듈은 인터페이스 필수, 클린코드, 단일 책임 |
| 4. 구현 | `test-driven-development` + `implementation` | SWE.3 | TDD로 구현, `void` 금지(상태 코드 반환), `return`은 함수 마지막에 한 번만 |
| 5. 단위 테스트 | `unit-test` | SWE.4 | MC/DC 100%, 성공률 100%, Doxygen 주석, LLVM 스크립트 |
| 6. 통합 테스트 | `integration-test` | SWE.5 | 인터페이스 중심, 함수 커버리지 100%, 성공률 100% |
| 7. 시스템 테스트 | `system-test-skill` | SWE.6 | 요구사항 커버리지 100% |
| 전 단계 | `review` | — | 산출물별 체크리스트(`.claude/skills/review/checklists.md`) 기반 리뷰 |

### 1.3 TDD 구현

- 구현은 `test-driven-development` 스킬로 **TDD 방식**(테스트 먼저 → 실패 확인 → 구현 → 통과 → 리팩터링)으로 한다.
- TDD 과정에서 만든 테스트는 **단위 테스트(`test/unit/`)와 분리**해 `test/tdd/`에 둔다.
- TDD 테스트는 **별도의 결과(리포트)로 도출**하고 **커버리지도 별도로 측정**한다. 단위 테스트의 결과·커버리지에 합산하지 않는다.
- 단위 테스트는 TDD 테스트를 재사용하지 않고, `unit-test` 스킬로 상세 설계 기준에서 독립적으로 설계한다.

### 1.4 단계 게이트 (필수)

각 단계가 완료되면 **`aspice-reviewer`(Reviewer 서브에이전트)가 산출물을 리뷰**한다.

1. 단계 산출물을 완료하면 `aspice-reviewer`를 호출한다.
2. 판정이 **반려**면 보고된 부적합을 **중대·경미 구분 없이 모두** 조치한다.
3. 조치 후 `aspice-reviewer`로 재리뷰한다. 판정이 **승인**일 때까지 2~3을 반복한다.
4. **승인 전에는 다음 단계로 넘어가지 않는다.** 부적합을 사용자 승인 없이 "수용"하거나 보류하지 않는다.
   조치가 불가능하다고 판단되면 다음 단계로 넘어가지 말고 사유와 함께 사용자에게 알린다.

### 1.5 이터레이션 완료와 PR

1. 이터레이션의 모든 단계가 승인되면 **사용자에게 이터레이션 결과를 보고하고 확인을 받는다.**
2. **사용자 확인 이후에만** PR 절차(3장)를 진행한다. 확인 전에는 PR을 만들지 않고 `develop`에 머지하지 않는다.

---

## 2. 소스코드 품질 기준

모든 기준은 **예외 없이** 만족해야 한다. 미달 시 이터레이션을 완료할 수 없고 PR을 머지할 수 없다.

### 2.1 테스트 커버리지

| 테스트 | 기준 |
|--------|------|
| 시스템 테스트 | **요구사항 커버리지 100%** |
| 통합 테스트 | **함수 커버리지 100%**, **테스트 성공률 100%** |
| 단위 테스트 | **MC/DC 커버리지 100%**, **테스트 성공률 100%** |

- 커버리지는 LLVM(`llvm-cov`)으로 측정한다 (MC/DC 포함).
- TDD 테스트의 결과와 커버리지는 별도로 측정·보고한다 (1.3).

### 2.2 정적 분석

- **Cppcheck** 룰 대상을 **100% 만족**한다. **예외 처리(suppress)는 허용하지 않는다.**
- 소스 코드에 `// cppcheck-suppress`를 추가하거나 suppress 파일로 경고를 숨기지 않는다. 경고는 코드를 고쳐서 해결한다.

### 2.3 소스코드 메트릭

| 항목 | 기준 | 도구 |
|------|------|------|
| 순환복잡도(CCN) | **10 이하** | Lizard |
| 함수 라인 수 | **80라인 이하** | Lizard |

- 구현이 끝났을 때 **Hook으로 Lizard 검사를 자동 실행**해야 한다 (`.claude/settings.json`에 설정).
- 기준 초과 시 구현을 완료로 보지 않고 함수를 분리해 해소한다.

### 2.4 주석

- 주석은 **Doxygen 형식**으로 작성한다.
- 모든 소스 파일과 함수 주석에 다음 **Human in the Loop 정보**를 반드시 포함한다. 이 정보도 Doxygen 형식을 따른다.
  - 어느 **AI 모델**이, **언제**, **누가 요청**해서 작성했는지
  - **사용자 리뷰 완료 여부**. 완료했다면 **언제, 누가** 했는지

```c
/**
 * @brief 센서 값을 필터링하여 출력 버퍼에 저장한다.
 * @param[in]  raw  원시 센서 값 (0..4095)
 * @param[out] out  필터링된 값 (NULL 불가)
 * @return STATUS_OK 성공 / STATUS_ERR_PARAM 입력 오류
 * @trace DD-F-012, SRS-F-007
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
```

- 규칙
  - AI가 작성한 코드의 초기 상태는 반드시 **"미완료"** 이다.
  - **AI는 "완료"로 바꾸지 않는다.** 사용자가 리뷰하고 확인했다고 말한 경우에만, 사용자가 알려준 리뷰어·일자로 갱신한다.
  - 리뷰 완료된 코드를 AI가 수정하면 **"미완료"로 되돌리고** 수정한 모델·일자를 갱신한다.
  - 사용자 리뷰는 이터레이션 완료 확인(1.5) 때 함께 받는다.

---

## 3. Git 브랜치 및 PR 전략

### 3.1 브랜치: Gitflow

| 브랜치 | 용도 | 분기 → 머지 대상 |
|--------|------|------------------|
| `main` | 릴리스 이력 (태그) | — |
| `develop` | 통합 개발 | — |
| `feature/<이터레이션-또는-기능>` | 이터레이션·기능 개발 | `develop` → `develop` |
| `release/<버전>` | 릴리스 준비 | `develop` → `main`, `develop` |
| `hotfix/<설명>` | 운영 긴급 수정 | `main` → `main`, `develop` |

- `main`, `develop`에는 직접 커밋·푸시하지 않고 PR로만 머지한다.
- 이터레이션 하나는 보통 `feature/iter-<번호>-<주제>` 브랜치 하나로 진행한다.

### 3.2 PR과 CI

- PR은 **CI(GitHub Actions, 5장)를 활용**하며, **CI를 통과해야만** PR을 완료(머지)한다.
- CI 통과 조건은 **빌드 성공 + 소스코드 품질 기준(2장) 전체 만족**이다.
  - 단위·통합·시스템 테스트 (성공률, 커버리지 기준)
  - Cppcheck 100% 만족
  - Lizard (CCN 10 이하, 함수 80라인 이하)
  - 주석 규칙 (Doxygen, HITL 정보 포함)
- PR은 **사용자 확인(1.5) 이후**에만 만든다. CI 실패 시 원인을 고쳐 다시 푸시하며, 검사를 약화·우회하지 않는다.

---

## 4. 개발 환경

| 항목 | 도구 |
|------|------|
| 개발 언어 | C/C++, **Clang 18 이상**의 최신 안정 버전 (Docker 이미지에서 버전 고정) |
| 빌드 도구 | **CMake + Ninja** (추천) |
| 정적 분석 | **Cppcheck** |
| 테스트 프레임워크 | **Google Test / Google Mock** |
| 커버리지 | **LLVM** (`llvm-profdata`, `llvm-cov`, MC/DC 포함) |
| 순환복잡도·함수 라인 수 | **Lizard** |
| 주석률 | **CLOC** |
| 문서 생성 | Doxygen (주석 검증 포함) |
| 테스트 조합 설계 | PICT |
| 코드 포맷 | clang-format |
| 보조 분석 | clang-tidy (Cppcheck을 대체하지 않고 보조) |

### Docker 이미지

- **개발 빌드와 CI 빌드 모두 같은 Docker 이미지**를 사용한다.
- 이미지 이름은 **`claudeclangbuild`** 이다. (요청한 `ClaudeCLangBuild`는 Docker 이미지 이름 규칙상 대문자를 쓸 수 없어 소문자로 쓴다.)
- 위 도구 전체(Clang, CMake, Ninja, Cppcheck, GoogleTest, LLVM, Lizard, CLOC, Doxygen, PICT, clang-format, clang-tidy)를 이미지에 포함한다.
- 빌드·테스트·분석 명령은 이 이미지의 컨테이너 안에서 실행한다. 호스트 PC에 도구를 따로 설치하지 않는다.

---

## 5. CI 환경

### 5.1 GitHub Actions (PR 머지 조건)

- CI는 **GitHub Actions**이며 `.github/workflows/ci.yml`에 정의한다. `main`/`develop` 대상 PR과 `feature/**`, `release/**`, `hotfix/**` 푸시에서 실행한다.
- `claudeclangbuild` 이미지를 빌드해 `scripts/ci/run_all.sh`(7개 게이트)를 실행하고, `reports/`를 아티팩트로 올린다. 개발 환경과 같은 이미지·스크립트를 쓴다.
- PR 완료의 필수 상태 체크는 **`quality-gate`** 이다. 저장소 설정에서 `main`, `develop`에 브랜치 보호(필수 체크 `quality-gate`)를 걸어야 강제된다 (미설정).

### 5.2 Jenkins (내부 정기 점검)

- 내부에서도 **Jenkins 안정화(LTS) 버전**을 **주기적으로 실행**한다. 이 PC의 **Docker 위에서 실행**하고 포트는 **8080**이다.
- `Jenkinsfile`의 `cron('H H/4 * * *')`로 변경이 없어도 약 4시간마다 실행한다. 브랜치 탐색(스캔)은 1시간 간격이다.
- 필요한 플러그인을 **설치 완료한 상태**로 구성하고, 테스트·정적 분석·커버리지·메트릭 결과를 **플러그인에서 확인**할 수 있다.
  (Warnings Next Generation, JUnit, Coverage, HTML Publisher 등)
- 구성 파일: `ci/jenkins/` (Dockerfile, `plugins.txt`, `casc.yaml`, `docker-compose.yml`), 루트의 `Jenkinsfile`. 빌드 에이전트는 `claudeclangbuild` 이미지를 쓴다.
- 관리자 계정은 `admin`이며 비밀번호는 `ci/jenkins/.env`(gitignore, 읽기 차단)에 있다. `env.example`을 복사해 만든다.
- 이 PC의 Jenkins는 GitHub에서 접근할 수 없어 웹훅을 쓰지 않고, PR 상태도 Jenkins가 아니라 Actions가 표시한다.

---

## 서브에이전트

| 이름 | 용도 |
|------|------|
| `aspice-reviewer` | A-SPICE 산출물 리뷰 (`review` 스킬 적용, 읽기·보고서 작성만 가능, 엄격 모드, 모델 haiku) |
| `code-reviewer` | 변경된 코드의 버그·보안·가독성 리뷰 (읽기 전용) |
| `test-runner` | 테스트 실행, 실패 원인 분석 및 최소 수정 |
| `doc-writer` | README·주석·변경 이력 등 문서 작성 |

## 스킬

| 이름 | 용도 |
|------|------|
| `requirements-analysis`, `architecture-design`, `detailed-design`, `implementation`, `unit-test`, `integration-test`, `system-test-skill`, `review` | A-SPICE 개발 스킬 (1.2 참조) |
| `test-driven-development` | TDD 구현 |
| `commit` | 변경 사항을 분석해 Conventional Commits 형식으로 커밋 |
| `changelog` | git 이력으로 CHANGELOG 항목 생성 |
| `new-agent` | 새 서브에이전트 파일 생성 |
| `new-skill` | 새 스킬 디렉터리와 SKILL.md 생성 |
