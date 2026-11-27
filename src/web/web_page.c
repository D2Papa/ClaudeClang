/**
 * @file    web_page.c
 * @brief   시험 화면(HTML) 구현.
 * @trace   SRS-C-004
 *
 *
 * @par AI 작성 정보
 *   - 모델: Claude Sonnet 5.5 (claude-sonnet-5-5)
 *   - 작성일: 2026-11-27
 *   - 요청자: KYLee
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
 */
#include "web/web_page.h"
#include <stddef.h>
#include <string.h>

/** 시험 화면: 입력 16개 필드와 결과 표시 (fetch 사용) */
static const char WEB_PAGE[] =
    "<!doctype html><html lang=\"ko\"><head><meta charset=\"utf-8\"><title>ClaudeClang 차일드락 시뮬레이터</title></head>"
    "<body><h1>전자식 차일드락 시뮬레이터</h1>"
    "<p>값은 숫자, 열거 식별자 또는 absent입니다. 비워 두면 기본(정상) 입력입니다. 운전자 명령은 cmd_side(0 좌, 1 우, 2 전체), cmd_action(0 lock, 1 unlock), cmd_source(0~3) 세 값을 모두 지정해야 합니다.</p>"
    "<form id=\"f\"></form><button id=\"go\">한 주기 실행</button> <button id=\"rs\">초기화</button>"
    "<h2>상태</h2><pre id=\"st\"></pre><h2>이력</h2><pre id=\"hi\"></pre>"
    "<script>"
    "const names=['speed','gear','source_timestamp','crash','approach_left','approach_right','fire','overtemp',"
    "'adult','isofix_left','isofix_right','ignition','sensor_fault','cmd_side','cmd_action','cmd_source'];"
    "const f=document.getElementById('f');"
    "for(const n of names){f.insertAdjacentHTML('beforeend','<label>'+n+' <input name=\"'+n+'\"></label><br>');}"
    "async function show(){document.getElementById('hi').textContent=await (await fetch('/history')).text();}"
    "document.getElementById('go').onclick=async()=>{const p=[];for(const n of names){const v=f.elements[n].value;"
    "if(v!=='')p.push(n+'='+v);}const r=await fetch('/step',{method:'POST',body:p.join('&')});"
    "document.getElementById('st').textContent=r.status+' '+await r.text();show();};"
    "document.getElementById('rs').onclick=async()=>{await fetch('/reset',{method:'POST'});"
    "document.getElementById('st').textContent='';show();};"
    "</script></body></html>";

Status_t WebPage_Get(const char **html, size_t *length)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((html != NULL) && (length != NULL))
    {
        *html = WEB_PAGE;
        *length = strlen(WEB_PAGE);
        result = STATUS_OK;
    }
    return result;
}
