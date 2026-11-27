#!/usr/bin/env bash
# Doxygen 주석 검증: 경고가 있으면 실패한다.
set -euo pipefail
cd "$(dirname "$0")/../.."
doxygen docs/Doxyfile
