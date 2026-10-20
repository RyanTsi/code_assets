# Offline dynamic connectivity

资产 ID：`connectivity.offline`

类型：组合配方

问题：`connectivity`

[文档目录](../README.md) | [结构化契约](../../contracts/offline_connectivity.json) | [公共头文件](../../include/algorithm_assets/recipes/offline_connectivity.hpp) | [可编译示例](../../examples/first_algorithms.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/recipes/offline_connectivity.hpp>
```

头文件：`recipes/offline_connectivity.hpp`。传入顶点数和完整 `ConnectivityEvent` 列表，事件为 add/remove/query，返回按 query 出现顺序排列的 `vector<bool>`。

初始图为空、无向；边两端顺序无关，支持自环和多次添加。同一边按引用计数处理，remove 只减少一份；最后一份删除前边仍有效。删除不存在的边或未知事件类型抛 invalid_argument；坏顶点抛 out_of_range。合法空事件列表返回空结果。

步骤：先将每条边转换为 `[添加时刻,最后一份删除时刻)` 的生存区间，再放入时间树节点。DFS 进入节点时合并其覆盖期间有效的边，叶子回答当前时刻查询，离开时恢复 checkpoint。根到叶的边恰好构成该时刻有效图，因此答案等于该图的连通性。

时间树采用紧凑索引与 CSR 风格的事件数组，避免每个节点单独分配 vector。它的职责是分发时间区间，并不等同于普通 SegmentTree 的幺半群聚合，因此没有强行复用那个类。当前只把这段结构留在配方内；出现第二个实际使用者再考虑抽出组件。

q 为事件数，K 为边生存区间数：预处理 O(q*log(q+1)+K*log(q+1))，处理 O(n+q+K*log(q+1)*log(n+1))，空间 O(n+q+K*log(q+1))。map 节点、构建游标、中间区间、输出和分配峰值需另外细化；回滚日志任一时刻最多 n-1 次有效合并。

全部事件必须事先已知，不能回答实时在线删边问题。内部回滚不等于对调用者暴露 undo/redo；它也不提供持久化版本。多个独立调用可并行，返回的 vector<bool> 不可随意并发写不同元素。

## 输入与输出

### 输入

- **initial_graph**：Empty undirected multigraph
- **events**：Entire chronological event sequence available before computation; valid endpoints
- **copies**：Repeated adds increase multiplicity; remove consumes one copy; edge live until last copy removed

### 输出

- **answers**：vector<bool> in query occurrence order; query(v,v) true; no queries => empty
- **input**：Not modified

## 前置条件与约束

- **directed**：false
- **execution_mode**：offline
- **updates**：{"subset_of": ["add", "remove", "query"]}
- **回滚**：false

- **最大 n**：2147483647
- **异常与失败行为**：Remove of inactive edge, unknown event kind invalid_argument; invalid ID out_of_range; storage overflow length_error; allocation may throw
- **deletion_scope**：Supports remove events only in this offline batch; no online delete API
- **undo_scope**：Internal rollback not exposed as user rollback/redo
- **并发条件**：Independent calls only; returned vector<bool> concurrent writes unsafe

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：支持
- **并发读取**：支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **preprocess**：O(q*log(q+1) + K*log(q+1)) where K is live intervals
- **traversal**：O(n+q+K*log(q+1)*log(n+1))
- **空间**：O(n+q+K*log(q+1)); input, allocator/map overhead and output also matter
- **保证范围**：deterministic ordered-map preprocessing

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每条有效边的生存区间被分解到时间树节点。根到查询叶子的节点所登记的边恰好组成该时刻的有效图；进入节点合并，离开恢复祖先 checkpoint，使相邻时间分支互不污染。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[RollbackDisjointSet](../algorithms/connectivity.rollback_dsu.md)（`connectivity.rollback_dsu`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formally verified
- **测试**：[tests/algorithms_test.cpp](../../tests/algorithms_test.cpp)
- **示例**：[examples/first_algorithms.cpp](../../examples/first_algorithms.cpp)
- **基准**：not_measured
- **补充**：BFS oracle at each query across valid randomized multigraph event streams; repeated edges and invalid removal

- **实现来源**：Written for this project; no source code copied
- **参考**：../oi-wiki-reference/docs/ and standard algorithm definitions
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
