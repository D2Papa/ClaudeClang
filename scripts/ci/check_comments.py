#!/usr/bin/env python3
"""주석 규칙 검사: src 아래의 모든 파일·함수 주석에 Human in the Loop 정보(AI 작성 정보, 사용자 리뷰)가 있어야 한다.

판정 방식(휴리스틱):
  - 각 파일은 파일 주석(@file)을 가져야 한다.
  - 파일의 @brief 개수(파일 주석 + 함수 주석)만큼 '@par AI 작성 정보'와 '@par 사용자 리뷰' 블록이 있어야 한다.
  - '완료 여부'는 '완료' 또는 '미완료'이고, '완료'이면 리뷰일과 리뷰어가 '-'이 아니어야 한다.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
EXTS = {".c", ".h", ".cpp", ".hpp"}

AI_BLOCK = re.compile(
    r"@par AI 작성 정보\s*\n\s*\*\s*-\s*모델:\s*(?P<model>.+)\n\s*\*\s*-\s*작성일:\s*(?P<date>.+)\n\s*\*\s*-\s*요청자:\s*(?P<who>.+)"
)
REVIEW_BLOCK = re.compile(
    r"@par 사용자 리뷰\s*\n\s*\*\s*-\s*완료 여부:\s*(?P<done>.+)\n\s*\*\s*-\s*리뷰일:\s*(?P<date>.+)\n\s*\*\s*-\s*리뷰어:\s*(?P<who>.+)"
)


def check(path: pathlib.Path) -> list:
    text = path.read_text(encoding="utf-8")
    problems = []
    rel = path.relative_to(ROOT).as_posix()

    if "@file" not in text:
        problems.append("%s: 파일 주석(@file)이 없다" % rel)

    briefs = len(re.findall(r"@brief", text))
    ai_blocks = list(AI_BLOCK.finditer(text))
    review_blocks = list(REVIEW_BLOCK.finditer(text))

    if len(ai_blocks) < briefs:
        problems.append("%s: @brief %d개 중 'AI 작성 정보'가 %d개뿐이다" % (rel, briefs, len(ai_blocks)))
    if len(review_blocks) < briefs:
        problems.append("%s: @brief %d개 중 '사용자 리뷰'가 %d개뿐이다" % (rel, briefs, len(review_blocks)))

    for m in review_blocks:
        done = m.group("done").strip()
        if done not in ("완료", "미완료"):
            problems.append("%s: 완료 여부는 '완료' 또는 '미완료'여야 한다 (값: %s)" % (rel, done))
        if done == "완료" and "-" in (m.group("date").strip(), m.group("who").strip()):
            problems.append("%s: 리뷰 '완료'이면 리뷰일과 리뷰어가 있어야 한다" % rel)
    return problems


def main() -> int:
    files = sorted(p for p in (ROOT / "src").rglob("*") if p.suffix in EXTS)
    problems = []
    for f in files:
        problems += check(f)
    for p in problems:
        print("[FAIL] " + p)
    print("주석 검사: 파일 %d개, 위반 %d건" % (len(files), len(problems)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
