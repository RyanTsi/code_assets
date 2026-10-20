# Manacher 回文半径

资产 ID：`strings.manacher`

类型：算法

问题：`palindrome_search`

[文档目录](../README.md) | [结构化契约](../../contracts/strings_manacher.json) | [公共头文件](../../include/algorithm_assets/strings/manacher.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/strings/manacher.hpp>
```

入口为 `palindrome_radii(std::string_view text)`，返回 `PalindromeRadii` 的 odd、even 与 longest。

字节字符串，允许任意字节，不插入 `$`/`#` 哨兵；odd[i] 含中心，even[i] 以 i 为右中心；longest `[first,last)`，并列选最早，空 `[0,0)`；n≤SIZE_MAX/2

O(n)，两组 size_t 半径共16n，本机；镜像复制只在已知窗口内，真正向右扩展总数 O(n)

## 输入与输出

### 输入

- **条件**：Arbitrary bytes including NUL; n<=SIZE_MAX/2

### 输出

- **条件**：Owned odd/even radii, earliest longest half-open range; empty [0,0)

## 前置条件与约束

- **text_unit**：byte
- **output**：radii_and_longest
- **updates**：none

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：Arbitrary bytes including NUL; n<=SIZE_MAX/2; Owned odd/even radii, earliest longest half-open range; empty [0,0)
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

- **操作成本**：O(n)
- **空间**：16n radii on 64-bit platform plus constant metadata
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

利用当前最右回文窗口的镜像半径初始化，只在超出已知部分时比较扩展；右边界单调前进，所有额外扩展总计线性，奇偶半径分开无需哨兵。

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
- **来源路径**：字符串/Manacher.md
- **来源 blob**：4677bda36b90121b5b54594b96a11c5ffb305f56
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
