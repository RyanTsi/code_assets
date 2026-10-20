# Dinic 最大流

资产 ID：`graph.dinic`

类型：算法

问题：`maximum_flow`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_dinic.json) | [公共头文件](../../include/algorithm_assets/graph/dinic.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/dinic.hpp>
```

`graph/dinic.hpp`。`Dinic(n)`，n≤INT32_MAX；`add_edge(from,to,uint64 capacity)` 返回稳定逻辑边 ID；`edge(id)` 返回 `{from,to,capacity,flow}`；`max_flow(s,t,limit=UINT64_MAX)` 返回 **本次新增流量**及 `maximal`；`reset_flow()` 将所有边流清零；`residual_reachable(source)` 返回残量可达标记。s≠t，自环、平行弧、反向逻辑边、容量 0 均合法。两条残量弧使用 uint32 链表索引，总弧数最多 UINT32_MAX-1。

每条逻辑边是两个 16 字节弧；初始容量由成对剩余容量之和恢复，不多存一份 capacity/flow 数组。成员顶点工作区本机约 20n，弧载荷 32m，扩容冗余另计；显式增广路径没有递归栈。加边摊销 O(1)，一般图最坏 O(n²m)，reset O(m)，残量可达 O(n+m) 加本地 O(n) 空间。层次图允许的弧逐步被当前弧游标或死端剪枝消耗，每个阻塞流阶段结束后最短增广路变长。

调用会改变残量网络；同一对端点可以继续增广，仍可加新边。改变端点前必须 reset；端点错误不改变流。limit=0 只检查是否已最大。真实最大流超过 uint64 时可分多次调用，单次严格不超过 limit，**达到 UINT64_MAX 且 maximal=false 不是溢出截断后的“最大流”**。只有 maximal=true 时源侧残量可达集才是最小割证据。不支持单边删除/容量缩小、历史撤销或费用流。加边与增广会影响此前最小割证据；普通值复制可备份全状态但成本 O(n+m)。

## 输入与输出

### 输入

- **条件**：Directed capacities, distinct valid terminals, optional uint64 flow limit

### 输出

- **条件**：Added flow, maximal flag; edge states and residual reachability; reset all flow

## 前置条件与约束

- **capacities**：uint64
- **costs**：none
- **graph_updates**：append_edges
- **flow_output**：added_and_maximal

- **最大 n**：2147483647
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Directed capacities, distinct valid terminals, optional uint64 flow limit; Added flow, maximal flag; edge states and residual reachability; reset all flow
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
- **并发读取**：有条件支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **操作成本**：General worst O(n^2*m); append amortized O(1); reachability O(n+m)
- **空间**：32m paired arcs, 20n member workspace; capacities/copies/allocator extra
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

BFS 给出残量最短路层次，阻塞流耗尽当前层次上的全部可增广路径；新阶段的最短增广路严格增长。成对残量弧保持容量与流守恒，只有确认 maximal 后才可用源侧可达集证明最小割。这是正确性论证，尚未进行形式化验证。

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
- **来源路径**：图论/网络流/Dinic.md
- **来源 blob**：4b404e064af74fc2ad4d454f34c31d683594c222
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
