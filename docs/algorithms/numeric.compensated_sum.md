# Neumaier compensated sum

资产 ID：`numeric.compensated_sum`

类型：算法

问题：`sum`

[文档目录](../README.md) | [结构化契约](../../contracts/compensated_sum.json) | [公共头文件](../../include/algorithm_assets/numeric/compensated_sum.hpp) | [可编译示例](../../examples/first_algorithms.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/numeric/compensated_sum.hpp>
```

头文件：`numeric/compensated_sum.hpp`。流式 `CompensatedSum::add/value/reset`，或直接 `compensated_sum(span<const double>)`。这里实现 Neumaier 补偿求和。

当普通加法遗漏了低位贡献，通过误差项累计补偿，再加到总和上。根据当前和与新值的绝对值大小选择误差计算顺序，处理新值较大的情况；例如 `[1e16,1,-1e16]` 普通逐项求和得到 0，本实现得到 1。

要求有限 double 输入、通常的 IEEE 浮点计算及默认舍入环境，禁止 fast-math 或会重结合表达式的优化。NaN/无穷抛 invalid_argument；工作和、补偿或最终值溢出抛 overflow_error，失败的 add 不改变累加器。即使最终真实和有限，中间溢出也可能导致拒绝。

输出仍是近似浮点数，不能保证任意数据上精确或正确舍入，没有无条件固定误差上限。极端消去、下溢及不同浮点环境仍需领域验证。空输入为零。

每次操作 O(1)，数组求和 Theta(n)，状态仅两个 double。对象复制 O(1) 可手动保存状态，但没有 checkpoint、undo 或 redo API。仅无 add/reset 时允许并发 value 读取。

## 输入与输出

### 输入

- **数值**：Finite doubles; streaming add or span convenience function
- **execution**：Ordinary IEEE floating evaluation; no fast-math/reassociation; intended default rounding

### 输出

- **value**：Compensated double approximation, not exact arithmetic or universally correctly rounded
- **empty**：Zero; reset returns to zero

## 前置条件与约束

- **finite_inputs**：true
- **reassociation**：false
- **intermediates_finite**：true
- **result_guarantee**：floating_approximate

- **最大 n**：18446744073709551615
- **异常与失败行为**：Nonfinite inputs invalid_argument; working sum/correction/result overflow_error; failed add preserves state
- **精度**：No universal fixed error tolerance; cancellation and subnormal handling depend on environment
- **快照**：Value copy is O(1), but no checkpoint/undo/redo API
- **并发条件**：Concurrent value reads only without add/reset

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

- **add_value_reset**：O(1)
- **span_sum**：Theta(n)
- **空间**：Two doubles; O(1) auxiliary space, excluding input
- **保证范围**：approximate numeric result

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

根据部分和与新值的绝对值大小选择舍入残差计算顺序，将遗漏低位累计到补偿项。该机制改善相消实例，但不提供任意输入的精确或正确舍入证书。这是正确性论证，尚未进行形式化验证。

## 关联资产

当前契约未登记关联；这不代表不存在潜在组合。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formally verified
- **测试**：[tests/algorithms_test.cpp](../../tests/algorithms_test.cpp)
- **示例**：[examples/first_algorithms.cpp](../../examples/first_algorithms.cpp)
- **基准**：not_measured
- **补充**：Cancellation examples and bounded integer sums; nonfinite inputs and overflow preserve accumulator

- **实现来源**：Written for this project; no source code copied
- **参考**：../oi-wiki-reference/docs/ and standard algorithm definitions
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
