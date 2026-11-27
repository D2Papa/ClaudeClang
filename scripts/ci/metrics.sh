#!/usr/bin/env bash
# 소스 메트릭: Lizard (순환복잡도 10 이하, 함수 80라인 이하), CLOC (주석률 보고)
set -uo pipefail
cd "$(dirname "$0")/../.."
mkdir -p reports

status=0
# 결과 리포트 (임계값 초과 함수가 표시된다)
lizard src -C 10 -L 80 --xml > reports/lizard.xml
lizard src -C 10 -L 80 --html > reports/lizard.html
# 게이트: 경고가 하나라도 있으면 실패
if ! lizard src -C 10 -L 80 -w; then
  echo "[FAIL] Lizard 기준 초과 (CCN > 10 또는 함수 > 80라인)"
  status=1
fi

cloc src --by-file --xml --out=reports/cloc.xml
cloc src
exit "$status"
