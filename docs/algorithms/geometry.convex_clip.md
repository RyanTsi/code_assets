# 凸包半平面裁剪

资产 ID：`geometry.convex_clip`

类型：算法

问题：`geometry.convex_clip`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_convex_clip.json) | [公共头文件](../../include/algorithm_assets/geometry/convex_clip.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/convex_clip.hpp>
```

`clip_left(hull,line)` 保留有向直线的闭左半平面，边界接触保留；空输入或空交集返回空边界。跨界边插入交点，输出至多 h+1 个点。整数输入也产生浮点构造，结果可能退化或包含共线点，不自动封装为严格凸包。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **hull**：Validated ConvexPolygon2<T>, n=h
- **line**：Directed nondegenerate Line and explicit tolerance

### 输出

- **boundary**：Owned vector<Point> in boundary order; empty intersection empty; boundary vertices retained; adjacent exact duplicates removed; collinear vertices may remain
- **numeric_conversion**：Integer input deliberately converts to floating for constructed intersections; result is not a typed strict convex hull

## 前置条件与约束

- **coordinates**：{"one_of": ["float64_abs_le_1e100", "int64_abs_le_1e9"]}
- **result_guarantee**：approximate
- **input_structure**：strict_convex_boundary
- **half_plane**：closed_left

- **最大 n**：1152921504606846975
- **coordinates**：Float: finite |x|,|y|<=1e100; integer: |x|,|y|<=1e9. Out-of-domain inputs throw invalid_argument.
- **数值限制**：Integer predicate/squared-distance operations exact within bound; floating operations approximate, with long double intermediates; no exact floating kernel. Underflow and near-degenerate topology are not certified. No fast-math.
- **所有权**：Inputs borrowed for call and not mutated unless explicitly stated; returned coordinates owned; no implicit I/O or global mutable state.
- **并发条件**：Concurrent calls may share read-only input; caller must not write input during a call.
- **异常与失败行为**：Invalid domain throws invalid_argument; constructed coordinates outside +/-1e100 or nonfinite scalar result throw overflow_error; detectable distinct-point squared-distance underflow throws underflow_error; allocation/size may throw.
- **历史状态**：No persistence, branching, snapshots, rollback, undo or redo. Ordinary value copying is not persistent storage.

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发写入**：不支持
- **并发读取**：支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **scan**：O(n)
- **空间**：Output <=16(n+1) bytes capacity, O(1) workspace; no input copy

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

沿凸包边逐段保留闭半平面内顶点，并在且仅在跨界时插入交点；凸性保证输出顺序仍为交区域边界，退化结果保留。

## 关联资产

- 依赖使用：[直线与线段几何](geometry.linear.md)（`geometry.linear`）。
- 依赖使用：[凸包 · 整数](geometry.convex_hull.int.md)（`geometry.convex_hull.int`）。条件：coordinates=int64_abs_le_1e9
- 依赖使用：[凸包 · 浮点](geometry.convex_hull.float.md)（`geometry.convex_hull.float`）。条件：coordinates=float64_abs_le_1e100

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in this document; floating correctness conditional on numeric predicates; not formally verified
- **测试**：[tests/geometry_test.cpp](../../tests/geometry_test.cpp)
- **示例**：[examples/geometry2d.cpp](../../examples/geometry2d.cpp)
- **基准**：not_measured
- **补充**：Regression and degeneracy cases; random integer/float comparisons; closest/diameter quadratic oracles; rectangle all-edge oracle; circle membership/tangency properties

- **实现来源**：Reorganized and rewritten from user-provided template; original preserved byte-for-byte
- **参考**：provenance/geometry2d-template.cpp.txt; docs/project/GEOMETRY2D.md lists read-only Wiki and primary-source references
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
