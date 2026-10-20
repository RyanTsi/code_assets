# 整数线段相交

资产 ID：`geometry.integer_segments`

类型：算法

问题：`geometry.segment_predicates`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_integer_segments.json) | [公共头文件](../../include/algorithm_assets/geometry/linear.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/linear.hpp>
```

`contains(IntegerSegment,p)` 与 `intersects(IntegerSegment,IntegerSegment)` 使用整数方向和闭包围盒判断；允许端点接触、共线重叠和零线段。输入坐标限 ±10⁹；此资产提供精确判定，不构造有理交点。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **segments**：IntegerSegment with valid bounded coordinates; includes point segments

### 输出

- **predicate**：Exact bool for point on closed segment or intersection of two closed segments; no rational intersection construction

## 前置条件与约束

- **coordinates**：int64_abs_le_1e9
- **result_guarantee**：exact
- **operation**：{"one_of": ["contains", "intersects"]}

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
- **空间**：O(1), no allocation

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

两条非退化线段相交需满足方向跨立，零方向时结合闭包围盒检查共线投影重叠；退化点段单独退化为 membership。±10⁹坐标界使两个乘积的和差落在 int64 内。

## 关联资产

- 依赖使用：[整数几何谓词](geometry.predicates.int.md)（`geometry.predicates.int`）。

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
