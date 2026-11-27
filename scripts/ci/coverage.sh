#!/usr/bin/env bash
# 커버리지: LLVM으로 스위트별 따로 측정한다. (TDD 테스트와 단위 테스트의 커버리지를 합산하지 않는다.)
#   unit        : MC/DC 100%
#   integration : 함수 커버리지 100%
#   tdd         : 별도 측정·보고만 한다 (기준 없음)
#   system      : 요구사항 커버리지는 system-test-skill 산출물로 확인한다 (코드 커버리지 기준 없음)
set -uo pipefail
cd "$(dirname "$0")/../.."

mapfile -t SOURCES < <(find src -name '*.c' | sort)
if [ "${#SOURCES[@]}" -eq 0 ]; then
  echo "[SKIP] 측정할 소스(src/**/*.c)가 없다"
  exit 0
fi

mkdir -p reports/coverage
status=0
for suite in tdd unit integration system; do
  exe="build/${suite}_tests"
  raw="build/cov/${suite}.profraw"
  if [ ! -x "$exe" ] || [ ! -f "$raw" ]; then
    continue
  fi
  data="build/cov/${suite}.profdata"
  out="reports/coverage/${suite}"
  mkdir -p "$out"

  if ! llvm-profdata merge -sparse "$raw" -o "$data"; then
    status=1
    continue
  fi
  llvm-cov report "$exe" -instr-profile="$data" "${SOURCES[@]}" | tee "$out/report.txt"
  llvm-cov export "$exe" -instr-profile="$data" -summary-only "${SOURCES[@]}" > "$out/summary.json"
  llvm-cov export "$exe" -instr-profile="$data" -format=lcov "${SOURCES[@]}" > "$out/coverage.lcov"
  lcov_cobertura "$out/coverage.lcov" --base-dir . --output "$out/cobertura.xml" > /dev/null
  llvm-cov show "$exe" -instr-profile="$data" --format=html --output-dir="$out/html" \
    --show-branches=count --show-mcdc "${SOURCES[@]}" > /dev/null

  python3 scripts/ci/coverage_gate.py "$suite" "$out/summary.json" || status=1
done
exit "$status"
