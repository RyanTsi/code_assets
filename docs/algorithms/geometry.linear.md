# 直线与线段几何

资产 ID：`geometry.linear`

类型：算法

问题：`geometry.linear_operations`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_linear.json) | [公共头文件](../../include/algorithm_assets/geometry/linear.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/linear.hpp>
```

`Line::through/from_direction/from_equation` 构造锚点与单位方向表示。零方向、重合构造点和零法向量拒绝；线段允许零长度。`projection`、`reflection`、`closest_point`、`distance`、`contains`、`intersects` 与带类型的 `intersection` 无堆分配。直线交集为 none/finite/infinite；线段交集为 none/point/overlap，重叠端点按字典序返回。交点是浮点构造。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **line**：Line from two distinct points, nonzero direction, or finite Ax+By=C coefficients with nonzero normal
- **segment**：Independent endpoint value type; zero-length segments permitted
- **tolerance**：Explicit absolute/relative policy for classification, default documented in GEOMETRY2D.md

### 输出

- **projection_reflection**：Constructed Point; segment closest point clamps parameter
- **distances**：double, zero for intersections under selected tolerance
- **line_intersection**：none/finite one point/infinite coincident
- **segment_intersection**：none/point/overlap with lex-ordered interval endpoints

## 前置条件与约束

- **coordinates**：float64_abs_le_1e100
- **result_guarantee**：approximate
- **operation**：{"one_of": ["projection", "reflection", "perpendicular_bisector", "line_intersection", "segment_intersection", "point_line_distance", "point_segment_distance", "segment_distance", "parallel", "perpendicular", "contains", "intersects"]}

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

- **操作成本**：O(1)
- **空间**：O(1); fixed-capacity results; no heap allocation

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

投影由单位方向上的点积确定，线段最近点把参数截断到端点范围；交点联立方向方程，平行、重合和退化情况单独分类。近退化浮点构造仍受容差与舍入影响。

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
