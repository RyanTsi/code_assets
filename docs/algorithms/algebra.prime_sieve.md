# 奇数位压缩素数筛

资产 ID：`algebra.prime_sieve`

类型：算法

问题：`primality_table`

[文档目录](../README.md) | [结构化契约](../../contracts/algebra_prime_sieve.json) | [公共头文件](../../include/algorithm_assets/algebra/prime_sieve.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/algebra/prime_sieve.hpp>
```

`algebra/prime_sieve.hpp`：`PrimeSieve(uint32 limit)` 构建奇数位压缩 Eratosthenes；`is_prime(x)` 仅 x≤limit，越界拒绝；`for_each_prime(callback)` 和 `primes()` 递增枚举，0/1非素数，2单独处理。构建 O(N log log N)，奇数标记约 N/16字节，无常驻质数表；primes 输出另有4π(N)及容量冗余，callback 成本另计。它比原线性筛降低标记存储，未提供 SPF/分解/欧拉函数，也未称时间优于所有筛法。

## 输入与输出

### 输入

- **条件**：uint32 inclusive limit N, queries within limit

### 输出

- **条件**：Exact primality or owned sorted primes or callback; 0/1 false

## 前置条件与约束

- **input_type**：uint32
- **output**：primality_or_primes
- **updates**：none

- **最大 n**：4294967295
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：uint32 inclusive limit N, queries within limit; Exact primality or owned sorted primes or callback; 0/1 false
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

- **操作成本**：O(N log log N) construction; O(1) query; O(N) enumeration plus callback
- **空间**：Approximately N/16 marking bytes; owned prime output adds4*pi(N)
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每个合数都有不超过其平方根的素因子。从 p² 开始标记奇数倍不会遗漏合数；此前倍数已被更小素因子处理，2独立判断。

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
- **来源路径**：数学/数论/质数筛.md
- **来源 blob**：79841144659dc38879cd0a8fda740f6834d5e3f8
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
