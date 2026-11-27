---
name: changelog
description: git 커밋 이력을 바탕으로 CHANGELOG.md 항목을 생성·갱신한다. 릴리스 준비, 변경 이력 정리, "변경 사항 정리해줘" 같은 요청에 사용한다.
---

# changelog

## 입력

- 인자로 범위를 받을 수 있다. 예: `/changelog v1.2.0..HEAD`, `/changelog 1.3.0`
- 범위가 없으면 마지막 태그(`git describe --tags --abbrev=0`)부터 `HEAD`까지 사용한다.
  태그가 없으면 전체 이력을 사용한다.

## 절차

1. `git log <범위> --pretty=format:"%h %s"`로 커밋을 모은다.
2. Conventional Commits의 type 기준으로 분류한다.
   - feat → **추가됨**, fix → **수정됨**, refactor/perf → **변경됨**, 그 밖의 항목(docs/chore/test)은 사용자에게 보이는 변화가 있을 때만 포함
   - 형식을 따르지 않는 커밋은 메시지 내용으로 판단해 분류한다.
3. 사용자 관점의 문장으로 다듬고, 중복이나 사소한 커밋(오타 수정 등)은 합친다.
4. `CHANGELOG.md` 맨 위(제목 아래)에 새 섹션을 추가한다. 파일이 없으면 만든다.

## 섹션 형식 (Keep a Changelog)

```markdown
## [버전 또는 Unreleased] - YYYY-MM-DD

### 추가됨
- ...

### 변경됨
- ...

### 수정됨
- ...
```

내용이 없는 하위 섹션은 생략한다. 작성 후 추가한 섹션을 사용자에게 보여준다.
