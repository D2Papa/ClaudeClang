#!/usr/bin/env bash
# 전체 품질 게이트를 순서대로 실행한다. 실패해도 끝까지 실행해 모든 결과를 보고한다. (컨테이너 안에서 실행)
cd "$(dirname "$0")/../.."
rm -rf reports
mkdir -p reports

declare -A RESULT
STEPS=("빌드" "테스트" "커버리지" "요구사항 커버리지" "정적 분석" "소스 메트릭" "주석 규칙" "Doxygen")

step() {
  local name="$1"
  shift
  echo
  echo "##### ${name} #####"
  if "$@"; then
    RESULT["$name"]=PASS
  else
    RESULT["$name"]=FAIL
  fi
}

step "빌드" scripts/ci/build.sh
step "테스트" scripts/ci/test.sh
step "커버리지" scripts/ci/coverage.sh
step "요구사항 커버리지" python3 scripts/ci/req_coverage.py
step "정적 분석" scripts/ci/static.sh
step "소스 메트릭" scripts/ci/metrics.sh
step "주석 규칙" python3 scripts/ci/check_comments.py
step "Doxygen" scripts/ci/docs.sh

echo
echo "===== 품질 게이트 결과 ====="
rc=0
for name in "${STEPS[@]}"; do
  printf '%-12s %s\n' "$name" "${RESULT[$name]:-SKIP}"
  if [ "${RESULT[$name]:-SKIP}" != "PASS" ]; then
    rc=1
  fi
done
exit "$rc"
