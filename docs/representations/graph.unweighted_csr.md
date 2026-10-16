# 无权 CSR

资产 ID：`graph.unweighted_csr`

类型：数据表示

问题：`unweighted_graph_storage`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_unweighted_csr.json) | [公共头文件](../../include/algorithm_assets/graph/unweighted_csr.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/unweighted_csr.hpp>
```

`graph/unweighted_csr.hpp`。构造 `(n,span<Edge{uint32 from,to}>)` 拷贝到不可变压缩邻接表；`neighbors(v).targets` 是借用视图，`vertex_count/edge_count/storage_bytes` 可查询。n ≤ UINT32_MAX-1；顶点/端点不合法抛 out_of_range。空图、自环、重边合法，保留邻接输入顺序。构建 O(n+m)，常驻 8(n+1)+4m，临时构建游标 8(n+1)。不含权值、不支持增删边，视图随对象存储有效。

## 输入与输出

### 输入

- **条件**：n vertices; borrowed uint32 directed edges; loops and parallel arcs

### 输出

- **条件**：Owned immutable CSR, borrowed neighbor spans

## 前置条件与约束

- **directed**：true
- **graph_updates**：none

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：n vertices; borrowed uint32 directed edges; loops and parallel arcs; Owned immutable CSR, borrowed neighbor spans
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

- **操作成本**：O(n+m) build, O(1) view
- **空间**：8(n+1)+4m resident; 8(n+1) construction cursor
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

出度的前缀和划分邻接区间；按输入次序使用各点游标写入目标，保证每条边只出现一次并保留同起点边的输入顺序。只存目标索引，无权值数组。这是正确性论证，尚未进行形式化验证。

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
- **来源路径**：图论/连通分量/tarjan.md
- **来源 blob**：fad8a132b4547b729d4e497c8fb08f72730be8f6
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
