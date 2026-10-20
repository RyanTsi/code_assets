# 凸形平移碰撞配方

资产 ID：`geometry.translated_convex_overlap`

类型：组合配方

问题：`geometry.translated_overlap`

[文档目录](../README.md) | [结构化契约](../../contracts/geometry_translated_convex_overlap.json) | [公共头文件](../../include/algorithm_assets/geometry/minkowski.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry/minkowski.hpp>
```

`TranslatedConvexOverlap(a,b)` 持有 A-B；`intersects(translation)` 在 O(log(h+k)) 判断 B 平移该向量后是否与 A 相交，包含边界接触。平移坐标也限±10⁹。差并非和的可逆运算，不支持旋转、缩放、非凸碰撞或穿透深度。需要输入点集时先复用已有凸包构建，成本另计。

几何数值公共约定见 [坐标、容差与所有权约定](../project/GEOMETRY2D.md)；具体输入限制以下列本资产契约为准。

## 输入与输出

### 输入

- **条件**：A/B validated convex shapes; translation within +/-1e9

### 输出

- **条件**：Exact intersection bool including touching; stores owned difference hull

## 前置条件与约束

- **coordinates**：int64_abs_le_5e8
- **input_structure**：validated_convex_polygons
- **transform**：translation
- **boundary_contact**：true
- **result_guarantee**：exact

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：A/B validated convex shapes; translation within +/-1e9; Exact intersection bool including touching; stores owned difference hull
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

- **操作成本**：Build O(n+m); each query O(log(n+m+1))
- **空间**：At most16*(n+m) resident difference hull; O(1) query scratch
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

A 与 B+t 相交当且仅当存在 a=b+t，即 t∈A-B。预计算差凸包后对 t 做凸包二分定位即可，闭边界保证接触计为相交。

## 关联资产

- 依赖使用：[整数闵可夫斯基差](../algorithms/geometry.minkowski_difference.int.md)（`geometry.minkowski_difference.int`）。
- 依赖使用：[凸多边形二分定位 · 整数](../algorithms/geometry.convex_location.int.md)（`geometry.convex_location.int`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/blog
- **固定提交**：6ea0ef09d8479c8775654c1627710af6da1d5392
- **来源路径**：content/posts/闵可夫斯基和.md
- **来源 blob**：9ff69cf7e4a67652a2671bb00d6fa29dfd14d299
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
