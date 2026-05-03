# SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Allow configuration to succeed without a full RISC-V Linux linker/sysroot
# setup. This is sufficient for building static libraries and object files.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(RISCV_LLVM_ROOT "/opt/homebrew/opt/llvm" CACHE PATH
    "Path to the LLVM installation used for RISC-V cross compilation")
set(RISCV_TARGET_TRIPLE "riscv64-unknown-linux-gnu" CACHE STRING
    "Target triple for RISC-V cross compilation")
set(RISCV_ARCH "rv64gc" CACHE STRING
    "RISC-V ISA string passed to clang via -march")
set(RISCV_ABI "lp64d" CACHE STRING
    "RISC-V ABI passed to clang via -mabi")
set(RISCV_SYSROOT "" CACHE PATH
    "Optional sysroot for RISC-V Linux linking")
set(RISCV_GCC_TOOLCHAIN "" CACHE PATH
    "Optional GCC toolchain root for RISC-V Linux headers and runtime")

set(CMAKE_C_COMPILER "${RISCV_LLVM_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${RISCV_LLVM_ROOT}/bin/clang++")
set(CMAKE_AR "${RISCV_LLVM_ROOT}/bin/llvm-ar")
set(CMAKE_RANLIB "${RISCV_LLVM_ROOT}/bin/llvm-ranlib")

set(_riscv_common_flags
    "--target=${RISCV_TARGET_TRIPLE}"
    "-march=${RISCV_ARCH}"
    "-mabi=${RISCV_ABI}")

if(RISCV_SYSROOT)
  list(APPEND _riscv_common_flags "--sysroot=${RISCV_SYSROOT}")
endif()

if(RISCV_GCC_TOOLCHAIN)
  list(APPEND _riscv_common_flags "--gcc-toolchain=${RISCV_GCC_TOOLCHAIN}")
endif()

string(JOIN " " _riscv_flags ${_riscv_common_flags})

set(CMAKE_C_FLAGS_INIT "${_riscv_flags}")
set(CMAKE_CXX_FLAGS_INIT "${_riscv_flags}")
set(CMAKE_ASM_FLAGS_INIT "${_riscv_flags}")

if(RISCV_SYSROOT)
  set(CMAKE_EXE_LINKER_FLAGS_INIT "${_riscv_flags} -fuse-ld=lld")
  set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_riscv_flags} -fuse-ld=lld")
endif()
