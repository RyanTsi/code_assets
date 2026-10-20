# KruskalMinimumSpanningForest

资产 ID：`graph.kruskal`

类型：算法

问题：`minimum_spanning_forest`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_kruskal.json) | [公共头文件](../../include/algorithm_assets/graph/kruskal.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/kruskal.hpp>
```

`graph/kruskal.hpp`：`kruskal_minimum_spanning_forest(n,span<const UndirectedWeightedEdge>)`。边为 `{uint32 first,second; int64 weight}`，顶点 `[0,n)`；n ≤ INT32_MAX，边数受 vector/size_t 和可用内存限制。允许负权、平行边、自环、孤立点；无向、静态，输入不变。返回拥有的原始边索引、int64 total_weight、component_count、is_spanning_tree。索引解释需保留原始边顺序。n=0 返回空森林且不是树；n=1 是树。断连返回覆盖所有分量的最小森林，不用 -1 混淆负权。

最坏 O(n+m log(m+1))，排序 m 个 size_t 索引（本机 8m），并查集 4n，输出至多 min(m,n-1) 个 size_t，排序栈 O(log m)。边记录不复制。借用输入的并发读取允许；不支持在线删边/更新后的增量 MST。非法端点抛 out_of_range；最终权重超 int64 抛 overflow_error，调用者输入不变。中间和使用 WideInt64Sum，以允许抵消。

正确性：每条跨当前分量的最轻边是相应割上的安全边；依次合并维持可扩展为最小森林的不变量。复用已有 DisjointSet。负权与断连行为超出了对应洛谷模板题的验证范围，由本地穷举森林对照补充。

## 输入与输出

### 输入

- **edges**：Borrowed undirected {uint32 first,second;int64 weight}; loops/parallel/negative allowed
- **vertices**：0-based

### 输出

- **edges**：Owned original edge indices
- **weight**：Final exact int64 or overflow_error
- **state**：component_count and is_spanning_tree; n=0 is not a tree

## 前置条件与约束

- **directed**：false
- **weights**：signed_int64
- **graph_updates**：none
- **result_guarantee**：exact

- **最大 n**：2147483647
- **分配**：size_t and vector max_size / available memory limits additionally apply
- **异常与失败行为**：Bad index/range or endpoint throws; overflow explicitly checked; allocations may throw
- **数值限制**：Signed weights; only final forest sum must fit int64; input unchanged

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

- **执行**：Worst O(n+m log(m+1))
- **空间**：4n DSU + sizeof(size_t)*m sorted indices + min(m,n-1) output indices; O(log m) stack

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

按权重顺序加入跨当前分量的边。每次选择都是相应割上的安全边，可将当前森林扩充为某个最小生成森林；并查集避免成环，断连分量分别得到最小树。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[DisjointSet](connectivity.dsu.md)（`connectivity.dsu`）。
- 依赖使用：[WideInt64Sum](../representations/numeric.wide_int64_sum.md)（`numeric.wide_int64_sum`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formal verification
- **测试**：[tests/repository_assets_test.cpp](../../tests/repository_assets_test.cpp)
- **示例**：[examples/repository_assets.cpp](../../examples/repository_assets.cpp)
- **基准**：not_measured
- **判题证据**：[judge/VALIDATION.md](../../judge/VALIDATION.md)

- **实现来源**：Rewritten modern C++20 for this project, not source-code copy
- **参考**：图论/最小生成树/Kruskal.md
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源链接**：[来源](https://github.com/RyanTsi/Algorithm-notes/blob/91c14556641c991c716b71906815abdd6a476188/图论/最小生成树/Kruskal.md)
