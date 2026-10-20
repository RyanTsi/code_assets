# 有序坐标压缩

资产 ID：`sequence.coordinate_compression`

类型：数据表示

问题：`coordinate_compression`

[文档目录](../README.md) | [结构化契约](../../contracts/sequence_coordinate_compression.json) | [公共头文件](../../include/algorithm_assets/sequence/coordinate_compression.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/sequence/coordinate_compression.hpp>
```

入口为 `CoordinateCompression(values)`；查询 `keys()`、`rank(value)`、`lower_rank(value)`、`value_at(rank)` 与 `size()`。

int64 数组按值转移或 span 拷贝；keys 借用视图，rank 值缺失返回 nullopt，lower_rank 返回插入秩，value_at 越界抛异常；空/重复值合法

O(n log n) 构建，O(log u) 查询；排序初始 8n，常驻 O(u)，过大容量压缩时有临时副本。保持顺序，不保持距离、面积或二维邻接

## 输入与输出

### 输入

- **条件**：span copied or vector moved; duplicates/empty accepted

### 输出

- **条件**：Borrowed sorted unique keys; optional exact rank, lower rank, checked inverse mapping

## 前置条件与约束

- **input_type**：int64
- **preserves**：order
- **updates**：none

- **最大 n**：9223372036854775807
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：span copied or vector moved; duplicates/empty accepted; Borrowed sorted unique keys; optional exact rank, lower rank, checked inverse mapping
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

- **操作成本**：Build O(n log(n+1)), rank O(log(u+1))
- **空间**：Initial 8n sorting payload, resident O(u); capacity compaction temporary copy
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

排序去重后，第 i 个键恰有 i 个不同更小键，故秩保持相等与大小关系。压缩不保持数值距离，区间边界需先在原始值域定义，再二分映射。

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
- **来源路径**：其他/离散化.md
- **来源 blob**：a78e58e1a07bb587e099a300cd877fa849cb34b0
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
