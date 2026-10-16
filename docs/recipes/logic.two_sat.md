# 2-SAT 配方

资产 ID：`logic.two_sat`

类型：组合配方

问题：`boolean_constraints`

[文档目录](../README.md) | [结构化契约](../../contracts/logic_two_sat.json) | [公共头文件](../../include/algorithm_assets/graph/two_sat.hpp) | [可编译示例](../../examples/repository_expansion.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/graph/two_sat.hpp>
```

`graph/two_sat.hpp`。`TwoSat(variable_count)`；`Literal{variable,value}` 表示“变量取 value”；`add_clause(a,b)` 记录 a∨b；`force(a)`；`implies(a,b)`；`solve()` 返回 `{satisfiable,assignment}`。无解时 assignment 空，不是部分解；0 变量且无子句有解。变量数 ≤ (UINT32_MAX-1)/2，非法变量抛异常且不添加子句。每个 Clause 为两 Literal，本机 16 字节，add 摊销 O(1)，solve O(v+q)。构图临时 16q，CSR 8(2v+1)+8q，SCC O(v)，输出 uint8[v]。每次 solve 重建工作区，原子句不修改，可重复/并发 solve；无 UNSAT 核或自动删子句。

正确性：a∨b 等价于 ¬a→b 与 ¬b→a；变量与否定同 SCC 时无解，否则根据逆拓扑分量序选较早的 literal。组合复用真实无权 CSR 和 SCC。

## 输入与输出

### 输入

- **条件**：n Boolean variables; Literal {variable,value}; OR, force and implication clauses

### 输出

- **条件**：SAT flag and complete owned uint8 assignment; UNSAT has empty assignment

## 前置条件与约束

- **clause_form**：two_literals_or
- **result_guarantee**：exact
- **updates**：append_clauses

- **最大 n**：2147483647
- **容量限制**：size_t, container max_size and allocation limits additionally apply
- **数值与状态**：n Boolean variables; Literal {variable,value}; OR, force and implication clauses; SAT flag and complete owned uint8 assignment; UNSAT has empty assignment
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

- **操作成本**：Append amortized O(1); solve O(n+q)
- **空间**：16q stored clauses; 16q temporary arcs; CSR/SCC O(n+q), output n bytes
- **保证范围**：Worst-case unless explicitly stated; no calibrated performance claim

内存下界用于排除明显超预算方案，不是实际峰值或时延证书。输入、输出、同时存活的中间结果、capacity 与分配器开销应按实际调用路径计入；未测项目不宣称最优。

## 正确性依据

子句 a∨b 等价于 ¬a→b 与 ¬b→a。变量及其否定同 SCC 时互相强制矛盾；否则按逆拓扑分量序选取取值，蕴含方向不会违背赋值。这是正确性论证，尚未进行形式化验证。

## 关联资产

- 依赖使用：[强连通分量](../algorithms/graph.strong_components.md)（`graph.strong_components`）。
- 依赖使用：[无权 CSR](../representations/graph.unweighted_csr.md)（`graph.unweighted_csr`）。

关联边只说明已登记关系，不能替代整个组合的语义、精度和资源验证。

## 验证与来源

- **正确性证据**：Reasoned invariants and reductions in documentation, not formal verification
- **测试**：[tests/repository_expansion_test.cpp](../../tests/repository_expansion_test.cpp)
- **示例**：[examples/repository_expansion.cpp](../../examples/repository_expansion.cpp)
- **基准**：not_measured
- **判题证据**：[judge/EXPANSION_VALIDATION.md](../../judge/EXPANSION_VALIDATION.md)

- **来源仓库**：RyanTsi/Algorithm-notes
- **固定提交**：91c14556641c991c716b71906815abdd6a476188
- **来源路径**：2-SAT&&差分约束.md
- **来源 blob**：3c48077e77ad63ef2a930f7087127cbde4b316c4
- **实现来源**：Independently rewritten for this project; source motivates algorithm and constraints
- **许可状态**：No repository license discovered; project distribution license not selected
