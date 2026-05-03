// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/kleidicv.h"
#include "kleidicv/utils.h"

#if KLEIDICV_EXPERIMENTAL_FEATURE_CANNY

namespace kleidicv::neon {

kleidicv_error_t canny_u8(const uint8_t *src, size_t src_stride, uint8_t *dst,
                          size_t dst_stride, size_t width, size_t height,
                          double low_threshold, double high_threshold);

}  // namespace kleidicv::neon

namespace kleidicv::sc {

static kleidicv_error_t canny_u8(const uint8_t *src, size_t src_stride,
                                 uint8_t *dst, size_t dst_stride, size_t width,
                                 size_t height, double, double) {
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  if (width == 0 || height == 0) {
    return KLEIDICV_OK;
  }

  return KLEIDICV_ERROR_NOT_IMPLEMENTED;
}

}  // namespace kleidicv::sc

static decltype(&kleidicv::sc::canny_u8) kleidicv_canny_u8_resolver() {
#if KLEIDICV_ENABLE_NEON
  return &kleidicv::neon::canny_u8;
#else
  return &kleidicv::sc::canny_u8;
#endif
}

extern "C" {

decltype(&kleidicv::sc::canny_u8) kleidicv_canny_u8 =
    kleidicv_canny_u8_resolver();

}  // extern "C"

#endif  // KLEIDICV_EXPERIMENTAL_FEATURE_CANNY
