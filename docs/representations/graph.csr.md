# CsrGraph

资产 ID：`graph.csr`

类型：数据表示

问题：`graph_storage`

[文档目录](../README.md) | [结构化契约](../../contracts/csr_graph.json) | [公共头文件](../../include/algorithm_assets/graph/csr_graph.hpp) | [可编译示例](../../examples/first_algorithms.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/csr_graph.hpp>
```

头文件：`graph/csr_graph.hpp`。`CsrGraph(n, edges)` 将 `{from,to,int64 weight}` 的边列表复制成不可变有向图，顶点为 uint32，权重非负且不超过 INT64_MAX，`n < UINT32_MAX`。空图、重边、自环合法；无向边需显式传入两个方向。

`neighbors(v)` 返回两个等长 const span：targets 与 weights；每个顶点内保留输入边的顺序。借用视图依赖图存储，不跨销毁、赋值或移动使用；移动后的原对象只用于销毁或重新赋值。

先计数并求前缀和，再分发边，故每个顶点对应一段连续存储。目标与权重分成两个数组，避免每条边结构体的对齐填充。构造 Theta(n+m)，邻接视图 O(1)；逻辑载荷 `sizeof(size_t)*(n+1)+12m` 字节，构建游标另需 `sizeof(size_t)*(n+1)`，输入边列表另计。

不可变图可供并发读取或多个独立 Dijkstra 调用。负权拒绝构造：这个首批存储变体有意服务非负整数最短路，不能直接作为所有图算法的通用图类型。

## 输入与输出

### 输入

- **edges**：Borrowed span<Edge>{uint32 from,to,int64 weight}; copied at construction
- **weights**：[0, INT64_MAX]
- **structure**：Directed arcs; loops and parallel arcs accepted; encode undirected edges as two arcs

### 输出

- **neighbors**：Paired const spans for targets and weights in insertion order within a vertex

## 前置条件与约束

- **directed**：true
- **graph_updates**：none
- **weights**：nonnegative_int64

- **最大 n**：4294967294
- **异常与失败行为**：Negative weights rejected; invalid endpoints/vertices throw; allocation may throw
- **lifetime**：Neighbor views depend on graph storage; no mutation while used; moved-from instance only destroy/reassign
- **并发条件**：Concurrent const access on the same live immutable graph

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **construction**：Theta(n+m)
- **neighbors**：O(1) view construction
- **空间**：sizeof(size_t)*(n+1) + 12m payload bytes; build cursor adds sizeof(size_t)*(n+1)
- **保证范围**：worst case

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每点的出边计数前缀和给出互不重叠的连续区间；独立写入游标将每条输入边恰好放入其起点对应区间。目标与权值同位置存储，邻接视图长度和顺序一致。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 提供表示：[Dijkstra with indexed heap](../algorithms/graph.dijkstra.md)（`graph.dijkstra`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formally verified
- **测试**：[tests/algorithms_test.cpp](../../tests/algorithms_test.cpp)
- **示例**：[examples/first_algorithms.cpp](../../examples/first_algorithms.cpp)
- **基准**：not_measured
- **补充**：Negative weights, endpoints, loops, parallel arcs and empty graph exercised through graph tests

- **实现来源**：Written for this project; no source code copied
- **参考**：../oi-wiki-reference/docs/ and standard algorithm definitions
- **许可状态**：Project distribution license not yet selected
