# 二分图匹配与最小点覆盖

资产 ID：`graph.hopcroft_karp`

类型：算法

问题：`bipartite_matching`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_hopcroft_karp.json) | [公共头文件](../../include/algorithm_assets/graph/bipartite_matching.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/bipartite_matching.hpp>
```

`graph/bipartite_matching.hpp`。`hopcroft_karp(left_size,right_size,graph)` 返回左右配对数组、匹配数和最小点覆盖位图。左右 ID 是独立空间；graph 的顶点数必须等于 max(left,right)，只有 `[0,left)` 行可有弧，目标在 `[0,right)`。输入不可变，重边允许，空侧合法；未匹配项是 `BipartiteMatching::unmatched`（UINT32_MAX）。它处理无权二分图，不是最小费用指派或一般图匹配。

BFS 找最短增广长度，显式栈 DFS 同时增广一批不相交路径；最坏 O((n+m)√n)，辅助 O(n)，不递归。Konig 最小点覆盖为交替路径可达左点的补集加可达右点；结果配对载荷 4(left+right)，覆盖使用位存储。DFS 游标 size_t[left]、层次 uint32[left]、队列和路径 O(left)，输入 CSR 另计。与流的单位容量建模在条件上有关联，但实现直接运行 HK，未伪造对 Dinic 的依赖。

## 输入与输出

### 输入

- **条件**：Separate left/right IDs, CSR n=max(left,right); rows left only, targets right

### 输出

- **条件**：Owned matching arrays, size and minimum vertex cover bitmaps

## 前置条件与约束

- **weighted**：false
- **graph_updates**：none
- **output**：matching_and_min_vertex_cover

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Separate left/right IDs, CSR n=max(left,right); rows left only, targets right; Owned matching arrays, size and minimum vertex cover bitmaps
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

- **操作成本**：O((n+m)*sqrt(n)) worst
- **空间**：O(n) workspace and output; borrowed CSR extra
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每阶段 BFS 确定最短增广长度，DFS 增广一批顶点互不相交的最短路。无增广路时匹配最大；从未匹配左点出发的交替可达集给出 König 最小点覆盖。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[无权 CSR](../representations/graph.unweighted_csr.md)（`graph.unweighted_csr`）。
- 可替代：[Dinic 最大流](graph.dinic.md)（`graph.dinic`）。条件：Only after explicit unit-capacity source/left/right/sink reduction; HK does not call Dinic

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：图论/网络流/匈牙利算法.md
- **来源 blob**：923e5934ee55a675aded602c9f745b2124df904b
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
