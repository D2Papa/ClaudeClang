#!/usr/bin/env bash
# Web 시뮬레이터 시연 (SRS-C-004): web_simulator를 띄워 7개 시나리오를 curl로 확인하고 reports/web_demo.txt에 저장한다. (컨테이너 안에서 실행)
cd "$(dirname "$0")/../.."
bash scripts/ci/build.sh >/dev/null 2>&1
mkdir -p reports
out=reports/web_demo.txt
fail=0
check() { # 설명 기대 실제
  if [ "$2" = "$3" ]; then echo "[OK] $1: $3" | tee -a "$out"; else echo "[FAIL] $1: 기대 $2, 실제 $3" | tee -a "$out"; fail=1; fi
}
: > "$out"
build/web_simulator 8081 >/tmp/web_simulator.log 2>&1 &
pid=$!
sleep 1
code() { curl -s -o /dev/null -w '%{http_code}' "$@"; }
check "1 GET /status (평가 전)" 204 "$(code localhost:8081/status)"
r2=$(curl -s -w '|%{http_code}' -X POST -d 'speed=3.0' localhost:8081/step)
echo "2 응답: $r2" >> "$out"
check "2 POST /step speed=3.0" 200 "${r2##*|}"
case "$r2" in *'"priority_reason":"AUTO_LOCK"'*) echo "[OK] 2 priority_reason AUTO_LOCK" | tee -a "$out";; *) echo "[FAIL] 2 priority_reason" | tee -a "$out"; fail=1;; esac
body='approach_left=1&cmd_side=0&cmd_action=1&cmd_source=0'
a=$(curl -s -X POST -d "$body" localhost:8081/step)
b=$(curl -s -X POST -d "$body" localhost:8081/step)
echo "3 첫째: $a" >> "$out"; echo "3 둘째: $b" >> "$out"
case "$a" in *APPROACH_RISK_SUPPRESSED*) echo "[OK] 3 첫 요청 억제" | tee -a "$out";; *) echo "[FAIL] 3 첫 요청" | tee -a "$out"; fail=1;; esac
case "$b" in *'"priority_reason":"OVERRIDE"'*) echo "[OK] 3 재입력 OVERRIDE" | tee -a "$out";; *) echo "[FAIL] 3 재입력" | tee -a "$out"; fail=1;; esac
h=$(curl -s -w '|%{http_code}' localhost:8081/history)
echo "4 응답: $h" >> "$out"
check "4 GET /history" 200 "${h##*|}"
check "5 GET /" 200 "$(code localhost:8081/)"
curl -s localhost:8081/ | grep -q '<form' && echo "[OK] 5 HTML에 form 포함" | tee -a "$out" || { echo "[FAIL] 5 HTML" | tee -a "$out"; fail=1; }
check "6a GET /x" 404 "$(code localhost:8081/x)"
check "6b POST /status" 405 "$(code -X POST localhost:8081/status)"
check "6c POST /step bogus=1" 400 "$(code -X POST -d bogus=1 localhost:8081/step)"
long=$(printf '/%070d' 0)
check "7 GET 70자 경로" 404 "$(code "localhost:8081$long")"
kill "$pid" 2>/dev/null
[ "$fail" -eq 0 ] && echo "시연 결과: 성공" | tee -a "$out" || echo "시연 결과: 실패" | tee -a "$out"
exit "$fail"
