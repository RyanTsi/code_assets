# 近似最小覆盖圆

资产 ID：`geometry.minimum_enclosing_circle`

类型：算法

问题：`geometry.minimum_enclosing_circle`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_minimum_enclosing_circle.json) | [公共头文件](../../include/algorithm_assets/geometry/minimum_enclosing_circle.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/minimum_enclosing_circle.hpp>
```

`geometry/minimum_enclosing_circle.hpp`：`minimum_enclosing_circle(span<Point>,seed)` 返回 optional<Circle>；空为 nullopt，重复/单点/共线允许，坐标有限且限±10⁵⁰。输入不变，缩放至单位范围，用 long double 临时坐标和有种子 shuffle，三层增量构造；三点行列式为0时取最远直径，不除零。最坏 O(n³)，均匀随机排列、精确运算下期望 O(n)；固定种子与浮点实现没有最坏线性保证。工作区 n 个临时点（2*sizeof(long double)*n）及固定 PRNG 状态，没有递归。

输出中心舍入后重新扫描全部点求覆盖半径，必要时向上舍入，确保不会仅因中心转换而直接缩小半径；**仍不是认证精确最小圆**。近共线/病态构造可能失准或拒绝非有限结果，long double 位数依平台，不提供绝对误差上界。种子复现以相同标准库/平台为条件。复杂度核对参考 [最小覆盖圆作者资料](https://www.nayuki.io/page/smallest-enclosing-circle)；实现按用户笔记的增量流程独立重写，未复制该外部库。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **条件**：Borrowed finite double points, seeded shuffle, repeated/collinear points supported

### 输出

- **条件**：optional Circle; empty nullopt; rounded center rescanned for radius; no certified minimality

## 前置条件与约束

- **coordinates**：float64_abs_le_1e50
- **result_guarantee**：approximate
- **updates**：none

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Borrowed finite double points, seeded shuffle, repeated/collinear points supported; optional Circle; empty nullopt; rounded center rescanned for radius; no certified minimality
- **异常与失败行为**：Invalid shape/index/domain rejected; allocations may throw; algorithm-specific overflow/tolerance behavior in documentation

## 状态与生命周期

- **持久化**：不支持
- **快照**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发写入**：不支持
- **并发读取**：支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **操作成本**：Worst O(n^3); ideal uniform-permutation exact-arithmetic expected O(n)
- **空间**：2*sizeof(long double)*n shuffled workspace plus fixed PRNG/local disks
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

随机增量扫描中，仅当新点在圆外才重建，并逐步把1/2/3个边界点固定。精确运算下最小圆由至多3个边界点确定；当前浮点实现的误差和固定种子不构成认证最优性或最坏线性证明。

## 关联资产

- 依赖使用：[圆交点、切线与构造](geometry.circle.md)（`geometry.circle`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/blog
- **固定提交**：6ea0ef09d8479c8775654c1627710af6da1d5392
- **来源路径**：content/posts/最小圆覆盖.md
- **来源 blob**：63b68bfada37e9abf33518739d494b21eda17e31
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
