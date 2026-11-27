# 시연 기록: Web 시뮬레이터 (SRS-C-004)

| 항목 | 내용 |
|------|------|
| 문서 ID | DOC-DEM-001 |
| 대상 | web_simulator (src/web/web_main.c, ARCH-C-016 어댑터) |
| 일자 | 2026-11-27 |
| 환경 | claudeclangbuild 컨테이너, 포트 8081, curl |
| 시연 결과: 성공 | 아래 6개 시나리오가 모두 기대한 HTTP 응답과 내용을 보였다 |

```
1) GET /status (평가 전): HTTP 204
2) POST /step speed=3.0:
{"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","priority_reason":"AUTO_LOCK","reason_code":"AUTO_LOCK","warning_code":"NONE","input_validity":{"speed":"VALID","gear":"VALID","source_timestam
HTTP 200
3) POST /step (접근위험 좌 + unlock 좌, 두 번):
{"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","priority_reason":"APPROACH_RISK_SUPPRESSED","reason_code":"APP

{"lock_left":"RELEASE","lock_right":"LOCK","state":"NORMAL","priority_reason":"OVERRIDE","reason_code":"OVERRIDE","warni

4) GET /history:
[{"id":1,"timestamp_s":0.000,"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","reason_code":"AUTO_LOCK"},{"id":2,"timestamp_s":0.010,"lock_left":"LOCK","lock_right":"LOCK","state":"NORMAL","reason_code":"APPROACH_RISK_SUPPRESSED"},{"id":3,"timestamp_s":0.020,"lock_left":"RELEASE","lock_right":"LOCK","state":"NORMAL","reason_code":"OVERRIDE"}]
HTTP 200
5) GET / (HTML 바이트): 1490
6) 오류 응답: GET /x 404 , POST /status 405 , 잘못된 본문 400
```

기대 결과: (1) 평가 전 204, (2) AUTO_LOCK과 입력 유효성이 포함된 JSON 200, (3) 접근위험 중 unlock이 억제된 뒤 재입력으로 OVERRIDE, (4) 변화마다 한 건씩 이력 JSON 200, (5) 시험 화면 HTML, (6) 404, 405, 400.
요청 처리 로직(`Web_Handle`)은 단위·통합·시스템 테스트로 검증했고, 이 시연은 소켓 어댑터(`web_main.c`)를 검증한다.
