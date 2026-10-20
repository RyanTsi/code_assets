# 完全背包

资产 ID：`optimization.knapsack_unbounded`

类型：算法

问题：`knapsack`

[文档目录](../README.md) | [结构化契约](../../contracts/optimization_knapsack_unbounded.json) | [公共头文件](../../include/algorithm_assets/optimization/knapsack.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/optimization/knapsack.hpp>
```

`optimization/knapsack.hpp` 输入 `{size_t cost,int64 value}`；bounded 额外 size_t count。本函数返回在 cost 总和**不超过 capacity**下的最大 int64 值，允许不取，负/零价值可忽略，不提供选取见证、恰好装满、多维约束或回滚；空物品返回0。容量表 C+1 必须可分配；真实可行值超 int64 抛 overflow_error，调用者输入不变。

`knapsack_unbounded`：预算从小到大更新；正价值零 cost 是无界目标，抛 domain_error；零/负价值零 cost 忽略。O(n(C+1))、8(C+1)。

## 输入与输出

### 输入

- **条件**：Same items; positive zero-cost item explicitly reports unbounded goal via domain_error

### 输出

- **条件**：Exact int64 optimum, no witness

## 前置条件与约束

- **multiplicity**：unbounded
- **objective**：maximum_value_at_most_capacity
- **input_type**：int64
- **output**：value

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Same items; positive zero-cost item explicitly reports unbounded goal via domain_error; Exact int64 optimum, no witness
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

- **操作成本**：O(n*(capacity+1))
- **空间**：8*(capacity+1) DP
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

容量正序使较小容量已包含当前物品的可重复使用；正价值零成本会使目标无界，因此必须显式拒绝。

## 关联资产

- 依赖使用：[WideInt64Sum](../representations/numeric.wide_int64_sum.md)（`numeric.wide_int64_sum`）。条件：Calls checked_int64_add helper; does not use the wide accumulator object

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：动态规划/背包问题/完全背包.md
- **来源 blob**：6f7e66f6a7527072642d026d507a50b26980e11f
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
