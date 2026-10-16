# 最近点对 · 整数

资产 ID：`geometry.closest_pair.int`

类型：算法

问题：`geometry.closest_pair`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_closest_pair_int.json) | [公共头文件](../../include/algorithm_assets/geometry/closest_pair.hpp) | [可编译示例](../../examples/geometry2d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/closest_pair.hpp>
```

`closest_pair<T>` 或 `ClosestPairWorkspace<T>::solve` 接收任意点集；n<2 为 nullopt，重复位置可产生坐标相同、平方距离为0的点对。结果拥有坐标值而不含原始索引，并列任取；`release()` 释放两份工作缓存，同实例不能并发 solve。整数坐标限 ±10⁹，符号与平方距离在界内精确。

分治双缓冲进入子区间时保持 x 顺序，子调用交换源/输出角色，返回 y 有序结果；父层只归并一次，旧源缓冲可用作条带。几何打包界限制条带候选，T(n)=2T(n/2)+O(n)。平方横纵界限避免内层开方，显式工作区避免 inplace_merge 的隐式分配和退化。复用后应按 capacity 而非本轮 n 计算保留内存。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **点集**：span<const Point2<T>>; borrowed, preserved order; input must NOT alias this workspace's private buffers
- **工作区**：Optional separate ClosestPairWorkspace<T>; capacity reusable across calls

### 输出

- **点对**：optional<PointPair<T>> with coordinate values and squared_distance, not original indices; ties arbitrary
- **退化输入**：n<2 -> nullopt, duplicates -> distinct input positions with equal coordinate values and distance squared 0
- **距离**：sqrt-derived double approximate, even for exact integer squared distance

## 前置条件与约束

- **coordinates**：int64_abs_le_1e9
- **result_guarantee**：exact_squared_distance
- **input_structure**：unordered_points
- **updates**：none

- **最大 n**：1152921504606846975
- **coordinates**：Float: finite |x|,|y|<=1e100; integer: |x|,|y|<=1e9. Out-of-domain inputs throw invalid_argument.
- **数值限制**：Integer predicate/squared-distance operations exact within bound; floating operations approximate, with long double intermediates; no exact floating kernel. Underflow and near-degenerate topology are not certified. No fast-math.
- **所有权**：Inputs borrowed for call and not mutated unless explicitly stated; returned coordinates owned; no implicit I/O or global mutable state.
- **并发条件**：Concurrent calls may share read-only input; caller must not write input during a call.
- **异常与失败行为**：Invalid domain throws invalid_argument; constructed coordinates outside +/-1e100 or nonfinite scalar result throw overflow_error; detectable distinct-point squared-distance underflow throws underflow_error; allocation/size may throw.
- **历史状态**：No persistence, branching, snapshots, rollback, undo or redo. Ordinary value copying is not persistent storage.
- **工作区状态**：solve mutates workspace. Separate instances for concurrent calls; failure leaves reusable but unspecified buffer contents/capacities. Returned pair is independent.
- **基准**：Only floating implementation measured in geometry_results.csv; integer performance not measured

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发写入**：不支持
- **并发读取**：有条件支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **求解**：O(n log(n+1)) under consistent geometric comparisons
- **空间**：Normally 32n point payload in two buffers for a fresh workspace; duplicate exit may need only 16n; O(log n) stack; reused capacities can exceed current n; release() frees both buffers
- **热路径**：No heap allocations once both capacities suffice on tested STL; no sqrt in strip; alternating buffer merge avoids per-level copy-back

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

分治返回两半内部的最近对及y有序点列；跨半最优对必须位于中线条带。两半已有最小距离给出常数候选的几何打包界，线性归并与条带扫描形成T(n)=2T(n/2)+O(n)。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[二维几何基础](../representations/geometry.primitives.md)（`geometry.primitives`）。
- 可替代：[最近点对 · 浮点](geometry.closest_pair.float.md)（`geometry.closest_pair.float`）。条件：Coordinate regime and result guarantee differ; check both contracts; no automatic conversion

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
