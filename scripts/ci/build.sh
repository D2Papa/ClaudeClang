#!/usr/bin/env bash
# 빌드: CMake + Ninja, LLVM 커버리지(MC/DC) 계측 포함. claudeclangbuild 컨테이너 안에서 실행한다.
set -euo pipefail
cd "$(dirname "$0")/../.."

cmake -S . -B build -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DENABLE_COVERAGE=ON
cmake --build build
