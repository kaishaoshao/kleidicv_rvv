#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

SCRIPT_PATH="$(realpath "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)")"
REPO_ROOT="$(realpath "${SCRIPT_PATH}/..")"

MODE="${1:-all}"

cd "${REPO_ROOT}"

run_lint() {
  CHECK_ONLY=ON VERBOSE=ON CLANG_FORMAT_BIN_PATH="${CLANG_FORMAT_BIN_PATH:-clang-format}" \
    scripts/format.sh
  scripts/cpplint.sh

  # shellcheck disable=SC2046
  shellcheck $(find scripts -name '*.sh' | tr '\n' ' ')

  reuse lint

  if git --no-pager grep -n '[^ -~]'; then
    echo "ERROR: non-ASCII characters or tab found."
    exit 1
  fi
}

run_clang() {
  rm -rf build/github/clang
  mkdir -p build/github/test-results

  export CMAKE_CXX_COMPILER_LAUNCHER=ccache

  cmake -S . -B build/github/clang -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_COMPILE_WARNING_AS_ERROR=ON \
    -DCMAKE_CXX_FLAGS="--target=aarch64-linux-gnu" \
    -DCMAKE_C_FLAGS="--target=aarch64-linux-gnu" \
    -DCMAKE_EXE_LINKER_FLAGS="--rtlib=compiler-rt -static -fuse-ld=lld" \
    -DKLEIDICV_ENABLE_SME=OFF \
    -DKLEIDICV_ENABLE_SME2=OFF \
    -DKLEIDICV_LIMIT_SVE2_TO_SELECTED_ALGORITHMS=OFF

  ninja -C build/github/clang

  local testresult=0
  local long_vector_tests="GRAY2.*:RGB*:Yuv*:Rgb*:Resize*"

  qemu-aarch64 build/github/clang/test/framework/kleidicv-framework-test \
    --gtest_output=xml:build/github/test-results/clang-framework/kleidicv-framework-test.xml || testresult=1
  qemu-aarch64 -cpu cortex-a35 build/github/clang/test/unit_neon/kleidicv-neon-unit-test \
    --gtest_output=xml:build/github/test-results/clang-unit-neon/kleidicv-neon-unit-test.xml || testresult=1
  qemu-aarch64 -cpu cortex-a35 build/github/clang/test/api/kleidicv-api-test \
    --gtest_output=xml:build/github/test-results/clang-neon/kleidicv-api-test.xml || testresult=1
  qemu-aarch64 -cpu max,sve128=on,sme=off build/github/clang/test/api/kleidicv-api-test \
    --gtest_output=xml:build/github/test-results/clang-sve128/kleidicv-api-test.xml \
    --vector-length=16 || testresult=1
  qemu-aarch64 -cpu max,sve2048=on,sve-default-vector-length=256,sme=off \
    build/github/clang/test/api/kleidicv-api-test \
    --gtest_filter="${long_vector_tests}" \
    --gtest_output=xml:build/github/test-results/clang-sve2048/kleidicv-api-test.xml \
    --vector-length=256 || testresult=1

  scripts/prefix_testsuite_names.py \
    build/github/test-results/clang-unit-neon/kleidicv-neon-unit-test.xml \
    "github-clang-unit-neon."
  scripts/prefix_testsuite_names.py \
    build/github/test-results/clang-neon/kleidicv-api-test.xml \
    "github-clang-neon."
  scripts/prefix_testsuite_names.py \
    build/github/test-results/clang-sve128/kleidicv-api-test.xml \
    "github-clang-sve128."
  scripts/prefix_testsuite_names.py \
    build/github/test-results/clang-sve2048/kleidicv-api-test.xml \
    "github-clang-sve2048."

  return "${testresult}"
}

run_gcc() {
  rm -rf build/github/gcc
  mkdir -p build/github/test-results

  export CMAKE_CXX_COMPILER_LAUNCHER=ccache

  CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ \
    cmake -S . -B build/github/gcc -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_COMPILE_WARNING_AS_ERROR=ON \
      -DCMAKE_EXE_LINKER_FLAGS="-static"

  ninja -C build/github/gcc

  local testresult=0
  qemu-aarch64 -cpu cortex-a35 build/github/gcc/test/api/kleidicv-api-test \
    --gtest_output=xml:build/github/test-results/gcc-neon/kleidicv-api-test.xml || testresult=1

  scripts/prefix_testsuite_names.py \
    build/github/test-results/gcc-neon/kleidicv-api-test.xml \
    "github-gcc-neon."

  return "${testresult}"
}

case "${MODE}" in
  lint)
    run_lint
    ;;
  clang)
    run_clang
    ;;
  gcc)
    run_gcc
    ;;
  all)
    run_lint
    run_clang
    run_gcc
    ;;
  *)
    echo "Usage: $0 [lint|clang|gcc|all]" >&2
    exit 2
    ;;
esac
