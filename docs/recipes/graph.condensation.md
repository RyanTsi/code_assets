# SCC 缩点配方

资产 ID：`graph.condensation`

类型：组合配方

问题：`condensation`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_condensation.json) | [公共头文件](../../include/algorithm_assets/graph/strong_components.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/strong_components.hpp>
```

`condense_graph(graph)` 是实际 SCC + 边归约配方，返回分区与新的无权 DAG。保留分量间平行弧，删除分量内部弧；不是对原图可逆的压缩。O(n+m)；除了 SCC 工作区，转换边临时最多 8m，新 DAG 8(c+1)+4m'，m'≤m，构建游标另计。重边不会破坏拓扑处理。不能用有向 SCC 代替无向割点/桥分析。

## 输入与输出

### 输入

- **条件**：Immutable unweighted CSR

### 输出

- **条件**：Owned partition and DAG, internal arcs removed, parallel external arcs kept

## 前置条件与约束

- **directed**：true
- **graph_updates**：none
- **parallel_arcs**：preserve

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Immutable unweighted CSR; Owned partition and DAG, internal arcs removed, parallel external arcs kept
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
- **空间**：SCC O(n); temporary 8m edges; output CSR and cursor
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

将每个顶点映射到 SCC，删除内部边而保留跨分量边。若商图存在有向环，环上的分量就相互可达，与 SCC 最大性矛盾，故结果是 DAG；内部结构与路径细节丢失，不可逆。

## 关联资产

- 依赖使用：[强连通分量](../algorithms/graph.strong_components.md)（`graph.strong_components`）。
- 依赖使用：[无权 CSR](../representations/graph.unweighted_csr.md)（`graph.unweighted_csr`）。

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
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
