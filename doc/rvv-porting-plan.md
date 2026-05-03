<!--
SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>

SPDX-License-Identifier: Apache-2.0
-->

# RVV 移植规划

本文档列出了建议从 KleidiCV 移植到 RISC-V RVV 的算子，包括：

- 基线实现位置
- 现有 Arm 优化实现位置
- 建议新增的 RVV 实现落点

## 移植原则

对于当前代码库，最适合优先做 RVV 移植的算子，通常具备下面这种结构：

- 公共 API 入口位于 `kleidicv/src/**/*_api.cpp`
- 通用实现或标量基线实现位于 `kleidicv/src/**/*_sc.h`
- Arm SIMD 实现位于 `*_neon.cpp`、`*_sve2.cpp`、`*_sme.cpp` 或 `*_sme2.cpp`

对于这类算子，推荐采用下面的 RVV 移植模式：

1. 保留现有通用实现作为正确性基线。
2. 新增 RVV 实现文件，例如 `*_rvv.cpp`。
3. 更新 dispatch 与构建接线，使 RVV 实现能够像现有 Arm 后端一样被选择。

## 第一批推荐移植算子

这一批算子已经具备比较清晰的“通用实现 + 优化实现”结构，是最适合优先补齐 RVV 后端的目标。

| 优先级 | 算子族 | 通用实现位置 | 现有 Arm 优化实现位置 | 建议新增 RVV 文件 |
|---|---|---|---|---|
| P0 | 饱和逐元素算术：`add`、`sub`、`absdiff`、`multiply` | `kleidicv/src/arithmetics/add_sc.h`<br>`kleidicv/src/arithmetics/sub_sc.h`<br>`kleidicv/src/arithmetics/absdiff_sc.h`<br>`kleidicv/src/arithmetics/multiply_sc.h` | `kleidicv/src/arithmetics/add_neon.cpp`<br>`kleidicv/src/arithmetics/add_sve2.cpp`<br>`kleidicv/src/arithmetics/add_sme.cpp`<br>`kleidicv/src/arithmetics/add_sme2.cpp`<br>以及 `sub`、`absdiff`、`multiply` 对应同名文件 | `kleidicv/src/arithmetics/add_rvv.cpp`<br>`kleidicv/src/arithmetics/sub_rvv.cpp`<br>`kleidicv/src/arithmetics/absdiff_rvv.cpp`<br>`kleidicv/src/arithmetics/multiply_rvv.cpp` |
| P0 | 逐元素逻辑与比较：`threshold`、`compare`、`in_range`、`bitwise_and`、`add_abs_with_threshold` | `kleidicv/src/arithmetics/threshold_sc.h`<br>`kleidicv/src/arithmetics/compare_sc.h`<br>`kleidicv/src/arithmetics/in_range_sc.h`<br>`kleidicv/src/arithmetics/bitwise_and_sc.h`<br>`kleidicv/src/arithmetics/add_abs_with_threshold_sc.h` | `kleidicv/src/arithmetics/` 下对应的 `*_neon.cpp`、`*_sve2.cpp`、`*_sme.cpp`、`*_sme2.cpp` | `kleidicv/src/arithmetics/threshold_rvv.cpp`<br>`kleidicv/src/arithmetics/compare_rvv.cpp`<br>`kleidicv/src/arithmetics/in_range_rvv.cpp`<br>`kleidicv/src/arithmetics/bitwise_and_rvv.cpp`<br>`kleidicv/src/arithmetics/add_abs_with_threshold_rvv.cpp` |
| P0 | 标量变换：`scale`、`exp` | `kleidicv/src/arithmetics/scale_sc.h`<br>`kleidicv/src/arithmetics/exp_sc.h` | `kleidicv/src/arithmetics/scale_neon.cpp`<br>`kleidicv/src/arithmetics/scale_sve2.cpp`<br>`kleidicv/src/arithmetics/scale_sme.cpp`<br>`kleidicv/src/arithmetics/scale_sme2.cpp`<br>`kleidicv/src/arithmetics/exp_neon.cpp`<br>`kleidicv/src/arithmetics/exp_sve2.cpp`<br>`kleidicv/src/arithmetics/exp_sme.cpp`<br>`kleidicv/src/arithmetics/exp_sme2.cpp` | `kleidicv/src/arithmetics/scale_rvv.cpp`<br>`kleidicv/src/arithmetics/exp_rvv.cpp` |
| P0 | 统计类：`sum`、`min_max` | `kleidicv/src/analysis/sum_sc.h`<br>`kleidicv/src/analysis/min_max_sc.h` | `kleidicv/src/analysis/sum_neon.cpp`<br>`kleidicv/src/analysis/sum_sve2.cpp`<br>`kleidicv/src/analysis/sum_sme.cpp`<br>`kleidicv/src/analysis/sum_sme2.cpp`<br>`kleidicv/src/analysis/min_max_neon.cpp`<br>`kleidicv/src/analysis/min_max_sve2.cpp`<br>`kleidicv/src/analysis/min_max_sme.cpp`<br>`kleidicv/src/analysis/min_max_sme2.cpp` | `kleidicv/src/analysis/sum_rvv.cpp`<br>`kleidicv/src/analysis/min_max_rvv.cpp` |
| P1 | 光流核心：`standalone_lucas_kanade_alg` | `kleidicv/src/analysis/standalone_lucas_kanade_alg_sc.h` | `kleidicv/src/analysis/standalone_lucas_kanade_alg_neon.cpp`<br>`kleidicv/src/analysis/standalone_lucas_kanade_alg_sve2.cpp`<br>`kleidicv/src/analysis/standalone_lucas_kanade_alg_sme.cpp`<br>`kleidicv/src/analysis/standalone_lucas_kanade_alg_sme2.cpp` | `kleidicv/src/analysis/standalone_lucas_kanade_alg_rvv.cpp` |
| P0 | 基础颜色与类型转换：`gray_to_rgb`、`rgb_to_rgb`、`float_conv` | `kleidicv/src/conversions/gray_to_rgb_sc.h`<br>`kleidicv/src/conversions/rgb_to_rgb_sc.h`<br>`kleidicv/src/conversions/float_conv_sc.h` | `kleidicv/src/conversions/` 下对应的 `*_neon.cpp`、`*_sve2.cpp`、`*_sme.cpp`、`*_sme2.cpp` | `kleidicv/src/conversions/gray_to_rgb_rvv.cpp`<br>`kleidicv/src/conversions/rgb_to_rgb_rvv.cpp`<br>`kleidicv/src/conversions/float_conv_rvv.cpp` |
| P0 | RGB 转 YUV：`rgb_to_yuv422`、`rgb_to_yuv444`、`rgb_to_yuv420p`、`rgb_to_yuv420sp` | `kleidicv/src/conversions/rgb_to_yuv422_sc.h`<br>`kleidicv/src/conversions/rgb_to_yuv444_sc.h`<br>`kleidicv/src/conversions/rgb_to_yuv420_sc.h` | `kleidicv/src/conversions/rgb_to_yuv422_neon.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv422_sve2.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv422_sme.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv444_neon.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv444_sve2.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv444_sme.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv444_sme2.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420p_neon.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420p_sve2.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420p_sme.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420p_sme2.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420sp_neon.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420sp_sve2.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420sp_sme.cpp` | `kleidicv/src/conversions/rgb_to_yuv422_rvv.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv444_rvv.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420p_rvv.cpp`<br>`kleidicv/src/conversions/rgb_to_yuv420sp_rvv.cpp` |
| P0 | YUV 转 RGB：`yuv422_to_rgb`、`yuv444_to_rgb`、`yuv420p_to_rgb`、`yuv420sp_to_rgb` | `kleidicv/src/conversions/yuv422_to_rgb_sc.h`<br>`kleidicv/src/conversions/yuv444_to_rgb_sc.h`<br>`kleidicv/src/conversions/yuv420_to_rgb_sc.h`<br>`kleidicv/src/conversions/yuv420p_to_rgb_sc.h`<br>`kleidicv/src/conversions/yuv420sp_to_rgb_sc.h` | `kleidicv/src/conversions/yuv422_to_rgb_neon.cpp`<br>`kleidicv/src/conversions/yuv422_to_rgb_sve2.cpp`<br>`kleidicv/src/conversions/yuv422_to_rgb_sme.cpp`<br>`kleidicv/src/conversions/yuv444_to_rgb_neon.cpp`<br>`kleidicv/src/conversions/yuv444_to_rgb_sve2.cpp`<br>`kleidicv/src/conversions/yuv444_to_rgb_sme.cpp`<br>`kleidicv/src/conversions/yuv420p_to_rgb_neon.cpp`<br>`kleidicv/src/conversions/yuv420p_to_rgb_sve2.cpp`<br>`kleidicv/src/conversions/yuv420p_to_rgb_sme.cpp`<br>`kleidicv/src/conversions/yuv420p_to_rgb_sme2.cpp`<br>`kleidicv/src/conversions/yuv420sp_to_rgb_neon.cpp`<br>`kleidicv/src/conversions/yuv420sp_to_rgb_sve2.cpp`<br>`kleidicv/src/conversions/yuv420sp_to_rgb_sme.cpp` | `kleidicv/src/conversions/yuv422_to_rgb_rvv.cpp`<br>`kleidicv/src/conversions/yuv444_to_rgb_rvv.cpp`<br>`kleidicv/src/conversions/yuv420p_to_rgb_rvv.cpp`<br>`kleidicv/src/conversions/yuv420sp_to_rgb_rvv.cpp` |
| P0 | 卷积基础算子：`sobel`、`scharr`、`separable_filter_2d` | `kleidicv/src/filters/sobel_sc.h`<br>`kleidicv/src/filters/scharr_sc.h`<br>`kleidicv/src/filters/separable_filter_2d_sc.h` | `kleidicv/src/filters/sobel_neon.cpp`<br>`kleidicv/src/filters/sobel_sve2.cpp`<br>`kleidicv/src/filters/sobel_sme.cpp`<br>`kleidicv/src/filters/scharr_neon.cpp`<br>`kleidicv/src/filters/scharr_sve2.cpp`<br>`kleidicv/src/filters/scharr_sme.cpp`<br>`kleidicv/src/filters/scharr_sme2.cpp`<br>`kleidicv/src/filters/separable_filter_2d_neon.cpp`<br>`kleidicv/src/filters/separable_filter_2d_sve2.cpp`<br>`kleidicv/src/filters/separable_filter_2d_sme.cpp` | `kleidicv/src/filters/sobel_rvv.cpp`<br>`kleidicv/src/filters/scharr_rvv.cpp`<br>`kleidicv/src/filters/separable_filter_2d_rvv.cpp` |
| P0 | 模糊类：`gaussian_blur_fixed`、`blur_and_downsample` | `kleidicv/src/filters/gaussian_blur_fixed_sc.h`<br>`kleidicv/src/filters/blur_and_downsample_sc.h` | `kleidicv/src/filters/gaussian_blur_fixed_neon.cpp`<br>`kleidicv/src/filters/gaussian_blur_fixed_sve2.cpp`<br>`kleidicv/src/filters/gaussian_blur_fixed_sme.cpp`<br>`kleidicv/src/filters/blur_and_downsample_neon.cpp`<br>`kleidicv/src/filters/blur_and_downsample_sve2.cpp`<br>`kleidicv/src/filters/blur_and_downsample_sme.cpp` | `kleidicv/src/filters/gaussian_blur_fixed_rvv.cpp`<br>`kleidicv/src/filters/blur_and_downsample_rvv.cpp` |
| P1 | `median_blur` 排序网络路径 | `kleidicv/src/filters/median_blur_sorting_network_sc.h` | `kleidicv/src/filters/median_blur_sorting_network_neon.cpp`<br>`kleidicv/src/filters/median_blur_sorting_network_sve2.cpp`<br>`kleidicv/src/filters/median_blur_sorting_network_sme.cpp` | `kleidicv/src/filters/median_blur_sorting_network_rvv.cpp` |
| P0 | 形态学：`erode`、`dilate` | `kleidicv/src/morphology/morphology_sc.h` | `kleidicv/src/morphology/morphology_neon.cpp`<br>`kleidicv/src/morphology/morphology_sve2.cpp`<br>`kleidicv/src/morphology/morphology_sme.cpp`<br>`kleidicv/src/morphology/morphology_sme2.cpp` | `kleidicv/src/morphology/morphology_rvv.cpp` |
| P0 | 缩放：`resize_linear`、`resize_to_quarter` | `kleidicv/src/resize/resize_linear_sc.h`<br>`kleidicv/src/resize/resize_linear_generic_sc.h`<br>`kleidicv/src/resize/resize_to_quarter_sc.h` | `kleidicv/src/resize/resize_linear_neon.cpp`<br>`kleidicv/src/resize/resize_linear_sve2.cpp`<br>`kleidicv/src/resize/resize_linear_sme.cpp`<br>`kleidicv/src/resize/resize_linear_sme2.cpp`<br>`kleidicv/src/resize/resize_to_quarter_neon.cpp`<br>`kleidicv/src/resize/resize_to_quarter_sve2.cpp`<br>`kleidicv/src/resize/resize_to_quarter_sme.cpp`<br>`kleidicv/src/resize/resize_to_quarter_sme2.cpp` | `kleidicv/src/resize/resize_linear_rvv.cpp`<br>`kleidicv/src/resize/resize_to_quarter_rvv.cpp` |

## 第二批推荐移植算子

这些算子同样值得做 RVV，但相比第一批，它们的通用实现还不够清晰，或者基线实现还不完整。

| 优先级 | 算子族 | 当前情况 | 相关源码位置 | 建议 |
|---|---|---|---|---|
| P1 | `remap` | 已有 Arm 优化实现，但没有明显独立的 `remap_sc.h` 基线文件 | `kleidicv/src/transform/remap_api.cpp`<br>`kleidicv/src/transform/remap_f32_neon.cpp`<br>`kleidicv/src/transform/remap_f32_sve2.cpp`<br>`kleidicv/src/transform/remap_s16_neon.cpp`<br>`kleidicv/src/transform/remap_s16_sve2.cpp`<br>`kleidicv/src/transform/remap_s16point5_neon.cpp`<br>`kleidicv/src/transform/remap_s16point5_sve2.cpp` | 建议先抽取或补齐通用基线，再新增 `remap_*_rvv.cpp` |
| P1 | `warp_perspective` | 目前只看到 Arm 优化实现 | `kleidicv/src/transform/warp_perspective_api.cpp`<br>`kleidicv/src/transform/warp_perspective_neon.cpp`<br>`kleidicv/src/transform/warp_perspective_sve2.cpp` | 先补通用路径，再做 RVV |
| P1 | `transpose`、`rotate` | 有共享 transform 头文件，但没有明显独立的标量后端文件 | `kleidicv/src/transform/transpose_api.cpp`<br>`kleidicv/src/transform/transpose_neon.cpp`<br>`kleidicv/src/transform/transpose_sme.cpp`<br>`kleidicv/src/transform/rotate_api.cpp`<br>`kleidicv/src/transform/rotate_neon.cpp`<br>`kleidicv/src/transform/rotate_sme.cpp`<br>`kleidicv/src/transform/transform_common.h` | 建议先补清晰的标量或通用 tile 路径，再做 RVV |
| P2 | `merge`、`split` | 当前只有 Neon 实现 | `kleidicv/src/conversions/merge_api.cpp`<br>`kleidicv/src/conversions/merge_neon.cpp`<br>`kleidicv/src/conversions/split_api.cpp`<br>`kleidicv/src/conversions/split_neon.cpp` | 先补 `merge_sc.h` 和 `split_sc.h`，再做 RVV |
| P2 | `count_nonzeros`、`min_max_loc`、`canny` | 当前只看到 Neon 实现 | `kleidicv/src/analysis/count_nonzeros_neon.cpp`<br>`kleidicv/src/analysis/min_max_loc_neon.cpp`<br>`kleidicv/src/analysis/canny_neon.cpp` | 建议先补通用基线，再做 RVV |
| P2 | `gaussian_blur_arbitrary` | 任意核大小 Gaussian 路径目前看起来只有 Neon 实现 | `kleidicv/src/filters/gaussian_blur_arbitrary_neon.cpp` | 建议在 fixed-kernel Gaussian 的 RVV 版本稳定后再做 |

## 建议实施顺序

下面的顺序在覆盖面、复用现有基础设施以及实现风险之间更均衡：

1. `arithmetics`
2. `analysis/sum` 和 `analysis/min_max`
3. `conversions/gray_to_rgb`、`rgb_to_rgb`、`float_conv`
4. `conversions/rgb_to_yuv*` 和 `yuv*_to_rgb`
5. `filters/sobel`、`scharr`、`separable_filter_2d`、`gaussian_blur_fixed`
6. `resize/resize_linear` 和 `resize_to_quarter`
7. `morphology`
8. `analysis/standalone_lucas_kanade_alg`
9. `transform/remap`
10. `transform/warp_perspective`、`transpose`、`rotate`

## 常见接线点

新增 RVV 后端时，通常不只是增加一个算子实现文件，还需要一起处理下面这些接线点。

| 区域 | 位置 | 预期 RVV 相关工作 |
|---|---|---|
| 构建集成 | `kleidicv/CMakeLists.txt` | 增加 RVV 源文件 glob 或显式文件列表，创建 RVV object library，设置 RVV 编译选项 |
| dispatch 与特性检测 | `kleidicv/include/kleidicv/dispatch.h` | 增加 RVV 能力检测与 dispatch 宏，形式可参考当前 Arm 后端 |
| API 解析与分发 | `kleidicv/src/**/*_api.cpp` | 扩展 resolver，使其在适用场景下能够选择 RVV 实现 |
| 公共头文件 | `kleidicv/include/kleidicv/**/*.h` | 仅在引入新的后端符号或命名空间契约时更新声明 |
| 测试 | `test/api/` 和 `test/unit_neon/` | 复用现有 API 测试校验正确性；若行为与向量长度相关，可补充 RVV 专项覆盖 |

## 说明

- 在这个仓库里，“通用实现”通常是 `*_sc.h` 中的模板路径或标量路径，而不一定是独立的 `.cpp` 文件。
- 第一批算子最适合做成“普通实现 + RVV 实现”的双路径设计。
- 对于第二批算子，先抽出清晰的通用后端，再做 RVV，会更容易验证和维护。
