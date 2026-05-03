#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../build-riscv"
TOOLCHAIN_FILE="${SCRIPT_DIR}/../cmake/toolchains/riscv64-linux-clang.cmake"

: "${RISCV_TARGET_TRIPLE:=riscv32-unknown-linux-gnu}"
: "${RISCV_ARCH:=rv32gc}"
: "${RISCV_ABI:=ilp32d}"
: "${RISCV_SYSROOT:=}"


mkdir -p "${BUILD_DIR}"

cmake -S "../kleidicv_rvv" -B "${BUILD_DIR}" \
  -G Ninja \
  -DCMAKE_C_COMPILER=zcc  \
  -DCMAKE_CXX_COMPILER=z++ \
  -DCMAKE_C_FLAGS="-DGTEST_HAS_STEADY_CLOCK_=0" \
  -DCMAKE_CXX_FLAGS="-DGTEST_HAS_STEADY_CLOCK_=0" \
  -DRISCV_ARCH="${RISCV_ARCH}" \
  -DRISCV_ABI="${RISCV_ABI}" \
  -DKLEIDICV_ENABLE_NEON=OFF \
  -DKLEIDICV_ENABLE_SVE2=OFF \
  -DKLEIDICV_ENABLE_SME=OFF \
  -DKLEIDICV_ENABLE_SME2=OFF \
  -DKLEIDICV_LIMIT_SVE2_TO_SELECTED_ALGORITHMS=OFF \
  -DKLEIDICV_LIMIT_SME2_TO_SELECTED_ALGORITHMS=OFF

ninja -C "${BUILD_DIR}"