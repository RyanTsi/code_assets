# FenwickSum

资产 ID：`range.fenwick_sum`

类型：算法

问题：`range_sum`

[文档目录](../README.md) | [结构化契约](../../contracts/range_fenwick_sum.json) | [公共头文件](../../include/algorithm_assets/range/fenwick_sum.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/range/fenwick_sum.hpp>
```

`range/fenwick_sum.hpp`：`FenwickSum(size)` 建零数组；`FenwickSum(span<const int64_t>)` 复制为树；`add(position,delta)`；`prefix_sum(last)` 和 `range_sum(first,last)`；`size()`、`storage_bytes()`。索引从 0 起，区间半开，空区间返回 0，空数组允许查询 `[0,0)`。不支持插入、删除、范围加、顺序统计或一般非交换聚合。

初始化 O(n)，单点加和查询最坏 O(log(n+1))，树载荷 8n 字节；查询只用固定 16 字节宽累加器。每个存储块的总和必须落在 int64 范围。输入各点是 int64，但更新只检查块和，逻辑点值可能超界；查询的最终结果超界则抛 overflow_error。两次遍历让更新先验证、后写入，失败不改变树。构建不要求中间前缀和都适配 int64；区间差也不会因两端前缀单独超界而误报。越界索引/区间抛 out_of_range。负数允许，故不提供依赖非负频数的 kth 搜索。

不变量：tree[i] 保存 `[i & (i+1),i]`，查询将前缀拆成互不相交的块；构建合并已完成的子块，各子块总访问数小于 n。相对当前通用线段树的 2n 元素，它用 n 个元素，并以“整数加法、单点增量”为更窄的适用条件。

## 输入与输出

### 输入

- **数值**：Copied span<const int64>; n zeros overload
- **range**：0-based half-open
- **delta**：int64

### 输出

- **sum**：Exact int64; empty zero; final overflow throws
- **storage_bytes**：Owned vector capacity bytes

## 前置条件与约束

- **input_type**：int64
- **updates**：{"subset_of": ["add", "query"]}
- **stored_blocks_fit_int64**：true
- **result_guarantee**：exact

- **最大 n**：9223372036854775807
- **分配**：size_t and vector max_size / available memory limits additionally apply
- **异常与失败行为**：Bad index/range or endpoint throws; overflow explicitly checked; allocations may throw
- **数值限制**：Stored block sums fit int64; updates validated before mutation; logical point may overflow; query narrows only final wide sum

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：有条件支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **构建**：O(n)
- **add_query**：Worst-case O(log(n+1))
- **空间**：8n bytes plus headers/capacity; 16-byte local wide sum

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

tree[i] 保存 [i & (i+1),i] 的块和；查询将前缀分解为互不相交块。更新遍历所有且仅有包含该位置的块，先验证后写入；宽累加器避免可抵消的中间值误报。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[WideInt64Sum](../representations/numeric.wide_int64_sum.md)（`numeric.wide_int64_sum`）。
- 可替代：[SegmentTree](range.segment_tree.md)（`range.segment_tree`）。条件：Only integer sum and point increment; generic ordered folds require SegmentTree

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formal verification
- **测试**：[tests/repository_assets_test.cpp](../../tests/repository_assets_test.cpp)
- **示例**：[examples/repository_assets.cpp](../../examples/repository_assets.cpp)
- **基准**：not_measured
- **判题证据**：[judge/VALIDATION.md](../../judge/VALIDATION.md)

- **实现来源**：Rewritten modern C++20 for this project, not source-code copy
- **参考**：数据结构/树状数组.md
- **许可状态**：No source LICENSE discovered; project distribution license not selected
- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源链接**：[来源](https://github.com/RyanTsi/Algorithm-notes/blob/91c14556641c991c716b71906815abdd6a476188/数据结构/树状数组.md)
