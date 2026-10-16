# 二维几何基础

资产 ID：`geometry.primitives`

类型：数据表示

问题：`geometry.representation`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_primitives.json) | [公共头文件](../../include/algorithm_assets/geometry/primitives.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/primitives.hpp>
```

`Point2<T>` 与 `Vector2<T>` 区分位置和位移，T 限 double 或 int64_t；Point/Vector 与 IntegerPoint 为相应别名，两个坐标连续存储。点减点得到向量、点加向量得到点。Tolerance 仅服务显式的近似比较；精确坐标相等与 almost_equal 不可互换。低层 dot/cross/向量运算要求调用者保证中间值可表示。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **点集**：Point2<double> or Point2<int64_t>; initialized coordinates; exact equality
- **vectors**：Vector2<T>, position/direction kept distinct; raw arithmetic must stay representable

### 输出

- **layout**：16-byte point/vector values; no ownership of arrays; Turn/RelativePosition/Containment enums; fixed-capacity IntersectionSet

## 前置条件与约束

- **coordinates**：{"one_of": ["float64_abs_le_1e100", "int64_abs_le_1e9"]}
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

- **操作成本**：O(1)
- **空间**：16 bytes per point/vector; fixed-size value types, no allocation

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

位置与位移分型限定合法算术，坐标域校验阻止高层算法越出声明的数值范围。数据表示本身不构成全部下游算法的正确性证书。

## 关联资产

当前契约未登记关联；这不代表不存在潜在组合。

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
