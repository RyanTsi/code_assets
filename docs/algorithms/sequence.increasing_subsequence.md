# 最长递增子序列

资产 ID：`sequence.increasing_subsequence`

类型：算法

问题：`increasing_subsequence`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_increasing_subsequence.json) | [公共头文件](../../include/algorithm_assets/sequence/subsequences.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/sequence/subsequences.hpp>
```

入口为 `increasing_subsequence_length(values,nondecreasing=false)` 和 `increasing_subsequence_indices(values,nondecreasing=false)`。

int64；length 与 indices 两接口；nondecreasing=false 严格，true 允许相等；indices 是原序列位置，空返回空；不保证字典序最小见证

O(n log(L+1))；length 只保存 O(L) 个值，见证保存 n 个父指针、O(L) 尾位置及 L 输出索引。每个长度的最小末尾为后续扩展保留最大空间

## 输入与输出

### 输入

- **条件**：Borrowed int64 span; explicit nondecreasing flag

### 输出

- **条件**：size_t length or owned original indices; not lexicographically minimal witness

## 前置条件与约束

- **input_type**：int64
- **comparison**：{"one_of": ["strict", "nondecreasing"]}
- **updates**：none

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Borrowed int64 span; explicit nondecreasing flag; size_t length or owned original indices; not lexicographically minimal witness
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

- **操作成本**：O(n log(L+1))
- **空间**：Length O(L) values; witness O(n+L) indices including output
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每个长度保存尽量小的末尾值，以不减弱任何后续扩展。严格模式用 lower_bound，非降模式用 upper_bound；父指针恢复的是原位置顺序。

## 关联资产

当前契约未登记关联；这不代表不存在潜在组合。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：动态规划/子序列问题/最长上升子序列.md
- **来源 blob**：73ae0e8a50574e86e855a89566cbbe5fed45595d
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
