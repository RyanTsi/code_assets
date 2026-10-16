# 全宽模运算

资产 ID：`algebra.modulus64`

类型：算法

问题：`modular_arithmetic`

[文档目录](../README.md) | [结构化契约](../../contracts/algebra_modulus64.json) | [公共头文件](../../include/algorithm_assets/algebra/modular.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/algebra/modular.hpp>
```

`algebra/modular.hpp`：`Modulus64(m)` 要求 m≥2，所有操作参数 uint64，add/subtract/multiply/power 自动按 m 归约，inverse 仅 gcd=1 时返回 optional 逆元，不要求 m 是质数。无堆存储，8字节模数。支持 `__uint128_t` 的编译器乘法用128位中间值，保持标准接口；其它后端用最多64步无溢出加倍法。power O(log exponent)次乘法，inverse O(log m)次 Euclid 迭代，各次含模乘成本；不可称所有平台 O(1) 乘法。负数需调用者定义归约方式。

## 输入与输出

### 输入

- **条件**：uint64 values; modulus>=2; no prime assumption

### 输出

- **条件**：Reduced add/subtract/multiply/power, optional inverse iff gcd=1

## 前置条件与约束

- **input_type**：uint64
- **modulus**：uint64_ge_2
- **result_guarantee**：exact

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：uint64 values; modulus>=2; no prime assumption; Reduced add/subtract/multiply/power, optional inverse iff gcd=1
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

- **操作成本**：Native uint128 multiply O(1), portable fallback <=64 additions; pow log(exponent) multiplications; inverse log(modulus) iterations
- **空间**：8-byte modulus; constant temporary state
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

加减以无符号范围检查避免溢出；模乘使用足够宽中间值或逐位加倍的不变量，二进制幂逐位累计底数幂；扩展欧几里得仅在 gcd=1 时产生逆元。

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
- **来源路径**：数学/数论/欧几里得算法.md
- **来源 blob**：5057c4027d42e76a2a4511904f69491d78457ef2
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
