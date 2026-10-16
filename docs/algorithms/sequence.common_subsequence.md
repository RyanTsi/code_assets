# 最长公共子序列长度

资产 ID：`sequence.common_subsequence`

类型：算法

问题：`common_subsequence`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_common_subsequence.json) | [公共头文件](../../include/algorithm_assets/sequence/subsequences.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/sequence/subsequences.hpp>
```

入口为 `common_subsequence_length(std::string_view a,std::string_view b)`，返回 size_t 长度。

string_view 字节序列，允许 NUL/空串；只返回 size_t 长度，不提供编辑脚本或 traceback

O(nm)，一行 8(min(n,m)+1)，不再存 n×m 表；对角旧值加上当前行/上一行维持 DP 不变量

## 输入与输出

### 输入

- **条件**：Borrowed arbitrary-byte string_views, empty allowed

### 输出

- **条件**：Exact size_t LCS length; no traceback

## 前置条件与约束

- **text_unit**：byte
- **output**：length
- **updates**：none

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Borrowed arbitrary-byte string_views, empty allowed; Exact size_t LCS length; no traceback
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

- **操作成本**：O(n*m)
- **空间**：8(min(n,m)+1) one row
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

DP(i,j) 在末字节相等时取对角旧值+1，否则取上方和左方最大值。一行更新保留旧对角值，等价于完整二维 DP 的状态转移。

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
- **来源路径**：动态规划/子序列问题/最长公共子序列.md
- **来源 blob**：f367c8ff154a69a171ae67dc1513f5d6557893c3
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
