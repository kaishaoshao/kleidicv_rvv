#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-riscv-lib"
TOOLCHAIN_FILE="${SCRIPT_DIR}/cmake/toolchains/riscv64-linux-clang.cmake"

: "${RISCV_LLVM_ROOT:=/opt/homebrew/opt/llvm}"
: "${RISCV_TARGET_TRIPLE:=riscv64-unknown-linux-gnu}"
: "${RISCV_ARCH:=rv64gc}"
: "${RISCV_ABI:=lp64d}"
: "${RISCV_SYSROOT:=}"
: "${RISCV_GCC_TOOLCHAIN:=}"

if [[ -z "${RISCV_SYSROOT}" ]]; then
  echo "RISCV_SYSROOT is not set."
  echo "Set it to a RISC-V Linux sysroot before building."
  echo "Example:"
  echo "  RISCV_SYSROOT=/path/to/riscv64-sysroot bash build.sh"
  exit 1
fi

mkdir -p "${BUILD_DIR}"

cmake -S "${SCRIPT_DIR}/kleidicv" -B "${BUILD_DIR}" \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
  -DRISCV_LLVM_ROOT="${RISCV_LLVM_ROOT}" \
  -DRISCV_TARGET_TRIPLE="${RISCV_TARGET_TRIPLE}" \
  -DRISCV_ARCH="${RISCV_ARCH}" \
  -DRISCV_ABI="${RISCV_ABI}" \
  -DRISCV_SYSROOT="${RISCV_SYSROOT}" \
  -DRISCV_GCC_TOOLCHAIN="${RISCV_GCC_TOOLCHAIN}" \
  -DKLEIDICV_ENABLE_NEON=OFF \
  -DKLEIDICV_ENABLE_SVE2=OFF \
  -DKLEIDICV_ENABLE_SME=OFF \
  -DKLEIDICV_ENABLE_SME2=OFF \
  -DCMAKE_C_FLAGS="--target=riscv32-unknown-linux-gnu " \
  -DCMAKE_CXX_FLAGS="--target=riscv32-unknown-linux-gnu" .

  -DKLEIDICV_LIMIT_SVE2_TO_SELECTED_ALGORITHMS=OFF \
  -DKLEIDICV_LIMIT_SME2_TO_SELECTED_ALGORITHMS=OFF

cmake --build "${BUILD_DIR}" --target kleidicv