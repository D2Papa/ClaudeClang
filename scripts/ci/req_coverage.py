#!/usr/bin/env python3
"""시스템 테스트 요구사항 커버리지 검사 (CLAUDE.md 2.1: 요구사항 커버리지 100%).

범위 파일(test/system/requirements_in_scope.txt)의 모든 요구사항이
  - 시스템 테스트 JUnit XML(reports/test/system.xml)의 testcase의 requirements 속성(또는 property)에 있고 해당 테스트가 통과했거나,
  - 검사(inspect) 항목이면 지정한 파일의 정규식 조건을 만족해야 한다.
미달이면 종료 코드 1로 끝난다. 결과는 reports/requirements_coverage.txt 에도 저장한다.
"""
import pathlib
import re
import sys
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[2]
SCOPE = ROOT / "test" / "system" / "requirements_in_scope.txt"
XML = ROOT / "reports" / "test" / "system.xml"
REPORT = ROOT / "reports" / "requirements_coverage.txt"


def load_scope():
    items = []
    for line in SCOPE.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            items.append(line.split(None, 3))
    return items


def load_passed_requirements():
    """통과한 시스템 테스트가 기록한 요구사항 ID 집합과 실패한 테스트 목록."""
    passed, failed = set(), []
    if XML.exists():
        for case in ET.parse(XML).getroot().iter("testcase"):
            raw = case.get("requirements", "")
            for prop in case.iter("property"):
                if prop.get("name") == "requirements":
                    raw = prop.get("value", "")
            ids = [x.strip() for x in raw.split(",") if x.strip()]
            if case.find("failure") is not None:
                failed.append(case.get("name"))
            else:
                passed.update(ids)
    return passed, failed


def check_inspect(mode, path, pattern):
    text = (ROOT / path).read_text(encoding="utf-8") if (ROOT / path).exists() else ""
    found = re.search(pattern, text) is not None
    return found if mode == "inspect-present" else (not found)


def main() -> int:
    scope = load_scope()
    passed, failed = load_passed_requirements()
    lines, missing = [], []
    for item in scope:
        rid = item[0]
        if len(item) >= 4 and item[1].startswith("inspect"):
            ok = check_inspect(item[1], item[2], item[3])
            how = "검사"
        else:
            ok = rid in passed
            how = "시험"
        lines.append("%-12s %-4s %s" % (rid, how, "PASS" if ok else "FAIL"))
        if not ok:
            missing.append(rid)

    total = len(scope)
    covered = total - len(missing)
    percent = 100.0 * covered / total if total else 0.0
    summary = "요구사항 커버리지 %.1f%% (%d/%d)" % (percent, covered, total)
    out = lines + ["", summary]
    if failed:
        out.append("실패한 시스템 테스트: " + ", ".join(failed))
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text("\n".join(out) + "\n", encoding="utf-8")
    print("\n".join(out))
    if missing:
        print("[FAIL] 시스템 테스트가 검증하지 않은 요구사항: " + ", ".join(missing))
    return 1 if (missing or failed or percent < 100.0) else 0


if __name__ == "__main__":
    sys.exit(main())
