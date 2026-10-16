# 凸包 · 浮点

资产 ID：`geometry.convex_hull.float`

类型：算法

问题：`geometry.convex_hull`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_convex_hull_float.json) | [公共头文件](../../include/algorithm_assets/geometry/convex_hull.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/convex_hull.hpp>
```

`convex_hull<T>` / `ConvexPolygon2<T>::from_points` 接收任意点集，返回去重、去中间共线点的 CCW 边界，从字典序最小点开始且不重复首点；空/全重复/共线得到0/1/2点。span 复制输入，vector 按值可转移存储。`from_boundary` 严格检查循环顺序，拒绝自交、凹形和冗余边界，不只检查局部拐向。浮点坐标有限且限 ±10¹⁰⁰，谓词与平方距离为近似；不提供退化拓扑认证。

上下链始终只保留左转，排序后每点在每条链最多入栈和出栈一次，因此扫描线性，总时间 O(n log n)。扫描预留至多 n+1 点；输出容量超过2h时先释放排序输入再紧凑复制，避免长期保留 n 规模容量。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **点集**：span<const Point2<T>>; duplicates and arbitrary order accepted
- **ownership_option**：ConvexPolygon2<T>::from_points(vector by value) permits std::move instead of input copy

### 输出

- **hull**：Owned strict CCW ConvexPolygon2<T>; lexicographic minimum first; no repeated closing point or intermediate collinear points
- **退化输入**：empty/identical/collinear input -> 0/1/2 vertices; integer topology exact

## 前置条件与约束

- **coordinates**：float64_abs_le_1e100
- **result_guarantee**：approximate_predicates
- **input_structure**：unordered_points
- **updates**：none

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

- **构建**：O(n log(n+1)) time, O(n) scan
- **空间**：16n copied input payload + <=16(n+1) hull capacity; O(log n) sort stack; move consumes input and avoids its copy
- **resident**：16*capacity() bytes; sparse output compacted after freeing input copy, capacity <=2h on tested STL; no retained n-point buffer for tiny hull

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

排序去重后建立上下链，仅保留所需转向；被移除的非凸点不可能成为相应链的凸包顶点。每点每条链最多入出栈一次，输出严格、规范的CCW凸边界；浮点版本另依赖谓词一致性。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[浮点几何谓词](geometry.predicates.float.md)（`geometry.predicates.float`）。
- 可替代：[凸包 · 整数](geometry.convex_hull.int.md)（`geometry.convex_hull.int`）。条件：Coordinate regime and result guarantee differ; check both contracts; no automatic conversion

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in this document; floating correctness conditional on numeric predicates; not formally verified
- **测试**：[tests/geometry_test.cpp](../../tests/geometry_test.cpp)
- **示例**：[examples/geometry2d.cpp](../../examples/geometry2d.cpp)
- **基准**：not_measured
- **补充**：Regression and degeneracy cases; random integer/float comparisons; closest/diameter quadratic oracles; rectangle all-edge oracle; circle membership/tangency properties

- **实现来源**：Reorganized and rewritten from user-provided template; original preserved byte-for-byte
- **参考**：provenance/geometry2d-template.cpp.txt; docs/project/GEOMETRY2D.md lists read-only Wiki and primary-source references
- **许可状态**：User-provided template; project distribution license not yet selected
