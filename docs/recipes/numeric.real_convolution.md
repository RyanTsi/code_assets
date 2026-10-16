# FFT 实卷积配方

资产 ID：`numeric.real_convolution`

类型：组合配方

问题：`convolution`

[文档目录](../README.md) | [结构化契约](../../contracts/numeric_real_convolution.json) | [公共头文件](../../include/algorithm_assets/numeric/fft.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/numeric/fft.hpp>
```

`convolve_real(a,b)` 是补零 + 两次 FFT + 逐点乘 + 逆 FFT 的真实配方；有限 double 输入，任一为空返回空，长度 n+m-1、N=bit_ceil(length)，长度/补零溢出拒绝。O(N log N)，两个复数工作区32N、根8N、实输出8length。输入不变，结果为近似实数，不自动四舍五入为“精确整数乘法”，没有整数精确性证书、任意长度 DFT、流式滤波或 GPU 加速。

## 输入与输出

### 输入

- **条件**：Two finite real spans, empty yields empty; length/padding checked

### 输出

- **条件**：Owned n+m-1 real approximate coefficients, no integer-exact promise

## 前置条件与约束

- **input_type**：float64
- **result_guarantee**：approximate
- **mode**：linear

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Two finite real spans, empty yields empty; length/padding checked; Owned n+m-1 real approximate coefficients, no integer-exact promise
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

- **操作成本**：O(N log N), N=bit_ceil(n+m-1)
- **空间**：32N complex workspace +8N roots +8(n+m-1) output
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

补零使 N≥n+m-1，消除循环卷积折返。卷积定理将线性卷积转为两次正变换、频点相乘和一次逆变换，截取 n+m-1 个实系数；数值近似误差不能用四舍五入自动认证。

## 关联资产

- 依赖使用：[Radix-2 FFT](../algorithms/numeric.radix2_fft.md)（`numeric.radix2_fft`）。

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
