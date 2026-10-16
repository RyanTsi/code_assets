# 资源评测

`geometry_benchmark.cpp` / `geometry_results.csv` 已记录最近点对在三种分布、三种规模下的调用时间、分配次数和工作区容量，参数与局限见 `../docs/project/GEOMETRY_VALIDATION.md`。

可通过 `ALGORITHM_ASSETS_BUILD_BENCHMARKS=ON` 构建，或按记录的 g++ 命令复现。容量不是实际峰值内存；当前没有其它算法的性能基准，也不据此宣称跨平台或所有分布最优。后续再测进程峰值、其它算法和平台。
