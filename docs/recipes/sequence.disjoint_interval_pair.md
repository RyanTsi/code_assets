# 非重叠区间对最低费用

资产 ID：`sequence.disjoint_interval_pair`

类型：组合配方

问题：`disjoint_interval_pair`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_disjoint_interval_pair.json) | [公共头文件](../../include/algorithm_assets/recipes/disjoint_interval_pair.hpp) | [可编译示例](../../examples/problem_combinations.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/recipes/disjoint_interval_pair.hpp>
```

`PricedInterval { int64_t begin, end, cost; }` 表示 `[begin,end)`。输入为只借用的 `span<const PricedInterval>`，各区间非空，费用非负，`total_duration` 在 `[1,INT64_MAX]`。端点支持完整 int64 范围，但单段长度须能放入 int64。相同区间或相同费用仍是不同的输入项。返回 `optional<DisjointIntervalPair>`，含精确总费用与原始输入编号，`first` 区间在 `second` 之前，任意一个最低费用方案均可；无解为 nullopt，空输入无解。

非法目标、空/反向区间、负费用抛 invalid_argument；长度超过 INT64_MAX 抛 overflow_error。最佳总费用超过 INT64_MAX 才抛 overflow_error：非最佳溢出候选被跳过，因此不会因无关昂贵方案拒绝可表示的最优解。分配异常正常传播。返回不依赖输入生命周期，函数不修改输入。

按 begin 扫描右区间，按 end 向状态中加入已结束的左区间。状态对每个**原始长度**只保留最低费用及编号。处理右区间时，所有且仅有满足 `left.end <= right.begin` 的区间已加入；长度为 `target-right_length` 的最小费用就是该右区间的最优左搭档。每对合法区间在其右区间处被考虑，从而得到全局最优。正长度保证不把当前区间或同起点的未来区间错误加入。

排序和压缩 O(n log(n+1))，扫描含二分查询 O(n log(u+1))，u 为不同长度数；整体 O(n log(n+1))，O(n+u) 额外存储、O(log n) 排序调用栈，输出 O(1)。64-bit 平台两份排序索引 16n 字节，长度键容量至多 16u 字节，最小费用及编号 16u 字节；压缩初始排序副本 8n 字节，缩容时有临时复制。上述为数组载荷，非测量的进程峰值。

这里无需线段树：状态只有单点取最小、单点查询，没有范围聚合。若长度上限较小且稠密，可用直接长度数组省去压缩二分，但需 O(最大长度) 空间；当前 API 面向稀疏大端点。区间合并和最少区间覆盖接口都不保留本题费用与恰好两项的约束，不能直接替代。

CF 的输入是闭区间整数天 `[l,r]`，适配器将其转成 `[l,r+1)`；只因官方 `r<=200000` 才安全执行 `r+1`。通用 API 没有偷偷把任意闭区间按此方式转换。

## 输入与输出

### 输入

- **条件**：Original endpoints; int64 duration <= INT64_MAX; nonnegative cost; positive total duration

### 输出

- **条件**：Optional exact minimum int64 cost and two original indices; optimum overflow throws

## 前置条件与约束

- **intervals**：nonempty_half_open_int64
- **costs**：nonnegative_int64
- **pair_count**：2
- **duration_rule**：exact_sum

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

扫描右区间时，状态包含所有且仅有已结束的左区间，并对每种原始长度保留最低费用。目标减右段长度确定唯一左长度，每个合法区间对都会在其右段被处理时考虑，因此取到全局最优。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[有序坐标压缩](../representations/sequence.coordinate_compression.md)（`sequence.coordinate_compression`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Documented sweep, DP and tree metric invariants; not formal verification
- **测试**：[tests/problem_combinations_test.cpp](../../tests/problem_combinations_test.cpp)
- **示例**：[examples/problem_combinations.cpp](../../examples/problem_combinations.cpp)
- **基准**：not_measured
- **判题证据**：[judge/COMBINATION_VALIDATION.md](../../judge/COMBINATION_VALIDATION.md)

- **来源仓库**：RyanTsi/blog
- **固定提交**：6ea0ef09d8479c8775654c1627710af6da1d5392
- **来源路径**：content/posts/高质量题解（持续更新中）.md
- **来源 blob**：9191f5b79147f82be1d3985aec2c6351ef8ff6a0
- **实现来源**：Independent engineering implementation; source supplies problem entry and motivation
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
