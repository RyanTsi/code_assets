# 差分约束配方

资产 ID：`optimization.difference_constraints`

类型：组合配方

问题：`difference_constraints`

[文档目录](../README.md) | [结构化契约](../../contracts/optimization_difference_constraints.json) | [公共头文件](../../include/algorithm_assets/graph/bellman_ford.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/bellman_ford.hpp>
```

`solve_difference_constraints(n,constraints)` 将 v→u,w 解释为 x[u]≤x[v]+w，虚拟源求解，返回 optional<vector<int64>>。无解返回 nullopt；有解给出一组非正可行势，不表示任意目标下的最优解，也不唯一。整体仍 O(n(n+m))，附加输出 8n；同权界和错误行为。与非负 Dijkstra 的替代须核对输入表示、权界与负环状态。

边权要求 `abs(w) <= floor(INT64_MAX/max(n,1))`；虚拟源连接所有变量，用 Bellman-Ford 的负环状态判断可行性。

## 输入与输出

### 输入

- **条件**：v->u,w encodes x[u]<=x[v]+w; Bellman weight bound

### 输出

- **条件**：optional owned int64 feasible potential; nullopt if inconsistent

## 前置条件与约束

- **inequality**：x_to_le_x_from_plus_weight
- **weights**：bounded_signed_int64
- **result_guarantee**：exact

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：v->u,w encodes x[u]<=x[v]+w; Bellman weight bound; optional owned int64 feasible potential; nullopt if inconsistent
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

- **操作成本**：O(n*(n+m))
- **空间**：Bellman workspace + 8n solution
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

约束 x[u]≤x[v]+w 对应弧 v→u。虚拟零权源使所有约束可达；无负环时最短路势满足全部三角不等式，有负环则沿环相加得到矛盾。

## 关联资产

- 依赖使用：[带负环状态的 Bellman-Ford](../algorithms/graph.bellman_ford.md)（`graph.bellman_ford`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：2-SAT&&差分约束.md
- **来源 blob**：3c48077e77ad63ef2a930f7087127cbde4b316c4
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
