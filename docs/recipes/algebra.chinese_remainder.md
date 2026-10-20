# 广义中国剩余定理配方

资产 ID：`algebra.chinese_remainder`

类型：组合配方

问题：`chinese_remainder`

[文档目录](../README.md) | [结构化契约](../../contracts/algebra_chinese_remainder.json) | [公共头文件](../../include/algorithm_assets/algebra/modular.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/algebra/modular.hpp>
```

同头 `chinese_remainder(span<Congruence{uint64 residue,modulus}>)` 支持不互质模数；modulus≥1，余数归约，空系统返回 `{0,1}`。不一致返回 nullopt，最终 lcm 超 uint64 抛 overflow_error；返回 `[0,lcm)` 唯一最小非负代表。结合 gcd 与模逆元，无大整数或浮点。每方程 O(log modulus)乘法/除法迭代，O(1)空间，结果不能支持任意长的大模数。

## 输入与输出

### 输入

- **条件**：Congruences, moduli>=1, residues normalized; noncoprime accepted

### 输出

- **条件**：optional least nonnegative residue and lcm; inconsistency nullopt, lcm overflow throws

## 前置条件与约束

- **input_type**：uint64
- **coprime_required**：false
- **result_guarantee**：exact

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Congruences, moduli>=1, residues normalized; noncoprime accepted; optional least nonnegative residue and lcm; inconsistency nullopt, lcm overflow throws
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

- **操作成本**：Per equation O(log modulus) Euclid/multiply iterations, platform multiply cost applies
- **空间**：O(1) temporary state, input borrowed
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

逐项合并 x≡a(mod m) 与 x≡b(mod n)：差值必须被 gcd(m,n) 整除，约去公因子后由模逆元解增量。解的周期为 lcm；不一致与表示溢出分别处理。

## 关联资产

- 依赖使用：[全宽模运算](../algorithms/algebra.modulus64.md)（`algebra.modulus64`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：数学/数论/中国剩余定理.md
- **来源 blob**：297eb31d84ac61cf6a73b1138492bd602da46819
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
