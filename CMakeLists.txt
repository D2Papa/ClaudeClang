cmake_minimum_required(VERSION 3.25)
project(ClaudeClang LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_COVERAGE "LLVM 소스 기반 커버리지(MC/DC 포함) 계측" OFF)

add_compile_options(-Wall -Wextra -Werror)

if(ENABLE_COVERAGE)
  # 라인/분기/함수 + MC/DC 계측 (clang 18 이상)
  add_compile_options(-O0 -g -fprofile-instr-generate -fcoverage-mapping -fcoverage-mcdc)
  add_link_options(-fprofile-instr-generate)
endif()

enable_testing()
find_package(GTest REQUIRED)

# 제품 코드: src/<모듈>/ 아래의 .c 파일을 모두 하나의 라이브러리로 묶는다.
file(GLOB_RECURSE PRODUCT_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.c)
add_library(product STATIC ${PRODUCT_SOURCES})
target_include_directories(product PUBLIC ${CMAKE_SOURCE_DIR}/src)

# 테스트 스위트: tdd / unit / integration / system 은 서로 분리된 실행 파일과 결과로 만든다.
function(add_test_suite SUITE)
  file(GLOB_RECURSE SUITE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/test/${SUITE}/*.cpp)
  if(SUITE_SOURCES)
    add_executable(${SUITE}_tests ${SUITE_SOURCES})
    target_link_libraries(${SUITE}_tests PRIVATE product GTest::gtest GTest::gmock GTest::gtest_main)
    add_test(NAME ${SUITE} COMMAND ${SUITE}_tests)
  endif()
endfunction()

foreach(SUITE tdd unit integration system)
  add_test_suite(${SUITE})
endforeach()
