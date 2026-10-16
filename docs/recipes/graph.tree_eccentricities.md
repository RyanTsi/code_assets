# 树上全部最远距离

资产 ID：`graph.tree_eccentricities`

类型：组合配方

问题：`tree_eccentricities`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_tree_eccentricities.json) | [公共头文件](../../include/algorithm_assets/recipes/tree_eccentricities.hpp) | [可编译示例](../../examples/problem_combinations.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/recipes/tree_eccentricities.hpp>
```

`tree_eccentricities(n, span<const UnweightedCsrGraph::Edge> edges, source=0)` 的边按无向边每条只列一次。`1 <= n < UINT32_MAX`、source 合法、恰好 n-1 边，必须连通；它内部双向建 CSR。返回所有点到指定 source 的单位边距离、各点到最远点的单位边距离，以及一对直径端点和直径长度。单点树返回两个零数组及直径 0。没有权重参数，不能直接处理不等长边、一般图或森林。

非法点数/边数/不连通抛 invalid_argument，越界 source 或边端点抛 out_of_range。先检查边数，再用 CSR 的第一轮遍历检查连通性：n-1 条边且连通的无向多重图必然是一棵简单树，因此自环、重边和非树环结构都无法通过，不额外保留 DSU。输入只借用、返回数组独立拥有，分配限制另外适用。

从 source 的最远点 a 是某条直径端点，从 a 的最远点 b 得到直径；树中任意点 v 的离心率为 `max(dist(v,a),dist(v,b))`。其依据是树上唯一路径：把支路投影到 a-b 直径上，任何支路高度不能使其到任一端点距离超过直径长度，故其它点到 v 的距离不超过 v 到这两个端点中的较大值。三次 BFS 得到所需数组，不使用递归或优先队列。

建图、三次遍历、合并都是 O(n)，读完所有边以及产生 n 个结果本就需要 Ω(n)，仅在该单位树输入/输出模型下达到渐进时间最优。队列重复使用，O(n) 存储，O(1) 调用栈。64-bit 平台 CSR 逻辑载荷约 16n 字节，遍历时队列 4n、三个距离数组 12n；建图时双向弧列表和 CSR/游标同时活跃，逻辑载荷约 40n，比遍历阶段更大。返回两个数组 8n 字节，不把输出存储隐去；实际分配/进程峰值未测量。

CF1822F 将树售价写成 `k*eccentricity[v]`，移动根的最少费用为 `c*dist(source,v)`，逐点取最大差值。官方 n<=200000、k,c<=1e9，乘积小于 2e14，int64 足够；这些业务系数只在适配器中处理。它也可用于无向树网络的中心候选、最坏延迟分析，而非仅服务该题。

## 输入与输出

### 输入

- **条件**：n>=1; n-1 once-listed edges; valid source; connected; CSR validates endpoints

### 输出

- **条件**：Owned uint32 source distances, eccentricities and diameter certificate

## 前置条件与约束

- **graph_class**：undirected_tree
- **edge_lengths**：unit
- **edge_listing**：once_undirected

- **最大 n**：4294967294
- **异常与失败行为**：Invalid domain rejected; documented arithmetic overflow rejected; allocations propagate; see complete contract

## 状态与生命周期

- **持久化**：不支持
- **快照**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **操作成本**：O(n) time and storage; O(1) stack; returned 8n bytes on 64-bit
- **保证范围**：Worst-case asymptotic bounds, no calibrated latency or peak-memory claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

连通且有n-1条无向边保证输入为树。两次最远点遍历得到直径端点a、b，树上唯一路径性质给出 eccentricity(v)=max(dist(v,a),dist(v,b))；另一次指定源遍历提供移动距离。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[无权 CSR](../representations/graph.unweighted_csr.md)（`graph.unweighted_csr`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Documented sweep, DP and tree metric invariants; not formal verification
- **测试**：[tests/problem_combinations_test.cpp](../../tests/problem_combinations_test.cpp)
- **示例**：[examples/problem_combinations.cpp](../../examples/problem_combinations.cpp)
- **基准**：not_measured
- **判题证据**：[judge/COMBINATION_VALIDATION.md](../../judge/COMBINATION_VALIDATION.md)

- **来源仓库**：RyanTsi/blog
- **固定提交**：6ea0ef09d8479c8775654c1627710af6da1d5392
- **来源路径**：content/posts/图论做题记录.md
- **来源 blob**：adfcbb9da0bced9f62d6f311f5e6fcaf34083197
- **实现来源**：Independent engineering implementation; source supplies problem entry and motivation
- **许可状态**：No repository license discovered; distribution license not selected
