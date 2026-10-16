# KmpPattern

资产 ID：`strings.kmp`

类型：算法

问题：`substring_search`

[文档目录](../README.md) | [结构化契约](../../contracts/strings_kmp.json) | [公共头文件](../../include/algorithm_assets/strings/kmp.hpp) | [可编译示例](../../examples/repository_assets.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/strings/kmp.hpp>
```

`strings/kmp.hpp`：构造时拥有 std::string 模式并生成前缀表；`find_first(text)` 返回 optional<size_t>；`contains(text)`；`for_each_match(text,callback)`；`find_all(text)` 返回所有偏移；`pattern()`、`prefix_function()` 返回借用视图，随对象存储有效。text 在调用期间借用且不能修改。callback 可抛出，模式状态不变，不得在 callback 中修改模式对象或 text。

精确比较字节，包括内嵌 NUL；偏移也是字节，未实现 Unicode 码点、规范化、大小写折叠或编辑距离。重叠匹配保留；空模式匹配 `[0,n]` 的每个边界，find_first 返回 0。不存在用 nullopt 表示。模式长度 m 的构建 O(m)，搜索 O(n+k) 加 callback 成本；常驻 m 个字符加 m 个 size_t（本机约 9m 载荷），扫描 O(1) 临时空间，find_all 额外 k 个 size_t 及容量冗余。不支持跨块流式状态。

不变量：matched 是已扫描文本后缀与模式前缀的最长匹配长度；失败沿最长 border 下降，文本位置从不回退，下降总次数受上升总次数约束。原笔记只返回首次命中且未处理空模式；资产定义了全部命中及空模式语义。

## 输入与输出

### 输入

- **pattern**：Owned string; borrowed const prefix/pattern views
- **text**：Borrowed string_view incl embedded NUL

### 输出

- **matches**：0-based offsets; overlapping; empty pattern has n+1 boundaries
- **first**：optional<size_t>
- **all**：Owned vector or callback

## 前置条件与约束

- **text_unit**：byte
- **matching**：exact
- **pattern_updates**：none
- **execution_mode**：whole_text

- **最大 n**：9223372036854775807
- **分配**：size_t and vector max_size / available memory limits additionally apply
- **异常与失败行为**：Bad index/range or endpoint throws; overflow explicitly checked; allocations may throw
- **lifetime**：Views tied to pattern storage; callback may throw, cannot modify pattern/text; no chunk streaming or Unicode semantics

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

- **构建**：O(m)
- **search**：O(n+k) plus callback; first can stop early
- **空间**：m chars + m size_t; O(1) scan; find_all adds k size_t and capacity

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

matched 始终为已扫描文本后缀与模式前缀的最长匹配长度。失配沿最长 border 回退，文本位置不回退；回退总数受此前上升次数约束，重叠匹配仍可保留。这是正确性论证，尚未进行形式化验证。

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
- **参考**：字符串/KMP.md
- **许可状态**：No source LICENSE discovered; project distribution license not selected
- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源链接**：[来源](https://github.com/RyanTsi/Algorithm-notes/blob/91c14556641c991c716b71906815abdd6a476188/字符串/KMP.md)
