# InversionCounter

资产 ID：`sequence.inversions`

类型：算法

问题：`inversion_count`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_inversions.json) | [公共头文件](../../include/algorithm_assets/sequence/inversions.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/sequence/inversions.hpp>
```

`sequence/inversions.hpp`：int64 序列的严格逆序对 `(i<j && a[i]>a[j])`，相等不计。`count(values)` 保持输入；`count_inplace(mutable_span)` 将输入稳定排序；自由函数 `count_inversions` 提供一次性调用。结果 uint64；空/单元素返回 0；n(n-1)/2 必须不超过 UINT64_MAX，事先校验，无溢出静默截断。最大 n 为 6074001000（仍受 size_t、容器容量、内存限制）。无数值近似。

最坏 O(n log(n+1))；count 两份 n 个 int64 工作区（16n），新对象 inplace 仅一份（8n），递归栈 O(log n)。两缓冲交替充当源/目标，避免每层再复制回去。对象复用保留历史最大容量；之前调用 count 后再 inplace 不会自动释放另一缓冲。`storage_bytes()` 显示保留容量，`release()` 释放。分配/尺寸错误发生在输入排序前；不保存原顺序或撤销记录。工作区不可同时调用。

正确性：递归得到两段有序序列；右侧元素小于左侧首元素时，与左段剩余全部元素形成跨段逆序对，加 middle-left；稳定选择左侧等值确保严格语义。

## 输入与输出

### 输入

- **数值**：Borrowed span<const int64>; count_inplace accepts mutable span

### 输出

- **计数**：Exact uint64 strict inversions
- **state**：count preserves input; count_inplace sorts it stably

## 前置条件与约束

- **input_type**：int64
- **comparison**：strict
- **execution_mode**：batch
- **strategy**：{"one_of": ["any", "merge"]}

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
- **并发读取**：不支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **执行**：Worst O(n log(n+1))
- **空间**：16n for count; 8n fresh inplace; retained capacities may exceed current n; O(log n) stack

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

归并前两段已经有序；取右侧严格更小元素时，左侧剩余每个元素都与它构成跨段逆序对。等值优先取左侧，保证相等元素不计入，递归计数覆盖且不重复全部逆序对。这是正确性论证，尚未进行形式化验证。

## 关联资产

当前契约未登记关联；这不代表不存在潜在组合。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formal verification
- **测试**：[tests/repository_assets_test.cpp](../../tests/repository_assets_test.cpp)
- **示例**：[examples/repository_assets.cpp](../../examples/repository_assets.cpp)
- **基准**：not_measured
- **判题证据**：[judge/VALIDATION.md](../../judge/VALIDATION.md)

- **实现来源**：Rewritten modern C++20 for this project, not source-code copy
- **参考**：其他/逆序对.md
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源链接**：[来源](https://github.com/RyanTsi/Algorithm-notes/blob/91c14556641c991c716b71906815abdd6a476188/其他/逆序对.md)
