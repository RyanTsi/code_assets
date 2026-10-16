# 部分主元 LU

资产 ID：`numeric.dense_lu`

类型：算法

问题：`linear_system`

[文档目录](../README.md) | [结构化契约](../../contracts/numeric_dense_lu.json) | [公共头文件](../../include/algorithm_assets/numeric/dense_lu.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/numeric/dense_lu.hpp>
```

入口为 `DenseLu(n,matrix,relative_tolerance)`，随后 `solve(rhs)`、`log_determinant()` 和 `size()`；matrix 支持 span 拷贝或 vector 按值转移。

`numeric/dense_lu.hpp`。输入 n×n 行优先有限 double 矩阵，按值可转移，span 会复制。部分行主元 LU 分解 O(n³)，常驻8n²+4n；`solve(rhs)` 长度 n，返回拥有的 x，O(n²)及8n输出，可重用分解、并发 solve。原矩阵/RHS 不修改。空矩阵可用；`log_determinant()` 返回符号与 log|det|，空矩阵为 `{+1,0}`，避免直接连乘上/下溢；求和复用 CompensatedSum。

relative_tolerance 在 `[0,1)` 且有限，默认64ε；阈值为 tolerance·输入无穷范数。零主元或小于阈值拒绝并抛 domain_error，**表示精确奇异或数值上不宜接受，不是证明矩阵奇异**。矩阵/RHS/中间计算非有限明确拒绝；零阈值仍是浮点近似，未提供条件数、秩、最小二乘、SVD、认证误差界或动态矩阵更新。矩阵行和溢出也拒绝。准确性依赖条件数，强缩放/病态输入宜交给成熟数值库。

分解复用与求解依据见 [LAPACK 线性方程文档](https://www.netlib.org/lapack/lug/node38.html)。本实现是小中规模的可读基线，不替代 BLAS/LAPACK 的块运算与诊断能力。

## 输入与输出

### 输入

- **条件**：Finite row-major n*n matrix, copy or ownership transfer, relative pivot tolerance [0,1)

### 输出

- **条件**：Reusable factorization, owned finite solution, sign/logabs determinant; small pivots rejected

## 前置条件与约束

- **matrix**：dense_square_float64
- **output**：solution
- **result_guarantee**：approximate
- **updates**：none

- **最大 n**：4294967294
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Finite row-major n*n matrix, copy or ownership transfer, relative pivot tolerance [0,1); Reusable factorization, owned finite solution, sign/logabs determinant; small pivots rejected
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

- **操作成本**：O(n^3) factor, O(n^2) solve, O(n) log determinant
- **空间**：8n^2 LU +4n row permutation; solve 8n output
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

带行置换的消元保持 PA=LU；只在可接受主元上消去下方元素。求解依次解 Ly=Pb 和 Ux=y，复用因子避免重新分解；容差拒绝小主元是数值策略，并不认证精确奇异性或解误差。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[Neumaier compensated sum](numeric.compensated_sum.md)（`numeric.compensated_sum`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/blog
- **固定提交**：6ea0ef09d8479c8775654c1627710af6da1d5392
- **来源路径**：content/posts/数值计算-矩阵论.md
- **来源 blob**：d06aaf392720038ed7e34f0c87441c1c2968a08c
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
