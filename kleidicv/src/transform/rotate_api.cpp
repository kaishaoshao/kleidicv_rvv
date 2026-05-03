// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/dispatch.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv/transform/rotate.h"

KLEIDICV_MULTIVERSION_C_API_WITH_SME(
    kleidicv_rotate, KLEIDICV_NEON_IMPL_IF(&kleidicv::neon::rotate), nullptr,
    &kleidicv::sme::rotate, nullptr);
