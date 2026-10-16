# 多边形面积与周长

资产 ID：`geometry.polygon_measures`

类型：算法

问题：`geometry.polygon_measures`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_polygon_measures.json) | [公共头文件](../../include/algorithm_assets/geometry/polygon.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/polygon.hpp>
```

`signed_area`、`area`、`perimeter` 与 `triangle_area` 接收有序边界；空、点、线段面积为零，线段的闭合周长是两倍长度。允许闭合点重复；自交边界的 signed_area 是代数面积。平移后鞋带求和并使用 CompensatedSum 减少相消，不能据此宣称精确浮点面积。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **polygon**：Borrowed ordered span, any winding; optional repeated closing point; no self-intersection validation

### 输出

- **area**：signed/absolute double algebraic area; empty/point/segment zero; integer accumulation output still approximate
- **perimeter**：Closed boundary length; a segment counted twice; empty zero

## 前置条件与约束

- **coordinates**：{"one_of": ["float64_abs_le_1e100", "int64_abs_le_1e9"]}
- **result_guarantee**：approximate
- **input_structure**：ordered_boundary
- **operation**：{"one_of": ["signed_area", "area", "perimeter"]}

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
- **空间**：O(1), no allocation; Neumaier compensation

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

鞋带公式累加有向边叉积给出代数面积，绝对值才是简单多边形区域面积；周长逐段求长度并闭合。平移和补偿求和改善误差，不改变几何定义。

## 关联资产

- 依赖使用：[二维几何基础](../representations/geometry.primitives.md)（`geometry.primitives`）。
- 依赖使用：[Neumaier compensated sum](numeric.compensated_sum.md)（`numeric.compensated_sum`）。

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
