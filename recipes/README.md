# 组合配方

已有离线动态连通性配方：生存区间 + 时间树事件分发 + 可回滚并查集。实现可直接 include `algorithm_assets/recipes/offline_connectivity.hpp`，契约位于 `../contracts/offline_connectivity.json`，说明见 [独立文档](../docs/recipes/connectivity.offline.md)。全部组合配方可从 [文档目录](../docs/README.md#组合配方19) 进入。

时间树分发与普通区间聚合线段树语义不同，因此配方没有强行依赖后者。后续依真实复用需求拆分组件。

2026-10-15 新增三个题目驱动的可调用配方：`disjoint_interval_pair.hpp`（扫描 + 长度压缩 + 最低费用）、`bounded_step_subsequence.hpp`（压缩 + 范围最大值 DP）、`tree_eccentricities.hpp`（无权 CSR + 直径端点 + 三次 BFS），均位于 `include/algorithm_assets/recipes/`。接口、组合条件与整体成本见 [配方文档](../docs/README.md#组合配方19)，三道正式 AC 和快照见 [COMBINATION_VALIDATION.md](../judge/COMBINATION_VALIDATION.md)。

## 新增组合：逆序对

`include/algorithm_assets/recipes/inversions_fenwick.hpp` 实现有序键压缩、从右向左扫描和 Fenwick 频数查询的整体组合；输入保持不变，重复值不算逆序对，输出精确 uint64。目录默认优先单算法归并计数；请求 `strategy=fenwick_recipe` 才选此配方。不是尚未实现的关系图设想，完整条件与成本见 `docs/README.md`。
