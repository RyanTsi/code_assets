# Dijkstra with indexed heap

资产 ID：`graph.dijkstra`

类型：算法

问题：`shortest_paths`

[文档目录](../README.md) | [结构化契约](../../contracts/dijkstra.json) | [公共头文件](../../include/algorithm_assets/graph/dijkstra.hpp) | [可编译示例](../../examples/first_algorithms.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/dijkstra.hpp>
```

头文件：`graph/dijkstra.hpp`。`dijkstra(graph, source)` 返回自有的 `ShortestPaths`，支持 `distance_to(v)`、`predecessor(v)` 和 `path_to(v)`。source 必须合法，空图不能调用。

距离是 optional<uint64>，合法有限值为 `[0,UINT64_MAX-1]`，nullopt 严格表示不可达。源点路径为单元素序列，不可达路径为空；源点和不可达点的 predecessor 为 nullopt。多个最短路中返回其中一条，不承诺字典序最小。

非负权保证每次取出的最小暂定距离已是最短距离；只在严格改进时设置父节点，避免零权环形成父指针循环。索引堆为每个顶点最多保留一项，减少键值时上浮，不保留过时重复项。

权值及加法经过范围处理：太长的候选途径不可能改善可表示的距离，可跳过。但如果确有溢出候选，搜索完成后检查图可达性，确保不会把超出表示范围的可达点误报为不可达；出现这种点时抛 overflow_error。仅有超长绕路而最短路可表示时仍正常返回。

令 r 为成功松弛次数，r <= m：时间 O(n+m+(n+r)*log(n+1))。结果逻辑载荷 12n 字节，堆及位置数组最多 8n；溢出分支释放堆后改用可达性工作区，最多约 5n 逻辑字节。图存储、vector 实际容量、返回路径分配另计。`path_to` 时间与输出空间均为 O(路径长度)。

结果不依赖图继续存活，允许并发 const 读取。当前没有负权、动态图增量更新、多源 API 或浮点权值变体。

## 输入与输出

### 输入

- **graph**：Immutable CsrGraph
- **source**：A valid vertex; empty graph has no valid source

### 输出

- **distances**：optional<uint64>: exact [0,UINT64_MAX-1], nullopt means unreachable
- **来源路径**：Vertex sequence; empty iff unreachable; source path contains source
- **predecessor**：nullopt for source or unreachable; shortest path need not be unique

## 前置条件与约束

- **weights**：nonnegative_int64
- **graph_updates**：none
- **sources**：single
- **result_guarantee**：exact

- **最大 n**：4294967294
- **异常与失败行为**：Invalid source/target throws; a reachable shortest distance >= UINT64_MAX throws overflow_error
- **溢出**：Large nonoptimal detours are skipped, not confused with unreachable vertices; conditional reachability pass distinguishes true overflow
- **并发条件**：Independent calls can share graph; result supports concurrent const reads
- **所有权**：Returned distances/parents owned; graph need not outlive result

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

- **search**：O(n+m+(n+r)*log(n+1)), r <= m successful relaxations
- **空间**：12n result bytes + 8n indexed heap payload; graph/input excluded; overflow-only reachability replaces heap with <=5n logical payload
- **来源路径**：O(path_length) time and output space
- **保证范围**：worst-case traversal; no lazy heap duplicates

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

非负权保证最小暂定距离一旦出堆就是最终最短距离。严格改进才设置父节点，避免零权环形成父指针环。溢出候选另行触发可达性核查，不能将超表示范围的可达点误报为不可达。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[CsrGraph](../representations/graph.csr.md)（`graph.csr`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formally verified
- **测试**：[tests/algorithms_test.cpp](../../tests/algorithms_test.cpp)
- **示例**：[examples/first_algorithms.cpp](../../examples/first_algorithms.cpp)
- **基准**：not_measured
- **补充**：Floyd oracle on random directed multigraphs; paths checked; true overflow distinguished from overflowing detours

- **实现来源**：Written for this project; no source code copied
- **参考**：../oi-wiki-reference/docs/ and standard algorithm definitions
- **许可状态**：Project distribution license not yet selected
