# 圆交点、切线与构造

资产 ID：`geometry.circle`

类型：算法

问题：`geometry.circle_operations`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_circle.json) | [公共头文件](../../include/algorithm_assets/geometry/circle.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/circle.hpp>
```

圆模块区分圆周交点与闭圆盘区域；Circle 的中心和非负半径均须有限。本页记录圆模块已登记的一组常数空间操作，圆盘与多边形判断另有独立资产。

`circle.hpp` 除圆与多边形的区域交集外，都是 O(1) 时间/空间、无堆分配。`IntersectionSet<T,N>` 的 `[0,count)` 是有效值，其他数组槽不具有输出意义；`kind` 区分 none、finite、infinite。

| API | 输入输出与特殊情况 |
|---|---|
| `locate(Circle,p)` | 圆盘内部、圆周、圆盘外；半径 0 的点圆允许 |
| `intersection(Circle,Line/Segment)` | **圆周**交点 0/1/2；线段端点和相切接触保留，零线段在圆周上为一个点；线段完全在圆盘内没有圆周交点 |
| `intersection(Circle,Circle)` | 圆周交点 0/1/2 或 infinite；同心不同半径 none；完全重合正半径 infinite；同一个点圆一个点；不把“近似重合”判为无限，无限分支要求坐标/半径精确相同 |
| `tangent_points` | 圆内点无切点，圆上点一个，圆外两个；点圆的唯一接触点是圆心。不是从圆内点开负数平方根 |
| `common_tangents`, `common_tangent_count` | 公切线固定容量最多 4；相切构造用切点+垂直法向量，避免两个相同端点；完全重合圆为 infinite、count API nullopt；两不同点圆一条直线；点圆与普通圆显式处理去重 |
| `circumcircle`, `incircle` | 退化/容差判为共线的三点 nullopt；有效结果 Circle；近退化构造超坐标/半径界仍可能抛异常 |
| `centers_through(a,b,r)` | 过两点的给定半径圆心 0/1/2 或 infinite；a=b,r>0 为无限，a=b,r=0 为唯一圆心 |
| `overlap_area` | **闭圆盘**公共面积；相离/外切 0，包含返回小圆面积；余弦 clamp；近外切使用 `theta-sin(theta)` 级数避免相消。此 API 不用容差提前吞掉薄透镜 |

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **circles**：Immutable Circle(center, radius); point circles permitted; finite nonnegative radius<=1e100
- **line_segment_points**：Nondegenerate Line or possibly point Segment; triangles may degenerate
- **tolerance**：Used for classification except overlap_area; explicitly configured units/scale

### 输出

- **intersections**：Circumference: 0/1/2 points or infinite for identical positive circle; coincident point circles one point
- **tangents**：0..4 Lines or infinite; count nullopt means infinite, inside-point tangent_points empty; point-circle degeneracies defined
- **triangle_circles**：optional<Circle>, nullopt if collinear/degenerate under policy
- **centers**：0/1/2/infinite; repeated point and positive radius infinite
- **overlap**：Closed-disk overlap area, NOT circumference intersection count

## 前置条件与约束

- **coordinates**：float64_abs_le_1e100
- **result_guarantee**：approximate
- **operation**：{"one_of": ["locate", "circle_line_intersection", "circle_segment_intersection", "circle_circle_intersection", "tangent_points", "common_tangents", "common_tangent_count", "circumcircle", "incircle", "centers_through", "overlap_area"]}

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
- **空间**：O(1), arrays of <=2 points or <=4 lines; no heap allocation

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

各圆周交点问题归约为圆心连线及其垂线上的一维距离；相离、相切、包含、同心和重合分别分类。切线依切点和垂直法向量构造；面积的薄透镜分支使用稳定级数减少相消。所有浮点分支仍是近似。

## 关联资产

- 依赖使用：[直线与线段几何](geometry.linear.md)（`geometry.linear`）。

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
