# 第三方依赖

`eigen3/` 包含官方 Eigen 3.4.0 的 `Eigen/` 头文件及原始 `COPYING.*`，未修改上游文件。来源、压缩包 SHA-256 与逐文件哈希见 [固定清单](../provenance/dependencies/eigen-3.4.0.json)。

Eigen 主体采用 MPL-2.0，个别文件使用其他条款，以文件声明与 COPYING 为准。本工程通过 `algorithm_assets::eigen` 目标传递 `EIGEN_MPL2_ONLY`，仅启用本次使用的 Eigen 核心与内建稀疏求解模块；不启用 unsupported、FFTW、SuiteSparse、METIS 等可选外部依赖。`EIGEN_DONT_PARALLELIZE` 固定本目标的 Eigen 内部并行配置，所有相关翻译单元必须一致。

使用示例：

```cmake
target_link_libraries(your_target PRIVATE algorithm_assets::eigen)
```

该目标同时传递本库和 Eigen 的 include 路径；原 `algorithm_assets::algorithm_assets` 不增加 Eigen 依赖。安装包包含所需 Eigen 头文件及上游许可文本。若应用同时使用其他 Eigen 版本，调用者必须统一头文件搜索顺序与配置，不能在同一程序内混用不兼容版本。

这份集成不改变 Eigen 的许可证，也不把第三方头文件重新授权为本工程自有代码。分发时应保留并履行相应源码与声明义务；本库整体许可仍待用户决定。
