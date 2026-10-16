# 有限步差最长子序列

资产 ID：`sequence.bounded_step_subsequence`

类型：组合配方

问题：`bounded_step_subsequence`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_bounded_step_subsequence.json) | [公共头文件](../../include/algorithm_assets/recipes/bounded_step_subsequence.hpp) | [可编译示例](../../examples/problem_combinations.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/recipes/bounded_step_subsequence.hpp>
```

`bounded_step_subsequence_length(span<const int64_t> values, int64_t max_difference)` 返回精确 size_t 长度，不返回实际子序列。值支持完整 int64 范围；差上限限 `[0,INT64_MAX]`，负上限抛 invalid_argument。空输入返回 0，重复值和上限 0 合法。输入只借用且不修改，不排序原序列。分配/容器尺寸限制另外适用。

在处理当前位置之前，线段树每个键存储先前已处理位置中以该值结束的最长长度。合法前驱值范围为 `[value-gap,value+gap]`；在完整 int64 边界处做饱和端点构造而不执行溢出运算，再用 lower_rank 找半开坐标范围。先查询、再写当前值，确保不在同一步重复使用当前元素。转移是 `1 + range_max`，同值状态保留最大值。归纳得到所有前缀的最佳长度。

压缩只保持序，不保持距离。`[0,100,101]` 在 gap=1 时答案 2；按相邻压缩排名认为三者都接近，会错误输出 3。SparseTable 虽可做静态 max，但当前 DP 每一步更新状态，不能直接替代 SegmentTree。

压缩 O(n log(n+1))，扫描 O(n log(u+1))，整体 O(n log(n+1))，O(n+u) 临时存储、O(log n) 排序调用栈。64-bit 平台树 16u 字节，压缩键容量最多 16u 字节；初始化零数组 8u 字节在扫描前释放，压缩排序副本 8n 字节及缩容临时副本另计。上限 gap=0 或覆盖全部输入值有更简单的特殊路径，但当前接口没有声称在所有工作负载下性能最优。

## 输入与输出

### 输入

- **条件**：Borrowed int64 sequence; 0 <= max_difference <= INT64_MAX; original order unchanged

### 输出

- **条件**：Exact size_t maximum length, empty=0; no witness

## 前置条件与约束

- **数值**：int64
- **adjacent_difference**：original_value_absolute
- **execution_mode**：batch
- **output**：length

- **最大 n**：9223372036854775807
- **异常与失败行为**：Invalid domain rejected; documented arithmetic overflow rejected; allocations propagate; see complete contract

## 状态与生命周期

- **持久化**：不支持
- **快照**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **操作成本**：O(n log(n+1)) time; O(n+u) workspace; O(log n) sort stack
- **保证范围**：Worst-case asymptotic bounds, no calibrated latency or peak-memory claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

线段树每个值的状态只含此前位置的最优长度。按原值范围映射查询合法前驱、再更新当前值，归纳保持前缀最优；不能用秩差代替原值差，也不能先写再读复用当前元素。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[有序坐标压缩](../representations/sequence.coordinate_compression.md)（`sequence.coordinate_compression`）。
- 依赖使用：[SegmentTree](../algorithms/range.segment_tree.md)（`range.segment_tree`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Documented sweep, DP and tree metric invariants; not formal verification
- **测试**：[tests/problem_combinations_test.cpp](../../tests/problem_combinations_test.cpp)
- **示例**：[examples/problem_combinations.cpp](../../examples/problem_combinations.cpp)
- **基准**：not_measured
- **判题证据**：[judge/COMBINATION_VALIDATION.md](../../judge/COMBINATION_VALIDATION.md)

- **来源仓库**：RyanTsi/blog
- **固定提交**：6ea0ef09d8479c8775654c1627710af6da1d5392
- **来源路径**：content/posts/dp题单.md
- **来源 blob**：33f521bad812951b788f56eec8f4466f2f9f370d
- **实现来源**：Independent engineering implementation; source supplies problem entry and motivation
- **许可状态**：No repository license discovered; distribution license not selected
