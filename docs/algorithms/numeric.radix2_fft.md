# Radix-2 FFT

资产 ID：`numeric.radix2_fft`

类型：算法

问题：`discrete_fourier_transform`

[文档目录](../README.md) | [结构化契约](../../contracts/numeric_radix2_fft.json) | [公共头文件](../../include/algorithm_assets/numeric/fft.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/numeric/fft.hpp>
```

`numeric/fft.hpp`。`Radix2Fft(N)` 必须是非零 2 次幂，构建 O(N)，预计算 N/2 个 complex<double> 根（本机 8N 字节），不存位逆序数组。`transform(span<complex<double>>,inverse=false)` 长度严格等于 N；原地 O(N log N)、O(1) 额外空间。正变换使用 exp(-i2πkn/N)，逆变换除以 N。输入非有限值拒绝且不修改；计算出现非有限结果抛异常，此时数组已修改。数学上的逆变换不等于无损 undo，round-trip 有误差，不能启用破坏 IEEE 行为的 fast-math。

根表只读，可复用 plan，并发 transform 必须使用互不重叠的工作数组。此实现是无外部依赖的可读基线，没有宣称达到 FFTW/MKL 的性能。

## 输入与输出

### 输入

- **条件**：Nonzero power-of-two N, finite complex<double> mutable span of exact plan size

### 输出

- **条件**：In-place Fourier coefficients, inverse normalized; computation overflow modifies array then throws

## 前置条件与约束

- **length_power_of_two**：true
- **input_type**：complex_float64
- **result_guarantee**：approximate
- **normalization**：inverse_divides_n

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Nonzero power-of-two N, finite complex<double> mutable span of exact plan size; In-place Fourier coefficients, inverse normalized; computation overflow modifies array then throws
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
- **并发读取**：有条件支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **操作成本**：Plan O(n); transform O(n log(n+1))
- **空间**：n/2 complex roots, O(1) transform scratch; caller array extra
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

按索引奇偶把 DFT 分成两个 N/2 子变换，单位根旋转后的蝶形合并得到全体系数。原地位逆序和分层蝶形仅重排计算；逆向共轭旋转并除以 N。在浮点实现中此论证不提供统一误差界。

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
- **来源路径**：数学/多项式/FFT.md
- **来源 blob**：8940ee1dbc560809e3d30681a916ddb62363b557
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
