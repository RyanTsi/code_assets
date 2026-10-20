# XorBasis64

资产 ID：`algebra.xor_basis64`

类型：算法

问题：`gf2_span`

[文档目录](../README.md) | [结构化契约](../../contracts/algebra_xor_basis64.json) | [公共头文件](../../include/algorithm_assets/algebra/xor_basis.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/algebra/xor_basis.hpp>
```

`algebra/xor_basis.hpp`：完整 uint64 位域的 GF(2) 向量空间。`insert(x)` 返回是否增加秩；`contains(x)`；`rank()`；`maximize(seed=0)` 是仿射集合 seed XOR span 的最大值；`minimum_nonzero()`；`merge(other)`（包括自合并）；`kth_span_value(index)` 返回从 0 开始的去重、有序张成空间值，**包含空子集产生的 0**。秩 r<64 时 index≥2^r 返回 nullopt；r=64 时所有 uint64 索引合法，不计算越界的 1<<64。全零/空输入只有值 0，minimum_nonzero 返回 nullopt。

固定 64 个 uint64 加秩，本机 sizeof=520；无堆分配。insert/contains/max/min O(64)，merge 和 kth 最坏 O(64²)，kth 额外 512 字节局部行副本。没有输入集合副本、重复次数、组合见证、删除、按非空子集计数或历史回滚。uint64 无精度误差；kth 的 index 本身不能表达 2^64。

不变量：每个非零行有唯一最高位，插入消掉高位后建立新主元；贪心从高位优先提高 XOR 数值。kth 在局部副本消掉各行的低主元，低到高编号的行组合与递增值一一对应。原模板的有符号移位、低 62 位和 kth 零值语义在此重新定义。

## 输入与输出

### 输入

- **vectors**：uint64 GF(2), all 64 bits
- **kth_index**：uint64, zero-based

### 输出

- **rank**：0..64
- **maximize**：seed XOR span maximum
- **kth**：Distinct sorted span including zero, optional if index out of range
- **minimum**：optional nonzero value

## 前置条件与约束

- **input_type**：uint64
- **updates**：{"subset_of": ["insert", "merge", "query"]}
- **enumeration**：distinct_including_zero

- **最大 n**：9223372036854775807
- **分配**：size_t and vector max_size / available memory limits additionally apply
- **异常与失败行为**：Bad index/range or endpoint throws; overflow explicitly checked; allocations may throw
- **enumeration**：No multiplicity, nonempty-subset interpretation, witness or deletion; full rank permits all uint64 indices

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

- **insert_query**：O(64)
- **merge_kth**：O(64^2)
- **空间**：512-byte rows + size_t rank; kth local 512 bytes; no heap

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每个非零基行占据唯一最高位，消元保持张成空间；最高位优先的贪心给出最大 XOR。局部简化行后从低到高的组合编号对应递增空间值，0来自空子集。这是正确性论证，尚未进行形式化验证。

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
- **参考**：数学/线性代数/线性基.md
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源链接**：[来源](https://github.com/RyanTsi/Algorithm-notes/blob/91c14556641c991c716b71906815abdd6a476188/数学/线性代数/线性基.md)
- **additional_reference**：RyanTsi/blog@6ea0ef09d8479c8775654c1627710af6da1d5392:content/posts/线性基.md
