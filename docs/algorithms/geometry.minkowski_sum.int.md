# 整数闵可夫斯基和

资产 ID：`geometry.minkowski_sum.int`

类型：算法

问题：`geometry.minkowski_sum`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_minkowski_sum_int.json) | [公共头文件](../../include/algorithm_assets/geometry/minkowski.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/minkowski.hpp>
```

`geometry/minkowski.hpp`：`minkowski_sum(a,b)` 接受已验证 IntegerConvexPolygon，每个坐标限±5×10⁸，输出限±10⁹以保持现有整数内核。空输入得空；单点、线段、多边形均支持。严格凸边界按极角合并，平行同方向边同时推进，再旋转到最小字典序顶点；最坏 O(h+k)，输出最多 h+k 点（16字节/点），不额外排序/重建凸包；与 O(hk) 所有点对求和后凸包做独立对照。借内部工厂保留已证明的 canonical hull 不变量，未增加公开 unchecked 构造。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **条件**：Validated strict integer convex boundaries; points/segments/empty allowed

### 输出

- **条件**：Owned canonical exact IntegerConvexPolygon; output coordinate bound1e9

## 前置条件与约束

- **coordinates**：int64_abs_le_5e8
- **input_structure**：validated_convex_polygons
- **result_guarantee**：exact

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Validated strict integer convex boundaries; points/segments/empty allowed; Owned canonical exact IntegerConvexPolygon; output coordinate bound1e9
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

- **操作成本**：O(n+m)
- **空间**：At most16*(n+m) output payload; constant merge scratch
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

严格凸边界的边向量已经按极角有序，归并两个边序列并累加，平行同向边一起前进，得到和集的规范凸边界。退化点/线段独立处理，坐标界保持整数计算安全。

## 关联资产

- 依赖使用：[凸包 · 整数](geometry.convex_hull.int.md)（`geometry.convex_hull.int`）。条件：Consumes validated IntegerConvexPolygon representation; does not recompute its hull

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：计算几何/闵可夫斯基和.md
- **来源 blob**：655d80c7d2356beb7aefcca3da4f42431786efe4
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
