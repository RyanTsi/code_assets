# TopologicalOrder

资产 ID：`graph.topological_sort`

类型：算法

问题：`topological_order`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_topological_sort.json) | [公共头文件](../../include/algorithm_assets/graph/topological_sort.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/topological_sort.hpp>
```

`graph/topological_sort.hpp`：`topological_sort(const CsrGraph&)` 或 `topological_sort(const UnweightedCsrGraph&)` 返回 `{vertices,acyclic}`。有向、静态，空图合法。重复边和自环允许。输入 CSR 保持不变，可重复调用。acyclic=true 时是全部顶点的一个拓扑序；false 时 vertices 仅为已处理前缀，未提供具体环路径，未处理顶点也不等于全部环顶点。不是字典序最小拓扑序。

O(n+m)，局部入度 size_t[n]，结果同时作为队列 Vertex[n]，本机载荷最多 12n（含输出），无另一份队列，分配失败输入不变。复用 CSR 的权值被忽略，但构造该表示仍要求非负 int64 权；n ≤ UINT32_MAX-1。CSR 自身权值数组对纯拓扑需求冗余，因此此路径不宣称全局空间最优；已有 UnweightedCsrGraph 路径可省去权值数组；该路径的顶点界与无权表示一致。

正确性：入度表示未移除顶点的入边数，零入度顶点可安全输出。处理数不足 n 时剩余子图没有零入度点，有限有向图必含环。原文修改全局入度，资产每次建立局部工作区。

## 输入与输出

### 输入

- **graph**：Immutable weighted or unweighted CSR; weights ignored

### 输出

- **order**：Owned vertex order; empty graph acyclic
- **acyclic**：False returns processed prefix, not exact cycle vertices/witness

## 前置条件与约束

- **directed**：true
- **graph_updates**：none
- **representation**：{"one_of": ["csr_nonnegative_weights", "unweighted_csr"]}
- **cycle_output**：processed_prefix

- **最大 n**：4294967294
- **分配**：size_t and vector max_size / available memory limits additionally apply
- **异常与失败行为**：Bad index/range or endpoint throws; overflow explicitly checked; allocations may throw
- **cycle**：Processed prefix only, no cycle witness; unweighted path omits weight storage

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：有条件支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **执行**：O(n+m)
- **空间**：sizeof(size_t)*n indegrees + up to 4n output/queue, no separate queue; CSR extra

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

局部入度始终等于未移除顶点的入边数，零入度点可安全输出。若未输出全部顶点，剩余有限子图没有零入度点，因此必含环；返回前缀不等于完整环见证。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[CsrGraph](../representations/graph.csr.md)（`graph.csr`）。条件：Weighted CsrGraph instantiation only
- 依赖使用：[无权 CSR](../representations/graph.unweighted_csr.md)（`graph.unweighted_csr`）。条件：UnweightedCsrGraph instantiation only

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formal verification
- **测试**：[tests/repository_assets_test.cpp](../../tests/repository_assets_test.cpp)
- **示例**：[examples/repository_assets.cpp](../../examples/repository_assets.cpp)
- **基准**：not_measured
- **判题证据**：[judge/VALIDATION.md](../../judge/VALIDATION.md)

- **实现来源**：Rewritten modern C++20 for this project, not source-code copy
- **参考**：图论/其他/拓扑排序.md
- **许可状态**：No source LICENSE discovered; project distribution license not selected
- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源链接**：[来源](https://github.com/RyanTsi/Algorithm-notes/blob/91c14556641c991c716b71906815abdd6a476188/图论/其他/拓扑排序.md)
