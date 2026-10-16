# 带负环状态的 Bellman-Ford

资产 ID：`graph.bellman_ford`

类型：算法

问题：`shortest_paths`

[文档目录](../README.md) | [结构化契约](../../contracts/graph_bellman_ford.json) | [公共头文件](../../include/algorithm_assets/graph/bellman_ford.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/bellman_ford.hpp>
```

`graph/bellman_ford.hpp`。边 `{uint32 from,to;int64 weight}`；`bellman_ford(n,edges,optional<uint32>source)`。source=nullopt 是向所有点连零权边的虚拟源。返回每点 `{value,status}`，status 为 unreachable、finite、negative_infinity；只有 finite 的 value 有语义。可达负环影响后继，不把其它点误标为负无穷；没有路径见证。空图仅允许虚拟源。n≤UINT32_MAX-1；每条权重必须在 ±floor(INT64_MAX/max(n,1)) 内，超界明确拒绝。

同步双缓冲第 k 轮只使用最多 k 条边的距离，n 边权界确保包含负环检测轮的加法精确。第 n 轮改善的点及其可达后继是负无穷。最坏 O(nm)，每轮复制 O(n)，因此完整界 O(n(n+m))；提前稳定退出。两份 SignedDistance[n]（本机共 32n），负环标记 O(n) 位；传播重复边扫描避免第二份邻接存储。输入不变。

## 输入与输出

### 输入

- **条件**：Directed signed edges; |w| <= INT64_MAX/max(n,1); optional source

### 输出

- **条件**：Owned finite/unreachable/negative_infinity labels; value meaningful only if finite

## 前置条件与约束

- **weights**：bounded_signed_int64
- **graph_updates**：none
- **sources**：{"one_of": ["single", "virtual_all"]}
- **result_guarantee**：exact

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Directed signed edges; |w| <= INT64_MAX/max(n,1); optional source; Owned finite/unreachable/negative_infinity labels; value meaningful only if finite
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

- **操作成本**：O(n*(n+m)) worst including copies; early termination
- **空间**：Two SignedDistance[n] arrays (32n here) plus O(n) affected bits
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

第k轮同步双缓冲仅使用最多k条边的路径。第n轮仍可改善的点受可达负环影响，其可达后继同为负无穷；其他有限和不可达点分别保留状态。声明边权界使检测轮的加法可表示。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[WideInt64Sum](../representations/numeric.wide_int64_sum.md)（`numeric.wide_int64_sum`）。
- 可替代：[Dijkstra with indexed heap](graph.dijkstra.md)（`graph.dijkstra`）。条件：Nonnegative overlap only; convert representation and check narrower Bellman numeric bound

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：图论/最短路/Bellman-Ford.md
- **来源 blob**：02086c31ad78f0efec3825da0dd5d55c5aaa6ad2
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
