#!/usr/bin/env python3
"""LLVM 소스 기반 커버리지 측정 스크립트 (구문/분기/MC/DC).

사용 예:
    python llvm_coverage.py --src src/a.c --test test/test_a.c --out build/cov
    python llvm_coverage.py --src src/a.c --test test/test_a.c --out build/cov --mcdc

흐름: clang으로 계측 빌드 -> 테스트 실행 -> llvm-profdata merge -> llvm-cov report/show
종료 코드: 0 = 성공(및 임계값 충족), 1 = 도구/빌드/실행 오류, 2 = 커버리지 임계값 미달
"""
import argparse
import json
import os
import shutil
import subprocess
import sys


def run(cmd, env=None, capture=False):
    """명령을 실행하고 (반환 코드, 출력)을 돌려준다."""
    print("$ " + " ".join(cmd))
    proc = subprocess.run(
        cmd, env=env, text=True,
        stdout=subprocess.PIPE if capture else None,
        stderr=subprocess.STDOUT if capture else None,
    )
    return proc.returncode, (proc.stdout or "")


def find_tool(name):
    """PATH에서 도구를 찾는다. 없으면 None."""
    return shutil.which(name)


def clang_major(clang):
    """clang 주 버전을 반환한다. 알 수 없으면 0."""
    rc, out = run([clang, "--version"], capture=True)
    result = 0
    if rc == 0:
        for token in out.replace("(", " ").split():
            if token[:1].isdigit() and "." in token:
                result = int(token.split(".")[0])
                break
    return result


def parse_args():
    ap = argparse.ArgumentParser(description="LLVM coverage runner")
    ap.add_argument("--src", nargs="+", required=True, help="측정 대상 소스 파일")
    ap.add_argument("--test", nargs="+", required=True, help="테스트 소스 파일 (main 포함)")
    ap.add_argument("--out", default="build/cov", help="출력 디렉터리")
    ap.add_argument("--include", nargs="*", default=[], help="추가 include 경로 (-I)")
    ap.add_argument("--std", default=None, help="언어 표준 (예: c11, c++17)")
    ap.add_argument("--mcdc", action="store_true", help="MC/DC 계측 및 리포트")
    ap.add_argument("--min-line", type=float, default=100.0, help="라인 커버리지 임계값(%%)")
    ap.add_argument("--min-branch", type=float, default=100.0, help="분기 커버리지 임계값(%%)")
    ap.add_argument("--clang", default="clang", help="clang 실행 파일")
    return ap.parse_args()


def percent(section, key):
    """llvm-cov export JSON 요약에서 퍼센트 값을 꺼낸다 (없으면 None)."""
    entry = section.get(key)
    return None if entry is None else float(entry.get("percent", 0.0))


def main():
    args = parse_args()
    result = 0

    clang = find_tool(args.clang)
    profdata = find_tool("llvm-profdata")
    cov = find_tool("llvm-cov")
    missing = [n for n, p in (("clang", clang), ("llvm-profdata", profdata), ("llvm-cov", cov)) if not p]
    if missing:
        print("오류: 다음 도구를 PATH에서 찾을 수 없습니다: " + ", ".join(missing))
        print("LLVM(clang, llvm-profdata, llvm-cov)을 설치한 뒤 다시 실행하세요.")
        return 1

    if args.mcdc and clang_major(clang) < 18:
        print("오류: MC/DC 계측(-fcoverage-mcdc)은 clang 18 이상이 필요합니다.")
        return 1

    os.makedirs(args.out, exist_ok=True)
    exe = os.path.join(args.out, "ut.exe" if os.name == "nt" else "ut")
    profraw = os.path.join(args.out, "ut.profraw")
    profdata_file = os.path.join(args.out, "ut.profdata")

    build = [clang, "-O0", "-g", "-fprofile-instr-generate", "-fcoverage-mapping"]
    if args.mcdc:
        build.append("-fcoverage-mcdc")
    if args.std:
        build.append("-std=" + args.std)
    build += ["-I" + p for p in args.include]
    build += args.src + args.test + ["-o", exe]

    rc, _ = run(build)
    if rc != 0:
        print("오류: 계측 빌드 실패")
        return 1

    env = dict(os.environ, LLVM_PROFILE_FILE=profraw)
    rc, _ = run([exe], env=env)
    test_failed = rc != 0
    if test_failed:
        print("경고: 테스트 실행이 실패했습니다 (종료 코드 %d). 커버리지는 계속 측정합니다." % rc)

    rc, _ = run([profdata, "merge", "-sparse", profraw, "-o", profdata_file])
    if rc != 0:
        print("오류: 프로파일 병합 실패")
        return 1

    common = [exe, "-instr-profile=" + profdata_file]
    # 테스트 파일은 측정 대상에서 제외하고 --src 파일만 측정한다.
    sources = [os.path.abspath(s) for s in args.src]

    print("\n=== 요약 리포트 ===")
    run([cov, "report"] + common + sources)

    show = [cov, "show"] + common + sources + [
        "--show-branches=count", "--show-line-counts-or-regions",
        "--format=html", "--output-dir=" + os.path.join(args.out, "html"),
    ]
    if args.mcdc:
        show.append("--show-mcdc")
    run(show)
    print("HTML 리포트: " + os.path.join(args.out, "html", "index.html"))

    if args.mcdc:
        print("\n=== MC/DC 상세 (텍스트) ===")
        run([cov, "show"] + common + sources + ["--show-mcdc", "--show-branches=count"])

    rc, out = run([cov, "export"] + common + ["-summary-only"] + sources, capture=True)
    if rc == 0:
        data = json.loads(out)
        totals = data["data"][0]["totals"]
        line = percent(totals, "lines")
        branch = percent(totals, "branches")
        print("\n라인 커버리지: %s%% (목표 %.1f%%)" % (line, args.min_line))
        print("분기 커버리지: %s%% (목표 %.1f%%)" % (branch, args.min_branch))
        if args.mcdc:
            mcdc = percent(totals, "mcdc")
            print("MC/DC 커버리지: %s%% (목표 100.0%%)" % mcdc)
            if mcdc is not None and mcdc < 100.0:
                result = 2
        if line is not None and line < args.min_line:
            result = 2
        if branch is not None and branch < args.min_branch:
            result = 2

    if test_failed and result == 0:
        result = 1
    return result


if __name__ == "__main__":
    sys.exit(main())
