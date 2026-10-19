# 资源评测

`mesh3d_benchmark.cpp` / `mesh3d_results.csv` 记录规则参数网格的校验、BVH构建/1000次查询、Eigen装配和split+提交。硬件、Release参数与局限见 [三维集成说明](../docs/project/MESH3D_INTEGRATION.md#实际验证与基准)。容量为SurfaceData数组capacity，不是进程峰值，编辑包含全量复制/重建成本。

`geometry_benchmark.cpp` / `geometry_results.csv` 已记录最近点对在三种分布、三种规模下的调用时间、分配次数和工作区容量，参数与局限见 `../docs/project/GEOMETRY_VALIDATION.md`。

可通过 `ALGORITHM_ASSETS_BUILD_BENCHMARKS=ON` 构建，或按记录的 g++ 命令复现。容量不是实际峰值内存；这些局部基准不代表其它算法、跨平台或所有分布最优。后续再测进程峰值、其它算法和平台。
