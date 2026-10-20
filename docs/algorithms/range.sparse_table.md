# 幂等稀疏表

资产 ID：`range.sparse_table`

类型：算法

问题：`range_fold`

[文档目录](../README.md) | [结构化契约](../../contracts/range_sparse_table.json) | [公共头文件](../../include/algorithm_assets/range/sparse_table.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/range/sparse_table.hpp>
```

入口为 `SparseTable<T,Combine>(values,combine)`；`fold(first,last)`、`size()` 与 `stored_values()` 提供查询和存储规模。

T、纯 Combine 与只读数组；要求结合律和幂等性；非空 `[l,r)` fold，不需要幺元；空表可建，空/越界查询拒绝；无修改

O(n log n) build，O(1) 两次载荷读取+一次合并；只存有效块，总元素 Σ(n-2^k+1)，不是每层 n 个填充。重叠块可因幂等性合并，普通 sum 不适用

## 输入与输出

### 输入

- **条件**：Copied T elements, pure Combine; no identity needed

### 输出

- **条件**：Ordered fold for nonempty half-open range; invalid/empty query throws

## 前置条件与约束

- **operation_associative**：true
- **operation_idempotent**：true
- **updates**：{"subset_of": ["query"]}

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Copied T elements, pure Combine; no identity needed; Ordered fold for nonempty half-open range; invalid/empty query throws
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

- **操作成本**：Build O(n log(n+1)) combine/copy; query O(1) combine/copy
- **空间**：sum_k(n-2^k+1) T objects + O(log n) offsets; T dynamic storage extra
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

预计算所有长度2^k的块，查询取两个覆盖目标且可能重叠的块。结合律与幂等性使重叠元素的重复贡献可消去；普通加法不满足此条件。

## 关联资产

- 可替代：[SegmentTree](range.segment_tree.md)（`range.segment_tree`）。条件：Static idempotent folds and nonempty queries; ordinary sums and point updates are excluded

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：数据结构/ST表.md
- **来源 blob**：7783a7889dfc40bf71a3398db005a34def1ec083
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
