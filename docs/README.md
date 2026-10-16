# 资产文档目录

每个已登记算法、数据表示和组合配方各有独立正文，契约的 `documentation` 直接指向该文件；整数与浮点等契约不同的变体分别成文。文档含接口说明、输入输出、限制、状态、成本、依据与关联，不要求先阅读某批次的长篇汇总。

公共约定见 [设计约定](project/DESIGN.md)、[几何数值约定](project/GEOMETRY2D.md) 和 [目录检索](project/CATALOG.md#目录检索)。阶段计划、验证报告、来源台账和应用案例单独保留，不充当某个算法的正文。

## 算法（54）

| 稳定 ID | 独立文档 | 问题 |
| --- | --- | --- |
| `algebra.modulus64` | [全宽模运算](algorithms/algebra.modulus64.md) | `modular_arithmetic` |
| `algebra.prime_sieve` | [奇数位压缩素数筛](algorithms/algebra.prime_sieve.md) | `primality_table` |
| `algebra.xor_basis64` | [XorBasis64](algorithms/algebra.xor_basis64.md) | `gf2_span` |
| `connectivity.dsu` | [DisjointSet](algorithms/connectivity.dsu.md) | `connectivity` |
| `connectivity.rollback_dsu` | [RollbackDisjointSet](algorithms/connectivity.rollback_dsu.md) | `connectivity` |
| `geometry.circle` | [圆交点、切线与构造](algorithms/geometry.circle.md) | `geometry.circle_operations` |
| `geometry.closest_pair.float` | [最近点对 · 浮点](algorithms/geometry.closest_pair.float.md) | `geometry.closest_pair` |
| `geometry.closest_pair.int` | [最近点对 · 整数](algorithms/geometry.closest_pair.int.md) | `geometry.closest_pair` |
| `geometry.convex_clip` | [凸包半平面裁剪](algorithms/geometry.convex_clip.md) | `geometry.convex_clip` |
| `geometry.convex_hull.float` | [凸包 · 浮点](algorithms/geometry.convex_hull.float.md) | `geometry.convex_hull` |
| `geometry.convex_hull.int` | [凸包 · 整数](algorithms/geometry.convex_hull.int.md) | `geometry.convex_hull` |
| `geometry.convex_location.float` | [凸多边形二分定位 · 浮点](algorithms/geometry.convex_location.float.md) | `geometry.point_in_polygon` |
| `geometry.convex_location.int` | [凸多边形二分定位 · 整数](algorithms/geometry.convex_location.int.md) | `geometry.point_in_polygon` |
| `geometry.convexity.float` | [凸性检查 · 浮点](algorithms/geometry.convexity.float.md) | `geometry.convexity` |
| `geometry.convexity.int` | [凸性检查 · 整数](algorithms/geometry.convexity.int.md) | `geometry.convexity` |
| `geometry.diameter.float` | [凸包直径 · 浮点](algorithms/geometry.diameter.float.md) | `geometry.farthest_pair` |
| `geometry.diameter.int` | [凸包直径 · 整数](algorithms/geometry.diameter.int.md) | `geometry.farthest_pair` |
| `geometry.disk_polygon` | [圆盘与多边形区域相交](algorithms/geometry.disk_polygon.md) | `geometry.disk_polygon_intersection` |
| `geometry.integer_segments` | [整数线段相交](algorithms/geometry.integer_segments.md) | `geometry.segment_predicates` |
| `geometry.linear` | [直线与线段几何](algorithms/geometry.linear.md) | `geometry.linear_operations` |
| `geometry.minimum_enclosing_circle` | [近似最小覆盖圆](algorithms/geometry.minimum_enclosing_circle.md) | `geometry.minimum_enclosing_circle` |
| `geometry.minimum_rectangle` | [凸包最小外接矩形](algorithms/geometry.minimum_rectangle.md) | `geometry.minimum_rectangle` |
| `geometry.minkowski_difference.int` | [整数闵可夫斯基差](algorithms/geometry.minkowski_difference.int.md) | `geometry.minkowski_difference` |
| `geometry.minkowski_sum.int` | [整数闵可夫斯基和](algorithms/geometry.minkowski_sum.int.md) | `geometry.minkowski_sum` |
| `geometry.polar_sort` | [极角排序](algorithms/geometry.polar_sort.md) | `geometry.polar_sort` |
| `geometry.polygon_location.float` | [多边形定位 · 浮点](algorithms/geometry.polygon_location.float.md) | `geometry.point_in_polygon` |
| `geometry.polygon_location.int` | [多边形定位 · 整数](algorithms/geometry.polygon_location.int.md) | `geometry.point_in_polygon` |
| `geometry.polygon_measures` | [多边形面积与周长](algorithms/geometry.polygon_measures.md) | `geometry.polygon_measures` |
| `geometry.predicates.float` | [浮点几何谓词](algorithms/geometry.predicates.float.md) | `geometry.predicates` |
| `geometry.predicates.int` | [整数几何谓词](algorithms/geometry.predicates.int.md) | `geometry.predicates` |
| `graph.bellman_ford` | [带负环状态的 Bellman-Ford](algorithms/graph.bellman_ford.md) | `shortest_paths` |
| `graph.dijkstra` | [Dijkstra with indexed heap](algorithms/graph.dijkstra.md) | `shortest_paths` |
| `graph.dinic` | [Dinic 最大流](algorithms/graph.dinic.md) | `maximum_flow` |
| `graph.hopcroft_karp` | [二分图匹配与最小点覆盖](algorithms/graph.hopcroft_karp.md) | `bipartite_matching` |
| `graph.kruskal` | [KruskalMinimumSpanningForest](algorithms/graph.kruskal.md) | `minimum_spanning_forest` |
| `graph.lca` | [森林倍增 LCA](algorithms/graph.lca.md) | `lowest_common_ancestor` |
| `graph.strong_components` | [强连通分量](algorithms/graph.strong_components.md) | `strong_components` |
| `graph.topological_sort` | [TopologicalOrder](algorithms/graph.topological_sort.md) | `topological_order` |
| `numeric.compensated_sum` | [Neumaier compensated sum](algorithms/numeric.compensated_sum.md) | `sum` |
| `numeric.dense_lu` | [部分主元 LU](algorithms/numeric.dense_lu.md) | `linear_system` |
| `numeric.radix2_fft` | [Radix-2 FFT](algorithms/numeric.radix2_fft.md) | `discrete_fourier_transform` |
| `optimization.knapsack_01` | [0/1 背包](algorithms/optimization.knapsack_01.md) | `knapsack` |
| `optimization.knapsack_bounded` | [单调队列多重背包](algorithms/optimization.knapsack_bounded.md) | `knapsack` |
| `optimization.knapsack_unbounded` | [完全背包](algorithms/optimization.knapsack_unbounded.md) | `knapsack` |
| `range.fenwick_sum` | [FenwickSum](algorithms/range.fenwick_sum.md) | `range_sum` |
| `range.segment_tree` | [SegmentTree](algorithms/range.segment_tree.md) | `range_fold` |
| `range.sparse_table` | [幂等稀疏表](algorithms/range.sparse_table.md) | `range_fold` |
| `sequence.common_subsequence` | [最长公共子序列长度](algorithms/sequence.common_subsequence.md) | `common_subsequence` |
| `sequence.increasing_subsequence` | [最长递增子序列](algorithms/sequence.increasing_subsequence.md) | `increasing_subsequence` |
| `sequence.inversions` | [InversionCounter](algorithms/sequence.inversions.md) | `inversion_count` |
| `sequence.merge_intervals` | [区间并集](algorithms/sequence.merge_intervals.md) | `interval_union` |
| `sequence.minimum_interval_cover` | [最少区间覆盖](algorithms/sequence.minimum_interval_cover.md) | `interval_cover` |
| `strings.kmp` | [KmpPattern](algorithms/strings.kmp.md) | `substring_search` |
| `strings.manacher` | [Manacher 回文半径](algorithms/strings.manacher.md) | `palindrome_search` |

## 数据表示（5）

| 稳定 ID | 独立文档 | 问题 |
| --- | --- | --- |
| `geometry.primitives` | [二维几何基础](representations/geometry.primitives.md) | `geometry.representation` |
| `graph.csr` | [CsrGraph](representations/graph.csr.md) | `graph_storage` |
| `graph.unweighted_csr` | [无权 CSR](representations/graph.unweighted_csr.md) | `unweighted_graph_storage` |
| `numeric.wide_int64_sum` | [WideInt64Sum](representations/numeric.wide_int64_sum.md) | `wide_integer_accumulation` |
| `sequence.coordinate_compression` | [有序坐标压缩](representations/sequence.coordinate_compression.md) | `coordinate_compression` |

## 组合配方（15）

| 稳定 ID | 独立文档 | 问题 |
| --- | --- | --- |
| `algebra.chinese_remainder` | [广义中国剩余定理配方](recipes/algebra.chinese_remainder.md) | `chinese_remainder` |
| `connectivity.offline` | [Offline dynamic connectivity](recipes/connectivity.offline.md) | `connectivity` |
| `geometry.point_set_diameter.float` | [点集直径配方 · 浮点](recipes/geometry.point_set_diameter.float.md) | `geometry.farthest_pair` |
| `geometry.point_set_diameter.int` | [点集直径配方 · 整数](recipes/geometry.point_set_diameter.int.md) | `geometry.farthest_pair` |
| `geometry.point_set_rectangle` | [点集外接矩形配方](recipes/geometry.point_set_rectangle.md) | `geometry.minimum_rectangle` |
| `geometry.translated_convex_overlap` | [凸形平移碰撞配方](recipes/geometry.translated_convex_overlap.md) | `geometry.translated_overlap` |
| `graph.condensation` | [SCC 缩点配方](recipes/graph.condensation.md) | `condensation` |
| `graph.tree_eccentricities` | [树上全部最远距离](recipes/graph.tree_eccentricities.md) | `tree_eccentricities` |
| `logic.two_sat` | [2-SAT 配方](recipes/logic.two_sat.md) | `boolean_constraints` |
| `numeric.real_convolution` | [FFT 实卷积配方](recipes/numeric.real_convolution.md) | `convolution` |
| `optimization.difference_constraints` | [差分约束配方](recipes/optimization.difference_constraints.md) | `difference_constraints` |
| `probability.absorbing_markov` | [吸收马尔科夫链配方](recipes/probability.absorbing_markov.md) | `absorbing_markov` |
| `sequence.bounded_step_subsequence` | [有限步差最长子序列](recipes/sequence.bounded_step_subsequence.md) | `bounded_step_subsequence` |
| `sequence.disjoint_interval_pair` | [非重叠区间对最低费用](recipes/sequence.disjoint_interval_pair.md) | `disjoint_interval_pair` |
| `sequence.inversions_fenwick` | [InversionsFenwick](recipes/sequence.inversions_fenwick.md) | `inversion_count` |

## 工程与验证

设计约定、建设计划、来源台账、验证报告和应用案例统一从 [工程资料](project/README.md) 进入。
