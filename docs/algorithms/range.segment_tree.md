# SegmentTree

资产 ID：`range.segment_tree`

类型：算法

问题：`range_fold`

[文档目录](../README.md) | [结构化契约](../../contracts/segment_tree.json) | [公共头文件](../../include/algorithm_assets/range/segment_tree.hpp) | [可编译示例](../../examples/first_algorithms.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/range/segment_tree.hpp>
```

头文件：`range/segment_tree.hpp`。构造 `SegmentTree<T,Combine>(values, identity, combine)`；支持 `get(i)`、`set(i,value)`、`fold(l,r)` 和 `all_fold()`。

Combine 必须是纯、确定、可通过 const 对象调用的结合运算，identity 必须是左右单位元。concept 只检查调用形式与返回 T，不能验证结合律。无需交换律，因此矩阵乘法、字符串拼接等有序运算也可使用。

输入复制到自有存储。范围为 `[l,r)`，空范围返回 identity；空数组合法。坏索引/范围抛异常且不改变数据。2n 个 T 必须可由 vector 分配；整数运算不得溢出。普通浮点加法不能当作精确满足结合律的运算。

使用紧凑 2n 迭代布局。查询分别维护左侧前缀聚合与右侧后缀聚合，最后按顺序合并；被取出的节点覆盖目标叶子的互不相交部分，因此得到正确的有序区间结果。修改叶子后重建祖先。

长度不是二次幂时，物理根的叶子顺序可能不是逻辑顺序，所以 `all_fold` 调用完整区间查询，而不直接返回根。这是空间紧凑与全区间 O(1) 查询之间的明确取舍。

建树 O(n*C)，修改/查询/全区间查询 O(log(n+1)*C)，C 包含 T 的复制与一次 Combine 成本。逻辑存储为 2n 个 T 和额外 identity/Combine；T 内部动态分配也必须计入。字符串拼接的 C 并非常数。

`get` 返回借用的 const 引用，依赖对象存储；set 可能改变它看到的值。无写入且 Combine 自身线程安全时允许并发读取。若 T 赋值或 Combine 在 set 中抛异常，聚合缓存可能只更新了一部分，须重建/丢弃对象后再用。没有区间修改、undo 或持久化 API。

## 输入与输出

### 输入

- **数值**：span<const T>; copied into owned storage
- **algebra**：Pure deterministic associative Combine(const T&,const T&)->T and two-sided identity
- **ranges**：0-based half-open [first,last)

### 输出

- **fold**：Ordered aggregation; empty range returns identity
- **all_fold**：Full ordered aggregation even for non-power-of-two lengths
- **get**：Borrowed const reference to leaf; value may change after set

## 前置条件与约束

- **operation_associative**：true
- **identity_available**：true
- **updates**：{"subset_of": ["set", "query"]}

- **最大 n**：9223372036854775807
- **分配**：2n must fit vector<T>::max_size(); n and storage must fit target size_t
- **异常与失败行为**：Bad range/index throws before mutation; allocation may throw
- **数值限制**：Caller must ensure no integer overflow and actual associativity; ordinary floating addition is not an exact monoid
- **exceptions**：If T assignment or Combine throws during set, rebuild/discard before further use
- **并发条件**：Concurrent reads only without writers; Combine must be thread-safe
- **lifetime**：get references tied to object storage; destruction/assignment/move invalidates assumptions

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

- **构建**：O(n * (copy_cost + combine_cost))
- **get**：O(1)
- **set_fold_all_fold**：O(log(n+1) * (copy_cost + combine_cost))
- **空间**：2n stored T objects + identity and Combine; dynamic T allocations extra
- **保证范围**：worst-case traversal

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

每个缓存节点保持其叶子片段的聚合，修改沿祖先重建。查询维护有序的左前缀和右后缀，最终按顺序合并；结合律和左右单位元保证结果，非交换操作不要求交换律。非2次幂长度不直接把物理根当全区间结果。这是正确性论证，尚未进行形式化验证。

## 关联资产

当前契约未登记关联；这不代表不存在潜在组合。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formally verified
- **测试**：[tests/algorithms_test.cpp](../../tests/algorithms_test.cpp)
- **示例**：[examples/first_algorithms.cpp](../../examples/first_algorithms.cpp)
- **基准**：not_measured
- **补充**：Random sums versus array scans; ordered string concatenation for n=5 and all subranges

- **实现来源**：Written for this project; no source code copied
- **参考**：../oi-wiki-reference/docs/ and standard algorithm definitions
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
