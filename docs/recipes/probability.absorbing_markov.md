# 吸收马尔科夫链配方

资产 ID：`probability.absorbing_markov`

类型：组合配方

问题：`absorbing_markov`

[文档目录](../README.md) | [结构化契约](../../contracts/probability_absorbing_markov.json) | [公共头文件](../../include/algorithm_assets/recipes/absorbing_markov.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/recipes/absorbing_markov.hpp>
```

`recipes/absorbing_markov.hpp`。`analyze_absorbing_markov(n,transient_count,transition,probability_tolerance,pivot_tolerance)` 要求瞬态在前、吸收态在后，吸收行严格为单位行；概率有限且 `[0,1]`，行和距1不超过 tolerance（默认1e-12，范围[0,1e-6]），不隐式归一化。先在正概率边上检查每个瞬态可达某个吸收态，不符合抛 domain_error；n=0/t=0 允许。不能把一般状态自动标成瞬态，也不接受只有“行和≤1”的次随机矩阵。

返回 t×a 行优先吸收概率和 t 个期望步数。令 A=I-Q，复用同一 DenseLu 解 AX=R 和 Aτ=1，**不显式求逆、不计算 P 的巨大幂**。最坏 O(n²+t³+at²)；常驻/峰值取决于8t² LU、4t排列、8ta输出、O(n)图可达工作区、O(t) RHS/解。容差接受近似概率、浮点病态可能拒绝；不会裁剪结果到[0,1]掩盖误差。未来几何 morphing/MDP 可复用此线性系统路径，但那些完整应用未实现。

## 输入与输出

### 输入

- **条件**：n*n probabilities in [0,1], row sum tolerance, exact absorbing identity rows

### 输出

- **条件**：Owned t*a absorption probabilities and t expected steps; no explicit inverse

## 前置条件与约束

- **matrix**：dense_float64
- **ordering**：transient_then_absorbing
- **result_guarantee**：approximate
- **transients_reach_absorption**：true

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：n*n probabilities in [0,1], row sum tolerance, exact absorbing identity rows; Owned t*a absorption probabilities and t expected steps; no explicit inverse
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

- **操作成本**：O(n^2+t^3+a*t^2)
- **空间**：8t^2 LU +4t permutation +8ta probabilities +8t expectations +O(n+t) scratch
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

规范吸收链的瞬态块Q满足最终离开瞬态集合，因而精确模型中I-Q可逆。吸收概率满足 (I-Q)B=R，期望步数满足 (I-Q)t=1；复用一次LU求解多个右端而不显式求逆。容差接受后的浮点模型仍需数值验证。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[部分主元 LU](../algorithms/numeric.dense_lu.md)（`numeric.dense_lu`）。
- 依赖使用：[Neumaier compensated sum](../algorithms/numeric.compensated_sum.md)（`numeric.compensated_sum`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/blog
- **固定提交**：6ea0ef09d8479c8775654c1627710af6da1d5392
- **来源路径**：content/posts/吸收马尔科夫链.md
- **来源 blob**：a6bb425768bdfaf92b3a2778f3017c4bdf4bb771
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
