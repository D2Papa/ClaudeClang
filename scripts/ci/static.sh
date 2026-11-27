#!/usr/bin/env bash
# 정적 분석: Cppcheck 룰 100% 만족. 예외 처리(suppress)는 허용하지 않는다.
set -uo pipefail
cd "$(dirname "$0")/../.."
mkdir -p reports

status=0
if grep -rnE 'cppcheck-suppress|cppcheck:disable' src; then
  echo "[FAIL] Cppcheck 예외 처리(suppress)는 허용되지 않는다. 코드를 고쳐서 해결한다."
  status=1
fi

if ! cppcheck --enable=warning,style,performance,portability --check-level=exhaustive \
    --std=c11 --language=c -I src --error-exitcode=1 \
    --xml --output-file=reports/cppcheck.xml src; then
  echo "[FAIL] Cppcheck 경고가 있다"
  grep '<error ' reports/cppcheck.xml || true
  status=1
fi

echo "Cppcheck 경고 수: $(grep -c '<error ' reports/cppcheck.xml || true)"
exit "$status"
