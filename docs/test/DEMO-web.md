# 시연 기록: Web 시뮬레이터 (SRS-C-004)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-DEM-001 |
| 대상 | web_simulator (main: src/web/web_main.c, 소켓 처리: src/web/web_adapter.c, ARCH-C-016) |
| 일자 | 2026-11-27 |
| 기준 커밋 | 30e1789 (시연 후 변경은 주석 추가와 테스트 주석 보강뿐) |
| 환경 | claudeclangbuild 컨테이너, 포트 8081, curl |
| 시연 결과: 성공 | 아래 7개 시나리오가 모두 기대한 HTTP 상태와 내용을 보였다 |

## 재현 방법

```
docker build -t claudeclangbuild .
docker run --rm -v "${PWD}:/work" claudeclangbuild bash scripts/demo/web_demo.sh
```

## 시나리오와 기대 결과

| 번호 | 요청 | 기대 |
|------|------|------|
| 1 | GET /status (평가 전) | 204 |
| 2 | POST /step speed=3.0 | 200, lock_left/right LOCK, priority_reason AUTO_LOCK |
| 3 | POST /step 접근위험(좌)+unlock(좌) 두 번 | 첫 번째 APPROACH_RISK_SUPPRESSED, 두 번째 OVERRIDE(좌 RELEASE) |
| 4 | GET /history | 200, 변화마다 한 건 |
| 5 | GET / | 200, HTML (<form 포함) |
| 6 | GET /x, POST /status, POST /step bogus=1 | 404, 405, 400 |
| 7 | GET 경로 70자(63자 초과) | 404 (잘린 경로가 어떤 경로와도 같지 않음) |

실제 출력은 `scripts/demo/web_demo.sh`를 실행하면 `reports/web_demo.txt`에 전체가 남고, 마지막 실행 결과의 요약은 아래와 같다.

```
[OK] 1 GET /status (평가 전): 204
2 응답: {"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","priority_reason":"AUTO_LOCK","reason_code":"AUTO_LOCK","warning_code":"NONE","input_validity":{"speed":"VALID","gear":"VALID","source_timestamp":"VALID",...
[OK] 2 POST /step speed=3.0: 200
[OK] 2 priority_reason AUTO_LOCK
3 첫째: {"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","priority_reason":"APPROACH_RISK_SUPPRESSED","reason_code":"APPROACH_RISK_SUPPRESSED","warning_code":"NONE","input_validity":{"speed":"VALID","gear":"VALI...
3 둘째: {"lock_left":"RELEASE","lock_right":"LOCK","state":"NORMAL","priority_reason":"OVERRIDE","reason_code":"OVERRIDE","warning_code":"NONE","input_validity":{"speed":"VALID","gear":"VALID","source_timestamp":"VALID"...
[OK] 3 첫 요청 억제
[OK] 3 재입력 OVERRIDE
4 응답: [{"id":1,"timestamp_s":0.000,"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","reason_code":"AUTO_LOCK"},{"id":2,"timestamp_s":0.010,"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","reason_code":"...
[OK] 4 GET /history: 200
[OK] 5 GET /: 200
[OK] 5 HTML에 form 포함
[OK] 6a GET /x: 404
[OK] 6b POST /status: 405
[OK] 6c POST /step bogus=1: 400
[OK] 7 GET 70자 경로: 404
시연 결과: 성공
```

요청 처리 로직(`Web_Handle`)은 단위·통합·시스템 테스트로 검증했고, 소켓 처리(`web_adapter.c`)는 socketpair와 로컬 TCP 단위·통합 테스트로 검증했고, 이 시연은 `main`(포트 열기와 무한 루프)과 실제 프로세스 동작을 확인한다.
