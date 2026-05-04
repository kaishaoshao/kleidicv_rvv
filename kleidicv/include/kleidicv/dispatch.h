// SPDX-FileCopyrightText: 2023 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_DISPATCH_H
#define KLEIDICV_DISPATCH_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

#include "kleidicv/config.h"
#include "kleidicv/ctypes.h"
#include "kleidicv/utils.h"

template <typename FuncPtr>
struct NotImplementedBackend;

template <typename... Args>
struct NotImplementedBackend<kleidicv_error_t (*)(Args...)> {
  static kleidicv_error_t fn(Args...) { return KLEIDICV_ERROR_NOT_IMPLEMENTED; }
};

#if KLEIDICV_ENABLE_NEON
#define KLEIDICV_NEON_IMPL_IF(func) func
#define KLEIDICV_SCALAR_OR_NEON(scalar_impl, neon_impl) neon_impl
#else
#define KLEIDICV_NEON_IMPL_IF(func) (&NotImplementedBackend<decltype(func)>::fn)
#define KLEIDICV_SCALAR_OR_NEON(scalar_impl, neon_impl) scalar_impl
#endif

namespace kleidicv::sc {

template <typename T>
using WideType = std::conditional_t<
    std::is_floating_point_v<T>, long double,
    std::conditional_t<std::is_signed_v<T>, __int128_t, __uint128_t>>;

template <typename T>
inline const T *row_ptr(const T *base, size_t stride, size_t y) {
  return reinterpret_cast<const T *>(reinterpret_cast<const char *>(base) +
                                     y * stride);
}

template <typename T>
inline T *row_ptr(T *base, size_t stride, size_t y) {
  return reinterpret_cast<T *>(reinterpret_cast<char *>(base) + y * stride);
}

template <typename T, typename Wide>
inline T saturate_cast(Wide value) {
  if constexpr (std::is_floating_point_v<T>) {
    return static_cast<T>(value);
  } else {
    Wide low = static_cast<Wide>(std::numeric_limits<T>::lowest());
    Wide high = static_cast<Wide>(std::numeric_limits<T>::max());
    if (value < low) {
      return std::numeric_limits<T>::lowest();
    }
    if (value > high) {
      return std::numeric_limits<T>::max();
    }
    return static_cast<T>(value);
  }
}

template <typename SrcA, typename SrcB, typename Dst, typename Fn>
inline kleidicv_error_t binary_image_op(const SrcA *src_a, size_t src_a_stride,
                                        const SrcB *src_b, size_t src_b_stride,
                                        Dst *dst, size_t dst_stride,
                                        size_t width, size_t height, Fn fn) {
  CHECK_POINTER_AND_STRIDE(src_a, src_a_stride, height);
  CHECK_POINTER_AND_STRIDE(src_b, src_b_stride, height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  for (size_t y = 0; y < height; ++y) {
    const SrcA *src_a_row = row_ptr(src_a, src_a_stride, y);
    const SrcB *src_b_row = row_ptr(src_b, src_b_stride, y);
    Dst *dst_row = row_ptr(dst, dst_stride, y);
    for (size_t x = 0; x < width; ++x) {
      dst_row[x] = fn(src_a_row[x], src_b_row[x]);
    }
  }

  return KLEIDICV_OK;
}

template <typename Src, typename Dst, typename Fn>
inline kleidicv_error_t unary_image_op(const Src *src, size_t src_stride,
                                       Dst *dst, size_t dst_stride,
                                       size_t width, size_t height, Fn fn) {
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  for (size_t y = 0; y < height; ++y) {
    const Src *src_row = row_ptr(src, src_stride, y);
    Dst *dst_row = row_ptr(dst, dst_stride, y);
    for (size_t x = 0; x < width; ++x) {
      dst_row[x] = fn(src_row[x]);
    }
  }

  return KLEIDICV_OK;
}

template <typename T>
inline kleidicv_error_t saturating_add(const T *src_a, size_t src_a_stride,
                                       const T *src_b, size_t src_b_stride,
                                       T *dst, size_t dst_stride, size_t width,
                                       size_t height) {
  return binary_image_op(
      src_a, src_a_stride, src_b, src_b_stride, dst, dst_stride, width, height,
      [](T a, T b) {
        using Wide = WideType<T>;
        return saturate_cast<T>(static_cast<Wide>(a) + static_cast<Wide>(b));
      });
}

template <typename T>
inline kleidicv_error_t saturating_sub(const T *src_a, size_t src_a_stride,
                                       const T *src_b, size_t src_b_stride,
                                       T *dst, size_t dst_stride, size_t width,
                                       size_t height) {
  return binary_image_op(
      src_a, src_a_stride, src_b, src_b_stride, dst, dst_stride, width, height,
      [](T a, T b) {
        using Wide = WideType<T>;
        return saturate_cast<T>(static_cast<Wide>(a) - static_cast<Wide>(b));
      });
}

template <typename T>
inline kleidicv_error_t saturating_absdiff(const T *src_a, size_t src_a_stride,
                                           const T *src_b, size_t src_b_stride,
                                           T *dst, size_t dst_stride,
                                           size_t width, size_t height) {
  return binary_image_op(
      src_a, src_a_stride, src_b, src_b_stride, dst, dst_stride, width, height,
      [](T a, T b) {
        using Wide =
            std::conditional_t<std::is_signed_v<T>, __int128_t, __uint128_t>;
        Wide wa = static_cast<Wide>(a);
        Wide wb = static_cast<Wide>(b);
        Wide diff = wa >= wb ? (wa - wb) : (wb - wa);
        return saturate_cast<T>(diff);
      });
}

template <typename T>
inline kleidicv_error_t saturating_multiply(const T *src_a, size_t src_a_stride,
                                            const T *src_b, size_t src_b_stride,
                                            T *dst, size_t dst_stride,
                                            size_t width, size_t height,
                                            double scale) {
  return binary_image_op(src_a, src_a_stride, src_b, src_b_stride, dst,
                         dst_stride, width, height, [scale](T a, T b) {
                           long double product = static_cast<long double>(a) *
                                                 static_cast<long double>(b) *
                                                 scale;
                           return saturate_cast<T>(std::nearbyint(product));
                         });
}

template <typename T>
inline kleidicv_error_t bitwise_and(const T *src_a, size_t src_a_stride,
                                    const T *src_b, size_t src_b_stride, T *dst,
                                    size_t dst_stride, size_t width,
                                    size_t height) {
  return binary_image_op(src_a, src_a_stride, src_b, src_b_stride, dst,
                         dst_stride, width, height,
                         [](T a, T b) { return static_cast<T>(a & b); });
}

template <typename T>
inline kleidicv_error_t compare_equal(const T *src_a, size_t src_a_stride,
                                      const T *src_b, size_t src_b_stride,
                                      T *dst, size_t dst_stride, size_t width,
                                      size_t height) {
  return binary_image_op(src_a, src_a_stride, src_b, src_b_stride, dst,
                         dst_stride, width, height, [](T a, T b) {
                           return a == b ? std::numeric_limits<T>::max()
                                         : static_cast<T>(0);
                         });
}

template <typename T>
inline kleidicv_error_t compare_greater(const T *src_a, size_t src_a_stride,
                                        const T *src_b, size_t src_b_stride,
                                        T *dst, size_t dst_stride, size_t width,
                                        size_t height) {
  return binary_image_op(src_a, src_a_stride, src_b, src_b_stride, dst,
                         dst_stride, width, height, [](T a, T b) {
                           return a > b ? std::numeric_limits<T>::max()
                                        : static_cast<T>(0);
                         });
}

template <typename T>
inline kleidicv_error_t threshold_binary(const T *src, size_t src_stride,
                                         T *dst, size_t dst_stride,
                                         size_t width, size_t height,
                                         T threshold, T value) {
  return unary_image_op(src, src_stride, dst, dst_stride, width, height,
                        [threshold, value](T x) {
                          return x > threshold ? value : static_cast<T>(0);
                        });
}

template <typename T>
inline kleidicv_error_t in_range(const T *src, size_t src_stride, uint8_t *dst,
                                 size_t dst_stride, size_t width, size_t height,
                                 T lower_bound, T upper_bound) {
  return unary_image_op(src, src_stride, dst, dst_stride, width, height,
                        [lower_bound, upper_bound](T x) -> uint8_t {
                          return (x >= lower_bound && x <= upper_bound) ? 255
                                                                        : 0;
                        });
}

template <typename T, typename U>
inline kleidicv_error_t scale(const T *src, size_t src_stride, U *dst,
                              size_t dst_stride, size_t width, size_t height,
                              double scale_value, double shift) {
  return unary_image_op(
      src, src_stride, dst, dst_stride, width, height,
      [scale_value, shift](T x) -> U {
        if constexpr (std::is_same_v<T, uint8_t> &&
                      std::is_same_v<U, uint8_t>) {
          float result = static_cast<float>(x * scale_value + shift);
          if (result < std::numeric_limits<uint8_t>::lowest()) {
            return std::numeric_limits<uint8_t>::lowest();
          }
          if (result > std::numeric_limits<uint8_t>::max()) {
            return std::numeric_limits<uint8_t>::max();
          }
          return static_cast<uint8_t>(lrintf(result));
        } else if constexpr (std::is_same_v<T, float> &&
                             std::is_same_v<U, float>) {
          return x * static_cast<float>(scale_value) +
                 static_cast<float>(shift);
        } else {
          return static_cast<U>(x * scale_value + shift);
        }
      });
}

template <typename T, typename TInternal>
inline kleidicv_error_t sum(const T *src, size_t src_stride, size_t width,
                            size_t height, T *sum_value) {
  CHECK_POINTERS(sum_value);
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  TInternal total = 0;
  for (size_t y = 0; y < height; ++y) {
    const T *src_row = row_ptr(src, src_stride, y);
    for (size_t x = 0; x < width; ++x) {
      total += static_cast<TInternal>(src_row[x]);
    }
  }

  *sum_value = static_cast<T>(total);
  return KLEIDICV_OK;
}

template <typename T>
inline kleidicv_error_t min_max(const T *src, size_t src_stride, size_t width,
                                size_t height, T *min_value, T *max_value) {
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  if (width == 0 || height == 0) {
    return KLEIDICV_ERROR_RANGE;
  }

  T current_min = row_ptr(src, src_stride, 0)[0];
  T current_max = current_min;

  for (size_t y = 0; y < height; ++y) {
    const T *src_row = row_ptr(src, src_stride, y);
    for (size_t x = 0; x < width; ++x) {
      current_min = std::min(current_min, src_row[x]);
      current_max = std::max(current_max, src_row[x]);
    }
  }

  if (min_value != nullptr) {
    *min_value = current_min;
  }
  if (max_value != nullptr) {
    *max_value = current_max;
  }
  return KLEIDICV_OK;
}

template <typename T>
inline kleidicv_error_t min_max_loc(const T *src, size_t src_stride,
                                    size_t width, size_t height,
                                    size_t *min_offset, size_t *max_offset) {
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  if (width == 0 || height == 0) {
    return KLEIDICV_ERROR_RANGE;
  }

  T current_min = row_ptr(src, src_stride, 0)[0];
  T current_max = current_min;
  size_t current_min_offset = 0;
  size_t current_max_offset = 0;

  for (size_t y = 0; y < height; ++y) {
    const T *src_row = row_ptr(src, src_stride, y);
    for (size_t x = 0; x < width; ++x) {
      size_t offset = y * src_stride + x * sizeof(T);
      if (src_row[x] < current_min) {
        current_min = src_row[x];
        current_min_offset = offset;
      }
      if (src_row[x] > current_max) {
        current_max = src_row[x];
        current_max_offset = offset;
      }
    }
  }

  if (min_offset != nullptr) {
    *min_offset = current_min_offset;
  }
  if (max_offset != nullptr) {
    *max_offset = current_max_offset;
  }
  return KLEIDICV_OK;
}

}  // namespace kleidicv::sc

#if KLEIDICV_ENABLE_SME2 || KLEIDICV_ENABLE_SME || KLEIDICV_ENABLE_SVE2
#include <cstdlib>
#include <cstring>

namespace kleidicv {
#if KLEIDICV_ENABLE_SVE2
uint64_t svcntb_sve();
#endif
#if KLEIDICV_ENABLE_SME2 || KLEIDICV_ENABLE_SME
uint64_t svcntb_sme();
#endif
}  // namespace kleidicv

#ifdef __APPLE__

#include <sys/sysctl.h>

static inline bool query_sysctl(const char *attribute_name) {
  uint64_t attribute_value = 0;
  size_t max_attribute_size = sizeof(attribute_value);
  if (sysctlbyname(attribute_name, &attribute_value, &max_attribute_size, NULL,
                   0)) {
    return false;
  }
  return attribute_value;
}

static inline bool is_sve2_supported() {
  return query_sysctl("hw.optional.arm.FEAT_SVE2");
}

static inline bool is_sme_supported() {
  return query_sysctl("hw.optional.arm.FEAT_SME");
}

static inline bool is_sme2_supported() {
  return query_sysctl("hw.optional.arm.FEAT_SME2");
}

#else  // __APPLE__

#include <sys/auxv.h>

static inline bool is_sve2_supported() {
  return getauxval(AT_HWCAP2) & (1UL << 1);
}

static inline bool is_sme_supported() {
  return getauxval(AT_HWCAP2) & (1UL << 23);
}

static inline bool is_sme2_supported() {
  return getauxval(AT_HWCAP2) & (1UL << 37);
}

#endif  // __APPLE__

static inline bool is_prefer_sme_backend_env_var_set() {
  const char *v = std::getenv("KLEIDICV_PREFER_SME_BACKEND");
  if (v) {
    if (strcmp(v, "ON") == 0) {
      return true;
    }
  }
  return false;
}

#if KLEIDICV_ENABLE_SVE2
#define KLEIDICV_SVE2_RESOLVE(sve2_impl)                                     \
  if (!std::is_null_pointer_v<decltype(sve2_impl)> && is_sve2_supported()) { \
    return sve2_impl;                                                        \
  }
#define KLEIDICV_SVE2_RESOLVE_VECLEN(sve2_impl, sve_len)                     \
  if (!std::is_null_pointer_v<decltype(sve2_impl)> && is_sve2_supported() && \
      kleidicv::svcntb_sve() == (sve_len)) {                                 \
    return sve2_impl;                                                        \
  }
#else
#define KLEIDICV_SVE2_RESOLVE(x)
#define KLEIDICV_SVE2_RESOLVE_VECLEN(x, sve_len)
#endif  // KLEIDICV_ENABLE_SVE2

#if KLEIDICV_ENABLE_SME
#define KLEIDICV_SME_RESOLVE(sme_impl)                                     \
  if (!std::is_null_pointer_v<decltype(sme_impl)> && is_sme_supported()) { \
    return sme_impl;                                                       \
  }
#define KLEIDICV_SME_RESOLVE_VECLEN(sme_impl, sme_len)                     \
  if (!std::is_null_pointer_v<decltype(sme_impl)> && is_sme_supported() && \
      kleidicv::svcntb_sme() == (sme_len)) {                               \
    return sme_impl;                                                       \
  }
#else
#define KLEIDICV_SME_RESOLVE(x)
#define KLEIDICV_SME_RESOLVE_VECLEN(x, sme_len)
#endif  // KLEIDICV_ENABLE_SME

#if KLEIDICV_ENABLE_SME2
#define KLEIDICV_SME2_RESOLVE(sme2_impl)                                     \
  if (!std::is_null_pointer_v<decltype(sme2_impl)> && is_sme2_supported()) { \
    return sme2_impl;                                                        \
  }
#define KLEIDICV_SME2_RESOLVE_VECLEN(sme2_impl, sme_len)                     \
  if (!std::is_null_pointer_v<decltype(sme2_impl)> && is_sme2_supported() && \
      kleidicv::svcntb_sme() == (sme_len)) {                                 \
    return sme2_impl;                                                        \
  }
#else
#define KLEIDICV_SME2_RESOLVE(x)
#define KLEIDICV_SME2_RESOLVE_VECLEN(x, sme_len)
#endif  // KLEIDICV_ENABLE_SME2

#define KLEIDICV_MULTIVERSION_C_API_WITHOUT_SME(api_name, neon_impl, \
                                                sve2_impl)           \
  static decltype(neon_impl) api_name##_resolver() {                 \
    KLEIDICV_SVE2_RESOLVE(sve2_impl);                                \
    return neon_impl;                                                \
  }                                                                  \
  extern "C" {                                                       \
  decltype(neon_impl) api_name = api_name##_resolver();              \
  }

#define KLEIDICV_MULTIVERSION_C_API_WITH_SME(api_name, neon_impl, sve2_impl, \
                                             sme_impl, sme2_impl)            \
  static decltype(neon_impl) api_name##_resolver_default() {                 \
    if (is_prefer_sme_backend_env_var_set()) {                               \
      KLEIDICV_SME2_RESOLVE(sme2_impl);                                      \
      KLEIDICV_SME_RESOLVE(sme_impl);                                        \
    }                                                                        \
    KLEIDICV_SVE2_RESOLVE(sve2_impl);                                        \
    return neon_impl;                                                        \
  }                                                                          \
  static decltype(neon_impl) api_name##_resolver_for_sme() {                 \
    KLEIDICV_SME2_RESOLVE(sme2_impl);                                        \
    KLEIDICV_SME_RESOLVE(sme_impl);                                          \
    KLEIDICV_SVE2_RESOLVE(sve2_impl);                                        \
    return neon_impl;                                                        \
  }                                                                          \
  extern "C" {                                                               \
  decltype(neon_impl) api_name = api_name##_resolver_default();              \
  decltype(neon_impl) api_name##_sme = api_name##_resolver_for_sme();        \
  }

#define KLEIDICV_MULTIVERSION_C_API_VECLEN(                                \
    api_name, neon_impl, sve2_impl, sme_impl, sme2_impl, sve_len, sme_len) \
  static decltype(neon_impl) api_name##_resolver_default() {               \
    if (is_prefer_sme_backend_env_var_set()) {                             \
      KLEIDICV_SME2_RESOLVE_VECLEN(sme2_impl, sme_len);                    \
      KLEIDICV_SME_RESOLVE_VECLEN(sme_impl, sme_len);                      \
    }                                                                      \
    KLEIDICV_SVE2_RESOLVE_VECLEN(sve2_impl, sve_len);                      \
    return neon_impl;                                                      \
  }                                                                        \
  static decltype(neon_impl) api_name##_resolver_for_sme() {               \
    KLEIDICV_SME2_RESOLVE_VECLEN(sme2_impl, sme_len);                      \
    KLEIDICV_SME_RESOLVE_VECLEN(sme_impl, sme_len);                        \
    KLEIDICV_SVE2_RESOLVE_VECLEN(sve2_impl, sve_len);                      \
    return neon_impl;                                                      \
  }                                                                        \
  extern "C" {                                                             \
  decltype(neon_impl) api_name = api_name##_resolver_default();            \
  decltype(neon_impl) api_name##_sme = api_name##_resolver_for_sme();      \
  }

#else  // KLEIDICV_HAVE_SVE2 || KLEIDICV_HAVE_SME || KLEIDICV_HAVE_SME2

#define KLEIDICV_MULTIVERSION_C_API_WITHOUT_SME(api_name, neon_impl, \
                                                sve2_impl)           \
                                                                     \
  extern "C" {                                                       \
  decltype(neon_impl) api_name = neon_impl;                          \
  }

#define KLEIDICV_MULTIVERSION_C_API_WITH_SME(api_name, neon_impl, sve2_impl, \
                                             sme_impl, sme2_impl)            \
                                                                             \
  extern "C" {                                                               \
  decltype(neon_impl) api_name = neon_impl;                                  \
  decltype(neon_impl) api_name##_sme = neon_impl;                            \
  }

#define KLEIDICV_MULTIVERSION_C_API_VECLEN(                              \
    api_name, neon_impl, sve2_impl, sme_impl, sme2_impl, minlen, maxlen) \
  KLEIDICV_MULTIVERSION_C_API_WITH_SME(api_name, neon_impl, sve2_impl,   \
                                       sme_impl, sme2_impl)

#endif  // KLEIDICV_ENABLE_SME2 || KLEIDICV_ENABLE_SME ||  KLEIDICV_ENABLE_SVE2

#if KLEIDICV_ALWAYS_ENABLE_SME2
#define KLEIDICV_SME2_IMPL_IF(func) func
#else
#define KLEIDICV_SME2_IMPL_IF(func) nullptr
#endif  // KLEIDICV_ALWAYS_ENABLE_SME2

#if KLEIDICV_ALWAYS_ENABLE_SVE2
#define KLEIDICV_SVE2_IMPL_IF(func) func
#else
#define KLEIDICV_SVE2_IMPL_IF(func) nullptr
#endif  // KLEIDICV_ALWAYS_ENABLE_SVE2

#endif  // KLEIDICV_DISPATCH_H
