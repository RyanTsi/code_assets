# 浮点几何谓词

资产 ID：`geometry.predicates.float`

类型：算法

问题：`geometry.predicates`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_predicates_float.json) | [公共头文件](../../include/algorithm_assets/geometry/primitives.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/primitives.hpp>
```

`orientation` 返回方向三态，`relative_position` 返回含退化情况的位置六态，`squared_distance` 与开方得到的 `distance` 分开。浮点坐标有限且限 ±10¹⁰⁰，谓词与平方距离为近似；不提供退化拓扑认证。 不把精确平方距离的开方结果也称为精确。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **点集**：Two/three valid coordinates; no topology data structure
- **tolerance**：Float orientation may explicitly accept Tolerance; no epsilon sorting

### 输出

- **orientation**：CW/collinear/CCW
- **position**：left/right/before/after/on_segment/degenerate; a=b,p!=a is degenerate
- **squared_distance**：int64 exact up to 8e18, or long double approximate; sqrt distance remains approximate

## 前置条件与约束

- **coordinates**：float64_abs_le_1e100
- **result_guarantee**：approximate
- **operation**：{"one_of": ["orientation", "relative_position", "squared_distance"]}

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
- **空间**：O(1), no allocations

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

方向由二维叉积符号决定，平方距离是位移各分量平方和；整数在声明坐标界内精确，浮点版本仅近似，零容差不能消除舍入误差。

## 关联资产

- 依赖使用：[二维几何基础](../representations/geometry.primitives.md)（`geometry.primitives`）。
- 可替代：[整数几何谓词](geometry.predicates.int.md)（`geometry.predicates.int`）。条件：Coordinate regime and result guarantee differ; check both contracts; no automatic conversion

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
