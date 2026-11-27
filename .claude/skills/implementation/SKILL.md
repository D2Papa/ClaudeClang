---
name: implementation
description: A-SPICE SWE.3 기준으로 상세 설계를 C/C++ 코드로 구현한다. 코드 작성, 함수 구현, 소스 구현 요청에 사용한다. 함수 단일 반환 및 반환값 규칙을 적용한다.
---

# implementation (A-SPICE SWE.3 — 유닛 구현)

## 입력
- 인자(`$ARGUMENTS`): 구현할 모듈·함수 ID 또는 상세 설계서 경로
- 리뷰 완료된 상세 설계가 없으면 `detailed-design` 스킬을 먼저 수행하도록 안내한다.

## 필수 코딩 규칙

### 1. 모든 함수는 반환값을 가진다
- `void` 함수는 작성하지 않는다.
- 의미 있는 결과 값이 없으면, **정상 실행 여부를 판단할 수 있는 값**을 반환한다.
  프로젝트의 상태 코드 타입(예: `Status_t`, `STATUS_OK`, `STATUS_ERR_PARAM`, …)을 설계서 기준으로 한 곳에 정의해서 재사용한다.
- 호출자는 반환값을 확인해야 한다. 반환값을 무시해야 하는 경우 이유를 주석으로 남긴다.

### 2. return은 함수 마지막에 한 번만
- 함수 중간의 `return`, 조기 반환(early return)을 쓰지 않는다.
- 결과 변수를 선언·초기화하고 마지막 줄에서 한 번 반환한다.
- 오류 처리는 중첩 `if`, 상태 변수 검사 등으로 구성한다. 깊은 중첩이 생기면 함수를 분리한다.

```c
/**
 * @brief 센서 값을 필터링하여 출력 버퍼에 저장한다.
 * @param[in]  raw  원시 센서 값 (0..4095)
 * @param[out] out  필터링된 값 (NULL 불가)
 * @return STATUS_OK 성공 / STATUS_ERR_PARAM 입력 오류
 * @trace DD-F-012, SRS-F-007
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
Status_t Sensor_Filter(uint16_t raw, uint16_t *out)
{
    Status_t result = STATUS_ERR_PARAM;

    if ((out != NULL) && (raw <= SENSOR_RAW_MAX))
    {
        *out = Filter_Apply(raw);
        result = STATUS_OK;
    }

    return result;
}
```

### 3. 구현은 TDD로 한다
- `test-driven-development` 스킬로 테스트를 먼저 쓰고 실패를 확인한 뒤 구현한다.
- TDD 테스트는 `test/tdd/`에 둔다. 단위 테스트(`test/unit/`)와 분리하고, 결과와 커버리지도 별도로 측정한다.

### 4. 소스 품질 기준 (예외 없음)
- 순환 복잡도 10 이하, 함수 80라인 이하 (Lizard). `src/` 파일을 수정하면 Hook이 자동 검사한다. 초과하면 함수를 분리한다.
- Cppcheck 경고 0건. `cppcheck-suppress` 등 예외 처리는 쓰지 않는다.

### 5. 주석은 Doxygen + Human in the Loop 정보
모든 파일과 함수 주석에 아래 블록을 포함한다. (형식은 CLAUDE.md 2.4)
```c
 * @par AI 작성 정보
 *   - 모델: <모델 이름 (모델 ID)>
 *   - 작성일: <YYYY-MM-DD>
 *   - 요청자: <요청한 사용자>
 * @par 사용자 리뷰
 *   - 완료 여부: 미완료
 *   - 리뷰일: -
 *   - 리뷰어: -
```
- 초기 상태는 항상 "미완료"다. AI는 "완료"로 바꾸지 않는다 (사용자가 리뷰했다고 알려준 경우만 갱신).
- 리뷰 완료된 코드를 수정하면 "미완료"로 되돌리고 모델·작성일을 갱신한다.

## 절차

1. 상세 설계서의 모듈·함수 정의를 읽는다. 설계에 없는 함수나 동작을 임의로 추가하지 않는다.
   구현 중 설계 결함을 발견하면 코드에 반영하기 전에 사용자에게 알리고 설계서를 먼저 수정한다.
2. 모듈의 공개 헤더(`.h`)에는 인터페이스만 선언하고, 내부 함수·데이터는 `static`으로 숨긴다.
3. 코드를 작성한다. 모든 함수에 Doxygen 주석(`@brief`, `@param`, `@return`, `@trace`)을 단다.
4. 자체 점검(아래)을 수행한다.
5. `claudeclangbuild` 컨테이너에서 `scripts/ci/build.sh`로 빌드하고 컴파일러 경고를 0으로 만든다 (`-Wall -Wextra -Werror` 적용됨).
   `scripts/ci/static.sh`(Cppcheck), `scripts/ci/metrics.sh`(Lizard), `python3 scripts/ci/check_comments.py`로 품질 기준을 확인한다.
6. 추적성 매트릭스에 `DD ID → 소스 파일:함수`를 기록한다.
7. `review` 스킬로 리뷰를 요청하고, 이어서 `unit-test` 스킬로 검증한다.

## 자체 점검

- 함수마다 `return` 문이 정확히 하나이고, 함수 마지막에 있다. (확인: 함수 본문의 `return` 개수 검색)
- `void` 반환 함수가 없다.
- 포인터 인자 검증, 범위 검증 등 설계서의 오류 처리가 모두 구현되었다.
- 매직 넘버 대신 이름 있는 상수를 쓴다.
- 다른 모듈의 내부 데이터·함수에 접근하지 않고 공개 인터페이스만 호출한다.
- 설계서에 없는 동작이 없다 (데드 코드·미사용 함수 없음).
- 반환값 규칙 때문에 `main`, 인터럽트 핸들러처럼 프레임워크가 시그니처를 강제하는 함수가 있으면
  예외 목록을 설계서에 명시하고 사용자에게 알린다.

## 완료 보고
- 작성·수정한 파일과 함수 목록, 빌드 결과(경고 수), 자체 점검 결과, 설계와 달라진 점
