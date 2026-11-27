---
name: unit-test
description: A-SPICE SWE.4 기준으로 유닛 테스트를 작성하고 커버리지 목표(MC/DC 포함)를 달성한다. 단위 테스트, 유닛 테스트, 커버리지 100%, 경계값·동치 클래스·PICT 테스트 설계, MC/DC 요청에 사용한다.
---

# unit-test (A-SPICE SWE.4 — 유닛 검증)

## 입력
- 인자(`$ARGUMENTS`): 테스트 대상 모듈·함수와 커버리지 목표 (예: `src/sensor.c --target stmt,branch` 또는 `--target mcdc`)
- 커버리지 목표가 정해져 있지 않으면 사용자에게 확인한다 (구문 / 분기 / MC/DC 중 선택). 목표는 테스트 계획에 기록한다.

## 핵심 규칙

1. **정해진 커버리지를 100% 달성할 때까지 테스트 케이스를 반복해서 추가한다.** 측정 → 미달 지점 분석 → 케이스 추가 → 재측정을 목표 달성까지 반복한다.
2. **의미 없는 테스트로 수치를 채우지 않는다.** 모든 케이스는 설계서의 기대 동작을 검증해야 한다.
   assertion이 없거나, 출력값을 확인하지 않고 코드만 통과시키는 케이스는 금지한다.
3. **Stub/Mock은 필요한 경우에만** 쓰고 의존하지 않는다. 하드웨어·외부 시스템·비결정적 요소(시간, 난수)처럼
   격리가 꼭 필요한 곳에만 쓰고, 실제 모듈을 그대로 쓸 수 있으면 실제 모듈을 쓴다.
4. 도달 불가능한 코드(죽은 분기)를 발견하면 커버리지를 맞추려 하지 말고, 코드·설계 결함으로 보고한다.
   방어 코드 때문에 정당하게 도달 불가한 경우에는 근거와 함께 사용자 승인을 받아 예외로 기록한다.
5. **모든 테스트 함수에 Doxygen 주석**으로 테스트 목적, 입력값, 절차, 예상 출력값을 쓴다.

## 테스트 함수 주석 형식

```c
/**
 * @brief   [목적] raw 값이 상한 경계(4095)일 때 필터가 정상 값을 출력하는지 확인한다.
 * @par 입력값
 *   - raw = 4095 (SENSOR_RAW_MAX, 경계값)
 *   - out = 유효한 버퍼 주소
 * @par 절차
 *   -# out 버퍼를 0으로 초기화한다.
 *   -# Sensor_Filter(4095, &out)를 호출한다.
 * @par 예상 출력값
 *   - 반환값 == STATUS_OK
 *   - out == 4095에 대한 필터 결과 (예: 4095)
 * @trace UT-012 -> DD-F-012, SRS-F-007
 * @technique 경계값 분석
 */
static void test_Sensor_Filter_RawAtUpperBound_ReturnsOk(void) { ... }
```

(테스트 프레임워크가 강제하는 `void` 시그니처 등은 테스트 코드에 한해 허용한다.)
테스트 이름은 `test_<대상>_<조건>_<기대결과>` 형식으로 쓴다.

## 테스트 설계 기법

| 기법 | 쓰는 곳 |
|------|---------|
| 동치 클래스 분할 | 입력 도메인을 유효/무효 클래스로 나누고 클래스마다 대표값 1개 이상 |
| 경계값 분석 | 각 경계의 직전·경계·직후 값 (min-1, min, min+1, max-1, max, max+1) |
| PICT 조합 | 독립 파라미터가 3개 이상일 때 pairwise 조합으로 케이스 축소 |
| 오류 추정 | NULL 포인터, 오버플로, 0, 음수 등 경험적 결함 유형 |

### PICT 사용
1. 파라미터와 값(동치 클래스·경계값 대표값)을 `.pict` 모델 파일로 작성한다. 제약(`IF ... THEN ...`)도 반영한다.
2. `pict model.pict /o:2 > cases.txt`로 조합을 생성한다 (기본 pairwise, 필요 시 `/o:3`).
3. 생성된 조합을 테스트 케이스로 옮기고, 예상 출력값은 **설계서에서** 도출한다 (코드를 실행해 본 결과를 그대로 기대값으로 쓰지 않는다).
4. PICT가 없으면 사용자에게 설치를 안내한다 (https://github.com/microsoft/pict).

## 절차

1. 대상 함수의 상세 설계(DD-F)를 읽고 기대 동작과 입력 도메인을 정리한다.
2. 위 기법으로 테스트 케이스를 설계하고 케이스별 추적(`UT → DD-F → SRS`)을 정한다.
3. 테스트 코드를 작성한다 (Doxygen 주석 포함).
4. 커버리지를 측정한다 (아래 도구).
5. 미달 지점을 분석해 의미 있는 케이스를 추가하고 4번으로 돌아간다. 목표를 달성하면 종료한다.
6. 실패한 테스트는 원인이 구현인지 설계서인지 판단해 보고한다. 테스트를 약화해서 통과시키지 않는다.
7. 추적성 매트릭스에 `DD ID/SRS ID → UT ID`를 기록한다.
8. `review` 스킬로 테스트 명세·코드를 리뷰한다.

## 커버리지 측정 (LLVM)

LLVM 도구는 이 스킬의 스크립트로 사용한다: `.claude/skills/unit-test/scripts/llvm_coverage.py`

```powershell
# 구문/분기 커버리지 측정
python .claude/skills/unit-test/scripts/llvm_coverage.py --src src/sensor.c --test test/test_sensor.c --out build/cov

# MC/DC 커버리지 측정 (목표가 MC/DC로 수립된 경우)
python .claude/skills/unit-test/scripts/llvm_coverage.py --src src/sensor.c --test test/test_sensor.c --out build/cov --mcdc
```

- 필요 도구: `clang`(MC/DC는 18 이상), `llvm-profdata`, `llvm-cov` (PATH에 있어야 함)
- MC/DC 모드: `-fcoverage-mcdc`로 계측하고 `llvm-cov show --show-mcdc`로 조건별 독립성(independence pair) 충족 여부를 보여준다.
  리포트에서 **미충족 조건**을 찾고, 그 조건이 결과를 단독으로 뒤집는 입력 쌍을 설계해 케이스를 추가한다.
- MC/DC 목표가 있으면 모든 결정(decision)의 모든 조건이 독립성을 보여야 완료다.
- 조건이 6개를 넘는 결정은 LLVM MC/DC 제한에 걸릴 수 있다. 이 경우 조건식 분해를 설계 수정 사항으로 보고한다.
- 도구가 없으면 임의 대체 방법을 쓰지 말고 설치 필요 사실을 사용자에게 알린다.

## 완료 보고
- 대상·목표, 케이스 수(기법별), 커버리지 결과(목표 대비), 예외 처리한 항목과 근거, 실패 테스트와 원인, 추적성 누락
