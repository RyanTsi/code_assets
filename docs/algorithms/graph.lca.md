# 森林倍增 LCA

资产 ID：`graph.lca`

类型：算法

问题：`lowest_common_ancestor`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_lca.json) | [公共头文件](../../include/algorithm_assets/graph/lowest_common_ancestor.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/lowest_common_ancestor.hpp>
```

`graph/lowest_common_ancestor.hpp`。构造只读 uint32 parent 数组，根指向自身；支持森林，非根循环与非法父亲明确拒绝。`query(a,b)`、`ancestor(v,steps)`、`distance(a,b)` 返回 optional；不同树无 LCA/距离，超过根的 ancestor 无值。空数组合法但不能查询。保存 depth/root 和层优先跳表，层数按实际最大深度而不是固定 20，树根链不递归。构建 O(n log(d+1))，查询 O(log(d+1))，载荷 4n(2+max(1,bit_width(d)))；构建状态和路径临时 O(n)。距离 uint64 是边数，不是带权距离；无动态重挂父亲。

## 输入与输出

### 输入

- **条件**：uint32 parent array, roots point to themselves; cycles rejected

### 输出

- **条件**：optional ancestor/LCA/uint64 distance; different roots nullopt

## 前置条件与约束

- **representation**：parent_forest
- **updates**：none
- **距离**：edge_count

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：uint32 parent array, roots point to themselves; cycles rejected; optional ancestor/LCA/uint64 distance; different roots nullopt
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

- **操作成本**：Build O(n log(d+1)), query O(log(d+1))
- **空间**：4n*(2+max(1,bit_width(max_depth))) resident; O(n) construction scratch
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

跳表第k层保存2ᵏ级祖先，由两次2ᵏ⁻¹跳转建立。先将查询点抬到同深度，再从大到小同步抬升但不越过公共祖先，最终父亲即LCA；不同根直接无解。距离由两深度减去公共祖先深度两次得到。这是正确性论证，尚未进行形式化验证。

## 关联资产

当前契约未登记关联；这不代表不存在潜在组合。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：图论/其他/最近公共祖先.md
- **来源 blob**：5d4c16d352b14287ade89789ce4c11e3d14f0331
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
