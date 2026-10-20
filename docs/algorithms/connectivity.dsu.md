# DisjointSet

资产 ID：`connectivity.dsu`

类型：算法

问题：`connectivity`

[文档目录](../README.md) | [结构化契约](../../contracts/dsu.json) | [公共头文件](../../include/algorithm_assets/connectivity/disjoint_set.hpp) | [可编译示例](../../examples/first_algorithms.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/connectivity/disjoint_set.hpp>
```

头文件：`connectivity/disjoint_set.hpp`。构造 `DisjointSet(n)`；随后调用 `merge(u,v)`、`same(u,v)`、`leader(v)`、`component_size(v)` 和 `component_count()`。

固定顶点集合，初始每个顶点独立。顶点 ID 为 `[0,n)`，`n <= INT32_MAX`；空集合合法，但没有合法顶点。越界抛 `out_of_range`，过大规模抛 `length_error`，分配也可能失败。

`merge` 返回是否真正合并两个集合。代表元只是实现选择，不能作为跨操作保持不变的业务标识。查询可能修改内部路径，不能同时并发调用。没有删除、checkpoint、undo、redo 或历史版本 API；普通对象复制是 O(n) 全量复制。

根节点用负数保存集合大小，非根保存父节点编号，因此只需要一个 `int32_t` 数组。合并小集合到大集合；路径压缩只改变到根的路径，不改变集合划分。负大小相加仍在声明的整数界内。

初始化 Theta(n)，操作序列均摊 O(alpha(n))，单次最坏 O(log(n+1))。逻辑数组载荷 4n 字节，容器头、capacity 和分配器额外计入；查询使用迭代循环，无递归栈。

需要撤销时选择 `RollbackDisjointSet`。单纯需要在线删边不能套用这两个结构。

## 输入与输出

### 输入

- **universe**：Fixed n vertices, IDs [0,n), n may be zero
- **操作成本**：["leader(v)", "same(u,v)", "component_size(v)", "merge(u,v)"]

### 输出

- **merge**：True iff two components were joined
- **representative**：An implementation representative, not a stable semantic label
- **partition**：Exact; initially each vertex is separate

## 前置条件与约束

- **directed**：false
- **execution_mode**：{"one_of": ["online", "offline"]}
- **updates**：{"subset_of": ["add", "query"]}
- **回滚**：false

- **最大 n**：2147483647
- **异常与失败行为**：out_of_range for invalid IDs; length_error for excessive n; allocation may throw
- **mutation**：All nontrivial queries may compress paths
- **copy**：A full value copy costs O(n); it is not a checkpoint API
- **并发条件**：Use external synchronization; queries also mutate

## 状态与生命周期

- **持久化**：不支持
- **检查点**：不支持
- **回滚**：不支持
- **撤销**：不支持
- **重做**：不支持
- **任意删除**：不支持
- **并发读取**：不支持
- **并发写入**：不支持

支持状态以公开 API 为准；普通对象复制、数学逆运算和真正的历史 undo/redo 需分别判断。并发支持不得脱离上面的所有权与写入条件。

## 时间与空间

- **initialization**：Theta(n)
- **操作成本**：O(alpha(n)) amortized across a sequence; O(log(n+1)) individual worst case
- **空间**：4n logical payload bytes, plus vector capacity/header; O(1) stack
- **保证范围**：amortized

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

父指针形成森林，负根值保存集合大小。按大小合并只改变两个集合的根关系；路径压缩仅缩短同一棵树中的路径，不改变集合划分，故 leader 相等恰好表示连通。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 可替代：[RollbackDisjointSet](connectivity.rollback_dsu.md)（`connectivity.rollback_dsu`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants in documentation; not formally verified
- **测试**：[tests/algorithms_test.cpp](../../tests/algorithms_test.cpp)
- **示例**：[examples/first_algorithms.cpp](../../examples/first_algorithms.cpp)
- **基准**：not_measured
- **补充**：Random label-partition oracle; boundary IDs; size/count; redundant unions

- **实现来源**：Written for this project; no source code copied
- **参考**：../oi-wiki-reference/docs/ and standard algorithm definitions
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
