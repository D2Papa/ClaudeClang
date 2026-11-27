#!/usr/bin/env python3
"""스위트별 커버리지 기준을 검사한다. 사용: coverage_gate.py <suite> <summary.json>"""
import json
import sys

# 스위트별 필수 기준(%). 기준이 없는 스위트는 보고만 한다.
GATES = {
    "unit": {"mcdc": 100.0},
    "integration": {"functions": 100.0},
}


def main() -> int:
    suite, path = sys.argv[1], sys.argv[2]
    with open(path, encoding="utf-8") as f:
        totals = json.load(f)["data"][0]["totals"]

    print("[%s] 커버리지 요약" % suite)
    for key in ("lines", "functions", "branches", "mcdc"):
        item = totals.get(key)
        if item is not None:
            print("  %-10s %6.2f%% (%d/%d)" % (key, item["percent"], item["covered"], item["count"]))

    failed = 0
    for key, minimum in GATES.get(suite, {}).items():
        item = totals.get(key)
        if item is None:
            print("[FAIL] %s: '%s' 커버리지 정보가 없다 (계측 옵션 확인)" % (suite, key))
            failed = 1
        elif item["percent"] < minimum:
            print("[FAIL] %s: %s 커버리지 %.2f%% < 기준 %.1f%%" % (suite, key, item["percent"], minimum))
            failed = 1
        else:
            print("[PASS] %s: %s 커버리지 %.2f%% >= 기준 %.1f%%" % (suite, key, item["percent"], minimum))
    if suite not in GATES:
        print("[INFO] %s: 별도 측정·보고만 한다 (합격 기준 없음)" % suite)
    return failed


if __name__ == "__main__":
    sys.exit(main())
