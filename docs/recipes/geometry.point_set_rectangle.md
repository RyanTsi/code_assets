# 点集外接矩形配方

资产 ID：`geometry.point_set_rectangle`

类型：组合配方

问题：`geometry.minimum_rectangle`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_point_set_rectangle.json) | [公共头文件](../../include/algorithm_assets/geometry/point_set_bounds.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/point_set_bounds.hpp>
```

`minimum_bounding_rectangle(points)` 先构建凸包，再执行最小面积外接矩形扫描。凸包与原点集有相同的包含外接矩形，因此这一归约保持最优面积；空、点和线段沿用矩形组件约定。O(n log n+h)，矩形为浮点构造，即便输入为精确整数也没有精确矩形输出保证。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **点集**：Borrowed unordered span; hull builder handles duplicates/collinearity

### 输出

- **rectangle**：Same optional OrientedRectangle as minimum_area_rectangle; corners constructed approximately

## 前置条件与约束

- **coordinates**：{"one_of": ["float64_abs_le_1e100", "int64_abs_le_1e9"]}
- **result_guarantee**：approximate
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

- **whole_recipe**：O(n log(n+1)+h)
- **peak_space**：Hull copy <=16n + output <=16(n+1) + O(log n) stack; calipers only hull+O(1) after build

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

一个矩形包含点集当且仅当包含其凸包，因此先求凸包不改变最优包含面积。随后枚举凸包支撑边方向的矩形，得到可调用的归约组合；数值构造仍为近似。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[凸包 · 整数](../algorithms/geometry.convex_hull.int.md)（`geometry.convex_hull.int`）。条件：coordinates=int64_abs_le_1e9
- 依赖使用：[凸包 · 浮点](../algorithms/geometry.convex_hull.float.md)（`geometry.convex_hull.float`）。条件：coordinates=float64_abs_le_1e100
- 依赖使用：[凸包最小外接矩形](../algorithms/geometry.minimum_rectangle.md)（`geometry.minimum_rectangle`）。

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
