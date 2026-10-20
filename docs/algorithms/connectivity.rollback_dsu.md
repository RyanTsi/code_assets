# RollbackDisjointSet

资产 ID：`connectivity.rollback_dsu`

类型：算法

问题：`connectivity`

[文档目录](../README.md) | [结构化契约](../../contracts/rollback_dsu.json) | [公共头文件](../../include/algorithm_assets/connectivity/rollback_disjoint_set.hpp) | [可编译示例](../../examples/first_algorithms.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/connectivity/rollback_disjoint_set.hpp>
```

头文件：`connectivity/rollback_disjoint_set.hpp`。API 与普通并查集相近，但查询为 const；另有 `checkpoint()`、`rollback(token)` 和 `undo_last_merge()`。

不做路径压缩，仅按大小合并，使一次有效合并只修改两个根。每次记录父根、子根和父根原来的负大小；撤销时由合并后的负大小减去父根原大小恢复子根大小。按后进先出顺序撤销可恢复之前的森林和集合数。

只有成功合并写日志。`undo_last_merge()` 撤销最近一次成功合并，而不是最近一次 API 调用；没有历史时返回 false。`rollback` 撤销至指定日志深度，未来深度抛异常。

**token 只适用于同一实例的当前祖先历史**，不是持久化版本 ID。回滚后丢弃的未来 token、其他对象的 token 即使数值恰好相同，也不能使用；这一误用无法由深度值检测。不要将 token 跨对象复制、分支或任意调用序列传递。读者应采用嵌套的进入/退出模式。

查根 O(log(n+1))；有效合并还包含 vector 日志追加的均摊 O(1)，一次扩容可能 O(h)。撤销 k 次合并需 Theta(k)，单次 undo 为 O(1)。当前有效日志 h <= n-1；逻辑载荷为 4n+12h 字节，capacity 与扩容峰值另计。仅无写入时允许并发只读查询。

可用于离线连通性配方；不提供任意删除、redo 或历史分支查询。

## 输入与输出

### 输入

- **universe**：Fixed n vertices; IDs [0,n)
- **检查点**：History depth in this same instance on current ancestral history

### 输出

- **merge**：Only successful merges append history
- **撤销**：undo_last_merge undoes one successful merge, not necessarily the last API call
- **回滚**：Restores partition and count to checkpoint

## 前置条件与约束

- **directed**：false
- **execution_mode**：{"one_of": ["online", "offline"]}
- **updates**：{"subset_of": ["add", "query"]}
- **回滚**：true

- **最大 n**：2147483647
- **异常与失败行为**：Invalid vertex/future depth throws; obsolete or foreign same-depth tokens cannot be detected and violate preconditions
- **mutation**：Read-only queries; merges and rollback mutate
- **历史状态**：Rolling back discards the future; no redo, branch access, or arbitrary deletion
- **并发条件**：Concurrent reads only when no mutation occurs
- **copy**：Deep value copy O(n+h); checkpoints do not transfer between objects

## 状态与生命周期

- **持久化**：不支持
- **检查点**：支持
- **回滚**：支持
- **撤销**：支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：有条件支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **initialization**：Theta(n)
- **query_or_merge**：O(log(n+1)); history vector append amortized O(1), individual reallocation O(h)
- **回滚**：Theta(k) for k undone successful merges; each undo O(1)
- **空间**：4n + 12h logical payload bytes; h <= n-1; capacities/expansion extra
- **保证范围**：logarithmic traversal; amortized append

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

不压缩路径，使有效合并仅修改两个根并追加固定日志。逆序恢复这些根及集合计数可逐步恢复祖先森林；token 的正确性依赖同实例当前祖先历史，而非数值相同。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 可替代：[DisjointSet](connectivity.dsu.md)（`connectivity.dsu`）。
- 参与组合：[Offline dynamic connectivity](../recipes/connectivity.offline.md)（`connectivity.offline`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formally verified
- **测试**：[tests/algorithms_test.cpp](../../tests/algorithms_test.cpp)
- **示例**：[examples/first_algorithms.cpp](../../examples/first_algorithms.cpp)
- **基准**：not_measured
- **补充**：Random nested rollback and undo versus full reference partitions; obsolete-token restrictions are documented

- **实现来源**：Written for this project; no source code copied
- **参考**：../oi-wiki-reference/docs/ and standard algorithm definitions
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
