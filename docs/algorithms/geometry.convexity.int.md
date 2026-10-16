# 凸性检查 · 整数

资产 ID：`geometry.convexity.int`

类型：算法

问题：`geometry.convexity`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_convexity_int.json) | [公共头文件](../../include/algorithm_assets/geometry/polygon.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/polygon.hpp>
```

`is_convex(points,allow_collinear)` 接受 CW 或 CCW 的简单多边形；默认拒绝共线相邻边。不足三点、全共线或连续重复点返回 false；不检查自交，简单性必须由调用方保证。整数坐标限 ±10⁹，符号与平方距离在界内精确。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **polygon**：SIMPLE boundary in either winding; optionally repeated final first vertex
- **allow_collinear**：Default false; true permits intermediate collinear boundary vertices

### 输出

- **convex**：bool; <3/all collinear/consecutive duplicate => false; not a self-intersection validator

## 前置条件与约束

- **coordinates**：int64_abs_le_1e9
- **result_guarantee**：exact
- **simple_polygon**：true
- **input_structure**：ordered_boundary
- **allow_collinear**：{"one_of": [true, false]}

- **最大 n**：1152921504606846975
- **coordinates**：Float: finite |x|,|y|<=1e100; integer: |x|,|y|<=1e9. Out-of-domain inputs throw invalid_argument.
- **数值限制**：Integer predicate/squared-distance operations exact within bound; floating operations approximate, with long double intermediates; no exact floating kernel. Underflow and near-degenerate topology are not certified. No fast-math.
- **所有权**：Inputs borrowed for call and not mutated unless explicitly stated; returned coordinates owned; no implicit I/O or global mutable state.
- **并发条件**：Concurrent calls may share read-only input; caller must not write input during a call.
- **异常与失败行为**：Invalid domain throws invalid_argument; constructed coordinates outside +/-1e100 or nonfinite scalar result throw overflow_error; detectable distinct-point squared-distance underflow throws underflow_error; allocation/size may throw.
- **历史状态**：No persistence, branching, snapshots, rollback, undo or redo. Ordinary value copying is not persistent storage.
- **topology**：SIMPLE boundary is a precondition. For full strict-boundary validation use ConvexPolygon2<T>::from_boundary, O(n log n).

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

- **查询**：O(n)
- **空间**：O(1), no allocations

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

对简单多边形逐边检查非零转向的一致性，同时执行共线与重复点策略；局部转向条件不能替代简单性前提。

## 关联资产

- 依赖使用：[二维几何基础](../representations/geometry.primitives.md)（`geometry.primitives`）。

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
