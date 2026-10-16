# 最少区间覆盖

资产 ID：`sequence.minimum_interval_cover`

类型：算法

问题：`interval_cover`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_minimum_interval_cover.json) | [公共头文件](../../include/algorithm_assets/sequence/intervals.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/sequence/intervals.hpp>
```

入口为 `minimum_interval_cover(span<const Interval> intervals,Interval target)`，返回 `optional<vector<size_t>>` 的原输入索引。

目标半开区间，空目标零段；无覆盖 nullopt；返回最少原始区间索引，需保留原始顺序解释；不处理加权最小费用

O(n log n)，索引排序8n+O(k)输出；每步从能接当前前沿的段中选最远者，交换论证可替换任意最优解第一段

## 输入与输出

### 输入

- **条件**：Valid intervals and target; no arithmetic +/-1 conversion

### 输出

- **条件**：optional owned minimum original index set; empty target needs zero; gap nullopt

## 前置条件与约束

- **coordinates**：int64
- **interval_semantics**：half_open
- **cost**：unit
- **updates**：none

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Valid intervals and target; no arithmetic +/-1 conversion; optional owned minimum original index set; empty target needs zero; gap nullopt
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

- **操作成本**：O(n log(n+1))
- **空间**：8n sorted indices + O(k) output
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每步从可接当前覆盖前沿的区间中取最远右端点。任意最优解的当前第一段均可替换为该段而不增加后续段数；无可推进区间时不存在覆盖。

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
- **来源路径**：其他/最小区间覆盖数.md
- **来源 blob**：44e7c152a370eeb3c52442fda3ff176927faf01f
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
