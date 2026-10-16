# 极角排序

资产 ID：`geometry.polar_sort`

类型：算法

问题：`geometry.polar_sort`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_polar_sort.json) | [公共头文件](../../include/algorithm_assets/geometry/polygon.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/polygon.hpp>
```

`lowest_leftmost` 对空输入返回 nullopt。`sort_by_angle` 从正 x 轴逆时针排序，零向量在前，同角按平方距离和精确字典序；预计算角键，原地改写输入顺序。比较器不使用 EPS，避免破坏严格弱序；临时键在本机约 32n 字节，平台布局可能不同。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **点集**：Writable span<Point>; origin Point; all values finite and bounded

### 输出

- **order**：In-place CCW order starting positive x; zero vector first; angular-key ties by squared distance then exact lex order; no epsilon comparator

## 前置条件与约束

- **coordinates**：float64_abs_le_1e100
- **result_guarantee**：approximate
- **input_structure**：unordered_points
- **input_mutation**：allowed

- **最大 n**：1152921504606846975
- **coordinates**：Float: finite |x|,|y|<=1e100; integer: |x|,|y|<=1e9. Out-of-domain inputs throw invalid_argument.
- **数值限制**：Integer predicate/squared-distance operations exact within bound; floating operations approximate, with long double intermediates; no exact floating kernel. Underflow and near-degenerate topology are not certified. No fast-math.
- **所有权**：Input order is mutated only after successful key construction/sort. No undo/history; caller may explicitly keep a copy.
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

- **sort**：O(n log(n+1)), n atan2 evaluations
- **空间**：n*sizeof(Key) (32n on tested MinGW, may be 24n on MSVC) plus O(log n) sort stack

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

先计算每点相对于原点的角键，再按角、距离与精确键排序；比较不依赖全局 EPS，不引入非传递的近似相等关系。

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
