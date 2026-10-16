# WideInt64Sum

资产 ID：`numeric.wide_int64_sum`

类型：数据表示

问题：`wide_integer_accumulation`

[文档目录](../README.md) | [结构化契约](../../contracts/numeric_wide_int64_sum.json) | [公共头文件](../../include/algorithm_assets/numeric/wide_int64_sum.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/numeric/wide_int64_sum.hpp>
```

`numeric/wide_int64_sum.hpp`：固定 16 字节、两字的有符号 128 位精确累加状态；`add(int64)`、`subtract(int64)`（含 INT64_MIN）、`value()`、`clear()`；初始化为 0。单操作 O(1)，无堆分配。累加范围为 [-2^127,2^127-1]，越界抛 overflow_error 且状态不变；value 只在最终值适配 int64 时返回，超界不改变状态。非一般大整数，不输出 128 位格式；不支持持久化/历史记录。add/subtract 可互逆的前提是两步均不超累加范围，与提供历史 undo/redo 不同。

正确性：低字使用无符号模 2^64 运算，高字加符号扩展和进位/借位；转换前同时检查高字与低字符号，避免有符号溢出。为 Fenwick 与森林权重解决“中间超 int64、最终可表示”的抵消问题。

## 输入与输出

### 输入

- **数值**：add/subtract int64 including INT64_MIN; initially zero

### 输出

- **value**：Exact final int64 or overflow_error
- **clear**：Reset zero

## 前置条件与约束

- **input_type**：int64
- **result_type**：int64
- **intermediate_type**：signed128

- **最大 n**：9223372036854775807
- **分配**：size_t and vector max_size / available memory limits additionally apply
- **异常与失败行为**：Bad index/range or endpoint throws; overflow explicitly checked; allocations may throw
- **数值限制**：Accumulator signed128; final narrowing int64 checked; failed operations leave state unchanged

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

- **构建**：O(1)
- **操作成本**：O(1)
- **空间**：16 bytes; no heap

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

低字进行无符号模2⁶⁴运算，高字累计符号扩展与进位/借位。对高低字的联合符号检查检测128位范围溢出；value 另检验最终结果是否可表示为 int64。这是正确性论证，尚未进行形式化验证。

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
- **参考**：standard two-word arithmetic
- **许可状态**：No source LICENSE discovered; project distribution license not selected
