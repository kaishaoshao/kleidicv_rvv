// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/kleidicv.h"
#include "kleidicv/utils.h"

namespace kleidicv::neon {

template <typename T>
kleidicv_error_t count_nonzeros(const T *src, size_t src_stride, size_t width,
                                size_t height, size_t *count);

}  // namespace kleidicv::neon

namespace kleidicv::sc {

static kleidicv_error_t count_nonzeros_u8(const uint8_t *src, size_t src_stride,
                                          size_t width, size_t height,
                                          size_t *count) {
  CHECK_POINTERS(count);
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  size_t total = 0;
  for (size_t y = 0; y < height; ++y) {
    const auto *src_row = reinterpret_cast<const uint8_t *>(
        reinterpret_cast<const char *>(src) + y * src_stride);
    for (size_t x = 0; x < width; ++x) {
      total += src_row[x] != 0;
    }
  }

  *count = total;
  return KLEIDICV_OK;
}

}  // namespace kleidicv::sc

static decltype(&kleidicv::sc::count_nonzeros_u8)
kleidicv_count_nonzeros_u8_resolver() {
#if KLEIDICV_ENABLE_NEON
  return &kleidicv::neon::count_nonzeros<uint8_t>;
#else
  return &kleidicv::sc::count_nonzeros_u8;
#endif
}

extern "C" {

decltype(&kleidicv::sc::count_nonzeros_u8) kleidicv_count_nonzeros_u8 =
    kleidicv_count_nonzeros_u8_resolver();

}  // extern "C"
