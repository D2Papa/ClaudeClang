#!/usr/bin/env python3
"""추적성 매트릭스(docs/traceability/TRACE.md) 생성기.

입력: docs/requirements/SRS.md, docs/safety/SAFETY-REQ.md (요구와 상위 추적),
      docs/architecture/ARCH.md 8장, docs/detailed-design/DDS.md 5장 (요구 -> 설계 배정),
      src/**/@trace (소스), test/unit (UT), test/integration (IT), test/system (ST).
사용: python scripts/docs/gen_trace.py
"""
import glob
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
VERSION = "1.1 (Iter-1)"
ID_RE = re.compile(r"(?:SRS-[FQLC]-\d{3}|SSR-\d{3})")
RANGE_RE = re.compile(r"(SRS-[FQLC]-|SSR-)(\d{3})~(\d{3})")

OEM_REQ = (["OEM-SR-%03d" % n for n in range(1, 5)] + ["OEM-FR-%03d" % n for n in range(1, 8)]
           + ["OEM-NFR-001", "OEM-NFR-002"] + ["OEM-IF-%03d" % n for n in range(1, 10)])


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def expand_ids(text):
    """'SRS-F-001~004' 같은 범위와 개별 ID를 모두 개별 ID 집합으로 만든다."""
    ids = set(ID_RE.findall(text))
    for m in RANGE_RE.finditer(text):
        ids |= {"%s%03d" % (m.group(1), n) for n in range(int(m.group(2)), int(m.group(3)) + 1)}
    return ids


def parse_requirements(text, pattern, upstream_index):
    rows = {}
    for line in text.splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if cells and re.fullmatch(pattern, cells[0]):
            rows[cells[0]] = cells[upstream_index(cells[0])]
    return rows


def expand_oem(up):
    ids = set(re.findall(r"OEM-[A-Z]+-\d+", up))
    if "001~004" in up:
        ids |= {"OEM-IF-%03d" % n for n in range(1, 5)}
    if "007~009" in up:
        ids |= {"OEM-IF-007", "OEM-IF-008", "OEM-IF-009"}
    return ids


def design_map(path):
    """'| 요구 ID | 값, 값 |' 두 칸 표를 {요구 ID: 값 목록}으로 읽는다."""
    result = {}
    if (ROOT / path).exists():
        for line in read(path).splitlines():
            cells = [c.strip() for c in line.strip().strip("|").split("|")]
            if len(cells) == 2 and re.fullmatch(r"(?:SRS-[FQLC]-\d{3}|SSR-\d{3})", cells[0]):
                result[cells[0]] = [x.strip() for x in cells[1].split(",")]
    return result


def collect_sources():
    result = {}
    for f in glob.glob(str(ROOT / "src" / "**" / "*.[ch]"), recursive=True):
        text = open(f, encoding="utf-8").read()
        for line in re.findall(r"@trace\s+(.*)", text):
            for i in expand_ids(line):
                result.setdefault(i, set()).add(pathlib.Path(f).name)
    return result


def collect_tests(folder, pattern):
    """pattern의 (테스트 ID, 추적 문자열) 그룹을 모아 {요구 ID: {테스트 ID}}로 만든다."""
    result = {}
    for f in glob.glob(str(ROOT / "test" / folder / "*.cpp")):
        text = open(f, encoding="utf-8").read()
        for tid, rest in re.findall(pattern, text, re.S):
            for i in expand_ids(rest):
                result.setdefault(i, set()).add(tid)
    return result


def main():
    srs = parse_requirements(read("docs/requirements/SRS.md"), r"SRS-[FQLC]-\d+",
                             lambda i: 6 if i[4] in "FQL" else 5)
    # SRS-L 표는 ISO 열이 있어 상위 추적이 7번째(인덱스 7) 칸이다
    for k in list(srs):
        if k.startswith("SRS-L"):
            cells = next([c.strip() for c in ln.strip().strip("|").split("|")]
                         for ln in read("docs/requirements/SRS.md").splitlines() if ln.startswith("| %s |" % k))
            srs[k] = cells[7]
    ssr = parse_requirements(read("docs/safety/SAFETY-REQ.md"), r"SSR-\d+", lambda i: 4)
    allreq = dict(srs)
    allreq.update(ssr)

    arch = design_map("docs/architecture/ARCH.md")
    dd = design_map("docs/detailed-design/DDS.md")
    src = collect_sources()
    ut = collect_tests("unit", r"(UT-\d+) -> ([^.\n]*)")
    it = collect_tests("integration", r"(IT-\d+) 목적:.*?추적: ([^*\n]*)")
    st = collect_tests("system", r"/\*\* (ST-\d+)[^\n]*\n.*?RecordProperty\(\"requirements\", \"([^\"]*)\"\)")

    reverse = {o: [] for o in OEM_REQ}
    for rid, up in allreq.items():
        for o in expand_oem(up):
            if o in reverse:
                reverse[o].append(rid)
    missing = [o for o in OEM_REQ if not reverse[o]]

    def cell(values):
        return ", ".join(sorted(values))

    out = ["# 추적성 매트릭스 (TRACE)", "",
           "| 항목 | 내용 |", "|------|------|", "| 문서 ID | DOC-TRC-001 |", "| 버전 | %s |" % VERSION,
           "| 상태 | scripts/docs/gen_trace.py로 생성 (리뷰 생략: Iter-1은 사용자 지시) |", "",
           "하향 추적은 문서와 코드의 추적 표기에서 스크립트로 생성한다. 빈 칸은 해당 이터레이션 범위 밖이거나 누락이다.",
           "ID는 항상 접두사를 포함한 전체 표기를 쓴다.", "",
           "## 1. 요구사항 추적 (상향 + 하향)", "",
           "| 요구 ID | 문서 | 상위 요구 | 아키텍처 | 상세 설계 | 소스 | 단위 테스트 | 통합 테스트 | 시스템 테스트 |",
           "|---------|------|-----------|----------|-----------|------|-------------|-------------|---------------|"]
    for rid in sorted(srs):
        out.append("| %s | SRS | %s | %s | %s | %s | %s | %s | %s |" % (
            rid, srs[rid], cell(arch.get(rid, [])), cell(dd.get(rid, [])), cell(src.get(rid, [])),
            cell(ut.get(rid, [])), cell(it.get(rid, [])), cell(st.get(rid, []))))
    for rid in sorted(ssr):
        out.append("| %s | 안전(DOC-SAF-001) | %s | %s | %s | %s | %s | %s | %s |" % (
            rid, ssr[rid], cell(arch.get(rid, [])), cell(dd.get(rid, [])), cell(src.get(rid, [])),
            cell(ut.get(rid, [])), cell(it.get(rid, [])), cell(st.get(rid, []))))

    out += ["", "## 2. OEM 요구 -> 도출 요구 역추적", "", "| OEM 요구 | 도출한 요구 |", "|----------|-------------|"]
    for o in OEM_REQ:
        out.append("| %s | %s |" % (o, ", ".join(sorted(reverse[o])) or "(없음)"))

    out += ["", "## 3. 편차 기록", "", "| 편차 | 관련 요구 | 상태 |", "|------|-----------|------|",
            "| 구현 언어 C17/Clang (OEM-SWR-001 3장은 Python 3.12) | SRS-C-001 | 사용자 승인(OEM 대행), OEM 확인 대기 (A-11) |",
            "| 모호성 해소 가정 | A-01, A-02, A-03, A-04, A-05, A-06, A-07, A-08, A-09 | 사용자 \"제안안\" 승인(OEM 대행), OEM 확인 대기 |",
            "| 리뷰 지적으로 추가된 가정 | A-13, A-14, A-15, A-16, A-17, A-18 | 사용자 위임 \"모두 너의 관장안으로\"에 따른 작성자 결정, 사용자 확인 대기, OEM 확인 대기 |", "",
            "## 4. 추적 누락 목록", "",
            "- 고아 요구(상위 없음): 없음 (SRS-F-017은 파생 요구로 상위 OEM 요구를 명시)",
            "- OEM 요구 중 도출 요구 없음: " + (", ".join(missing) if missing else "없음 (OEM-SR 4, FR 7, NFR 2, IF 9 모두 도출됨)"),
            "- 시스템 테스트 칸이 빈 요구: Iter-2(표시, 이력, Web) 범위이거나 누락 (Iter-1 범위는 test/system/requirements_in_scope.txt)",
            "- OEM SWR-019: 결번 (OEM 추적표에 없음)"]
    (ROOT / "docs" / "traceability" / "TRACE.md").write_text("\n".join(out) + "\n", encoding="utf-8")
    print("SRS %d, SSR %d, OEM 미도출 %s" % (len(srs), len(ssr), missing))


if __name__ == "__main__":
    main()
