# 区间并集

资产 ID：`sequence.merge_intervals`

类型：算法

问题：`interval_union`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_merge_intervals.json) | [公共头文件](../../include/algorithm_assets/sequence/intervals.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/sequence/intervals.hpp>
```

入口为 `merge_intervals(intervals,merge_touching=true)`，接受 vector 按值或 `span<const Interval>`，返回拥有的区间数组。

int64 半开区间；first>last 拒绝，空段丢弃；merge_touching 决定端点相接是否并合；按值允许转移，span 保持输入

O(n log n)，排序载荷16n，原数组上压缩，稀疏输出压缩容量；输出拥有有序互不相交区间，原分段信息丢失

## 输入与输出

### 输入

- **条件**：Valid int64 intervals, value or borrowed span; empty intervals discarded

### 输出

- **条件**：Owned sorted disjoint union; segmentation information lost

## 前置条件与约束

- **coordinates**：int64
- **interval_semantics**：half_open
- **updates**：none
- **merge_touching**：{"one_of": [true, false]}

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Valid int64 intervals, value or borrowed span; empty intervals discarded; Owned sorted disjoint union; segmentation information lost
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
- **空间**：16n sorted input payload, in-place compaction, sparse output capacity compaction
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

按左端点排序后，已处理且尚未输出的最后一段包含当前连通并集。下一段与其相接/相交时扩右界，否则前段不可能再与后续段合并，可以输出。

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
- **来源路径**：其他/区间合并.md
- **来源 blob**：c18b1354fec0f95855da2f5743897dd28d597b15
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
