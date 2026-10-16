# InversionsFenwick

资产 ID：`sequence.inversions_fenwick`

类型：组合配方

问题：`inversion_count`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_inversions_fenwick.json) | [公共头文件](../../include/algorithm_assets/recipes/inversions_fenwick.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/recipes/inversions_fenwick.hpp>
```

`recipes/inversions_fenwick.hpp`：`count_inversions_fenwick(span<const int64_t>)` 返回同一严格 uint64 计数。复制、排序、去重得到有序键；从右往左查询小于当前秩的频数，再将当前秩加一。排序压缩保持大小关系，统计树中始终只有当前元素右侧的频次，保证整体正确。

O(n log(n+1))；初始化排序副本为 8n；复用 CoordinateCompression 后常驻键容量为 O(u)，u 个唯一键的 Fenwick 为 8u，排序栈 O(log n)，输入不变。共享逆序对计数尺寸检查，频数 ≤ n，当前允许范围内落在 int64。本配方不会优于所有输入下的归并实现；它验证“有序压缩 + 已有 Fenwick”的真实代码组合。目录存在单算法时优先返回归并；仅请求已验证配方时可得到此组合。关系说明目前不是自动生成组合的证明。

## 输入与输出

### 输入

- **数值**：Borrowed span<const int64>; sort unique keys preserves ordering

### 输出

- **计数**：Exact uint64; input preserved

## 前置条件与约束

- **input_type**：int64
- **comparison**：strict
- **execution_mode**：batch
- **strategy**：{"one_of": ["any", "fenwick_recipe"]}

- **最大 n**：6074001000
- **分配**：size_t and vector max_size / available memory limits additionally apply
- **异常与失败行为**：Bad index/range or endpoint throws; overflow explicitly checked; allocations may throw
- **数值限制**：n(n-1)/2 <= UINT64_MAX checked before sorting; duplicate values are not inversions

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **执行**：O(n log(n+1)) including compression
- **空间**：8n initial sort copy; O(u) compacted keys +8u Fenwick; O(log n) stack

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

压缩保持严格大小关系；从右到左扫描时 Fenwick 只保存右侧出现频次。查询严格小于当前值的秩前缀，恰好得到以当前元素为左端的逆序对数。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[FenwickSum](../algorithms/range.fenwick_sum.md)（`range.fenwick_sum`）。
- 依赖使用：[InversionCounter](../algorithms/sequence.inversions.md)（`sequence.inversions`）。条件：Only shared count-size validation; does not invoke merge counting
- 可替代：[InversionCounter](../algorithms/sequence.inversions.md)（`sequence.inversions`）。条件：Same strict count; different workspace and operation composition
- 依赖使用：[有序坐标压缩](../representations/sequence.coordinate_compression.md)（`sequence.coordinate_compression`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formal verification
- **测试**：[tests/repository_assets_test.cpp](../../tests/repository_assets_test.cpp)
- **示例**：[examples/repository_assets.cpp](../../examples/repository_assets.cpp)
- **基准**：not_measured
- **判题证据**：[judge/VALIDATION.md](../../judge/VALIDATION.md)

- **实现来源**：Rewritten modern C++20 for this project, not source-code copy
- **参考**：其他/逆序对.md
- **许可状态**：No source LICENSE discovered; project distribution license not selected
- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源链接**：[来源](https://github.com/RyanTsi/Algorithm-notes/blob/91c14556641c991c716b71906815abdd6a476188/其他/逆序对.md)
