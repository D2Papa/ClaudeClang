#!/usr/bin/env bash
# 테스트 실행: tdd / unit / integration / system 스위트를 각각 따로 실행하고 결과(JUnit XML)를 분리해 저장한다.
# 테스트 성공률 100%: 하나라도 실패하거나 실행된 스위트가 0개면 실패로 본다.
set -uo pipefail
cd "$(dirname "$0")/../.."

mkdir -p build/cov reports/test
status=0
ran=0
for suite in tdd unit integration system; do
  exe="build/${suite}_tests"
  if [ ! -x "$exe" ]; then
    echo "[SKIP] ${suite}: 테스트 실행 파일 없음 (${exe})"
    continue
  fi
  ran=$((ran + 1))
  echo "=== ${suite} 테스트 ==="
  if ! LLVM_PROFILE_FILE="build/cov/${suite}.profraw" "$exe" --gtest_output="xml:reports/test/${suite}.xml"; then
    echo "[FAIL] ${suite}: 실패한 테스트가 있다"
    status=1
  fi
done

if [ "$ran" -eq 0 ]; then
  echo "[FAIL] 실행된 테스트 스위트가 없다"
  status=1
fi
exit "$status"
