# 强连通分量

资产 ID：`graph.strong_components`

类型：算法

问题：`strong_components`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_strong_components.json) | [公共头文件](../../include/algorithm_assets/graph/strong_components.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/strong_components.hpp>
```

`graph/strong_components.hpp`。`strong_components(graph)` 返回拥有的 `component[v]`、`count`。迭代 Tarjan 用 discovery、low、活动栈和显式 DFS 帧，避免调用栈深度依赖 n；树边传播 low，活动返祖边使用 discovery[v]。O(n+m)，辅助 O(n)，本机每帧 16 字节，三个 uint32 数组（含输出）、活动栈和位标记另计。空图 count=0；每点都分配分量，含单点自环；ID 为逆拓扑序，跨分量边始终由大 ID 指向小 ID，不保证其它稳定编号语义。

## 输入与输出

### 输入

- **条件**：Immutable unweighted CSR

### 输出

- **条件**：Owned uint32 component IDs and count; reverse topological IDs

## 前置条件与约束

- **directed**：true
- **graph_updates**：none
- **representation**：unweighted_csr

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Immutable unweighted CSR; Owned uint32 component IDs and count; reverse topological IDs
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
- **空间**：O(n) explicit DFS frames, low/discovery/active arrays and output
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

Tarjan 保持活动 DFS 栈；low 记录到活动祖先的最早发现位置。low 等于 discovery 的根弹出一个完整 SCC；迭代帧模拟递归，返回子帧时传播 low。

## 关联资产

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
