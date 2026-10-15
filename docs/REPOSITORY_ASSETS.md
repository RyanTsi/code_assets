# 从个人笔记迁入的可调用资产

本文件保留第一批迁入记录。第二批已实现 FFT、最小圆覆盖等原暂缓候选，更新状态以 [REPOSITORY_EXPANSION.md](REPOSITORY_EXPANSION.md) 和 [完整台账](REPOSITORY_IMPORT.md) 为准。

本批以实际可复用的需求筛选：精确字节搜索、动态区间统计、网络布线、依赖调度、排序差异度量及 GF(2) 消元。不是逐题翻译。实现重新编写，保留来源与语义差异；原文没有发现 LICENSE，未据此推定第三方许可。工程整体分发许可证仍待选择。

已取得两仓库完整递归树，筛出的 Markdown 文件清单保存于 `provenance/repository-assets/`；实际精读的是下面列出的候选文章，未宣称逐字读完所有博客与长篇汇总文件。

| 来源（固定提交） | 阅读内容 | 处理 |
| --- | --- | --- |
| [Algorithm-notes](https://github.com/RyanTsi/Algorithm-notes/tree/91c14556641c991c716b71906815abdd6a476188) | 字符串/KMP.md；数据结构/树状数组.md；图论/最小生成树/Kruskal.md；图论/其他/拓扑排序.md；其他/逆序对.md；数学/线性代数/线性基.md | 迁入以下六种算法及一个组合 |
| [blog](https://github.com/RyanTsi/blog/tree/6ea0ef09d8479c8775654c1627710af6da1d5392) | content/posts/线性基.md | 迁入 GF(2) 核心，排除特定博弈结论 |
| 同上 | content/posts/最小圆覆盖.md；傅里叶变换和谱域.md | 有实用价值，暂缓至数值、复杂度与验证方案齐备 |
| Algorithm-notes | 算法.md 开头部分（非全文） | 莫队与 P5356 特定优化暂不收录；不能据此断言所有相关技术无实用价值 |

下面的字节成本使用本机 64 位 size_t；容器头、容量冗余、分配器、调用者输入另计。并未用评测 AC 证明全输入正确或工程性能最优。所有对象支持普通值复制形成独立副本，但没有内建持久化版本、checkpoint、历史 undo/redo 或分支共享。const 查询可并行读取且不得同时修改；复用逆序对工作区不可并发调用。分配失败可能抛出；对象移动后仅保证可销毁或重新赋值。

## FenwickSum

`range/fenwick_sum.hpp`：`FenwickSum(size)` 建零数组；`FenwickSum(span<const int64_t>)` 复制为树；`add(position,delta)`；`prefix_sum(last)` 和 `range_sum(first,last)`；`size()`、`storage_bytes()`。索引从 0 起，区间半开，空区间返回 0，空数组允许查询 `[0,0)`。不支持插入、删除、范围加、顺序统计或一般非交换聚合。

初始化 O(n)，单点加和查询最坏 O(log(n+1))，树载荷 8n 字节；查询只用固定 16 字节宽累加器。每个存储块的总和必须落在 int64 范围。输入各点是 int64，但更新只检查块和，逻辑点值可能超界；查询的最终结果超界则抛 overflow_error。两次遍历让更新先验证、后写入，失败不改变树。构建不要求中间前缀和都适配 int64；区间差也不会因两端前缀单独超界而误报。越界索引/区间抛 out_of_range。负数允许，故不提供依赖非负频数的 kth 搜索。

不变量：tree[i] 保存 `[i & (i+1),i]`，查询将前缀拆成互不相交的块；构建合并已完成的子块，各子块总访问数小于 n。相对当前通用线段树的 2n 元素，它用 n 个元素，并以“整数加法、单点增量”为更窄的适用条件。

## KmpPattern

`strings/kmp.hpp`：构造时拥有 std::string 模式并生成前缀表；`find_first(text)` 返回 optional<size_t>；`contains(text)`；`for_each_match(text,callback)`；`find_all(text)` 返回所有偏移；`pattern()`、`prefix_function()` 返回借用视图，随对象存储有效。text 在调用期间借用且不能修改。callback 可抛出，模式状态不变，不得在 callback 中修改模式对象或 text。

精确比较字节，包括内嵌 NUL；偏移也是字节，未实现 Unicode 码点、规范化、大小写折叠或编辑距离。重叠匹配保留；空模式匹配 `[0,n]` 的每个边界，find_first 返回 0。不存在用 nullopt 表示。模式长度 m 的构建 O(m)，搜索 O(n+k) 加 callback 成本；常驻 m 个字符加 m 个 size_t（本机约 9m 载荷），扫描 O(1) 临时空间，find_all 额外 k 个 size_t 及容量冗余。不支持跨块流式状态。

不变量：matched 是已扫描文本后缀与模式前缀的最长匹配长度；失败沿最长 border 下降，文本位置从不回退，下降总次数受上升总次数约束。原笔记只返回首次命中且未处理空模式；资产定义了全部命中及空模式语义。

## Kruskal

`graph/kruskal.hpp`：`kruskal_minimum_spanning_forest(n,span<const UndirectedWeightedEdge>)`。边为 `{uint32 first,second; int64 weight}`，顶点 `[0,n)`；n ≤ INT32_MAX，边数受 vector/size_t 和可用内存限制。允许负权、平行边、自环、孤立点；无向、静态，输入不变。返回拥有的原始边索引、int64 total_weight、component_count、is_spanning_tree。索引解释需保留原始边顺序。n=0 返回空森林且不是树；n=1 是树。断连返回覆盖所有分量的最小森林，不用 -1 混淆负权。

最坏 O(n+m log(m+1))，排序 m 个 size_t 索引（本机 8m），并查集 4n，输出至多 min(m,n-1) 个 size_t，排序栈 O(log m)。边记录不复制。借用输入的并发读取允许；不支持在线删边/更新后的增量 MST。非法端点抛 out_of_range；最终权重超 int64 抛 overflow_error，调用者输入不变。中间和使用 WideInt64Sum，以允许抵消。

正确性：每条跨当前分量的最轻边是相应割上的安全边；依次合并维持可扩展为最小森林的不变量。复用已有 DisjointSet。负权与断连行为超出了对应洛谷模板题的验证范围，由本地穷举森林对照补充。

## TopologicalOrder

`graph/topological_sort.hpp`：`topological_sort(const CsrGraph&)` 返回 `{vertices,acyclic}`。有向、静态，空图合法。重复边和自环允许。输入 CSR 保持不变，可重复调用。acyclic=true 时是全部顶点的一个拓扑序；false 时 vertices 仅为已处理前缀，未提供具体环路径，未处理顶点也不等于全部环顶点。不是字典序最小拓扑序。

O(n+m)，局部入度 size_t[n]，结果同时作为队列 Vertex[n]，本机载荷最多 12n（含输出），无另一份队列，分配失败输入不变。复用 CSR 的权值被忽略，但构造该表示仍要求非负 int64 权；n ≤ UINT32_MAX-1。CSR 自身权值数组对纯拓扑需求冗余，因此此路径不宣称全局空间最优；未来有真实负载再引入无权 CSR。

正确性：入度表示未移除顶点的入边数，零入度顶点可安全输出。处理数不足 n 时剩余子图没有零入度点，有限有向图必含环。原文修改全局入度，资产每次建立局部工作区。

## XorBasis64

`algebra/xor_basis.hpp`：完整 uint64 位域的 GF(2) 向量空间。`insert(x)` 返回是否增加秩；`contains(x)`；`rank()`；`maximize(seed=0)` 是仿射集合 seed XOR span 的最大值；`minimum_nonzero()`；`merge(other)`（包括自合并）；`kth_span_value(index)` 返回从 0 开始的去重、有序张成空间值，**包含空子集产生的 0**。秩 r<64 时 index≥2^r 返回 nullopt；r=64 时所有 uint64 索引合法，不计算越界的 1<<64。全零/空输入只有值 0，minimum_nonzero 返回 nullopt。

固定 64 个 uint64 加秩，本机 sizeof=520；无堆分配。insert/contains/max/min O(64)，merge 和 kth 最坏 O(64²)，kth 额外 512 字节局部行副本。没有输入集合副本、重复次数、组合见证、删除、按非空子集计数或历史回滚。uint64 无精度误差；kth 的 index 本身不能表达 2^64。

不变量：每个非零行有唯一最高位，插入消掉高位后建立新主元；贪心从高位优先提高 XOR 数值。kth 在局部副本消掉各行的低主元，低到高编号的行组合与递增值一一对应。原模板的有符号移位、低 62 位和 kth 零值语义在此重新定义。

## InversionCounter

`sequence/inversions.hpp`：int64 序列的严格逆序对 `(i<j && a[i]>a[j])`，相等不计。`count(values)` 保持输入；`count_inplace(mutable_span)` 将输入稳定排序；自由函数 `count_inversions` 提供一次性调用。结果 uint64；空/单元素返回 0；n(n-1)/2 必须不超过 UINT64_MAX，事先校验，无溢出静默截断。最大 n 为 6074001000（仍受 size_t、容器容量、内存限制）。无数值近似。

最坏 O(n log(n+1))；count 两份 n 个 int64 工作区（16n），新对象 inplace 仅一份（8n），递归栈 O(log n)。两缓冲交替充当源/目标，避免每层再复制回去。对象复用保留历史最大容量；之前调用 count 后再 inplace 不会自动释放另一缓冲。`storage_bytes()` 显示保留容量，`release()` 释放。分配/尺寸错误发生在输入排序前；不保存原顺序或撤销记录。工作区不可同时调用。

正确性：递归得到两段有序序列；右侧元素小于左侧首元素时，与左段剩余全部元素形成跨段逆序对，加 middle-left；稳定选择左侧等值确保严格语义。

## InversionsFenwick 配方

`recipes/inversions_fenwick.hpp`：`count_inversions_fenwick(span<const int64_t>)` 返回同一严格 uint64 计数。复制、排序、去重得到有序键；从右往左查询小于当前秩的频数，再将当前秩加一。排序压缩保持大小关系，统计树中始终只有当前元素右侧的频次，保证整体正确。

O(n log(n+1))；键向量仍保留 n 容量约 8n，u 个唯一键的 Fenwick 8u，排序栈 O(log n)，输入不变。共享逆序对计数尺寸检查，频数 ≤ n，当前允许范围内落在 int64。本配方不会优于所有输入下的归并实现；它验证“有序压缩 + 已有 Fenwick”的真实代码组合。目录存在单算法时优先返回归并；仅请求已验证配方时可得到此组合。关系说明目前不是自动生成组合的证明。

## WideInt64Sum

`numeric/wide_int64_sum.hpp`：固定 16 字节、两字的有符号 128 位精确累加状态；`add(int64)`、`subtract(int64)`（含 INT64_MIN）、`value()`、`clear()`；初始化为 0。单操作 O(1)，无堆分配。累加范围为 [-2^127,2^127-1]，越界抛 overflow_error 且状态不变；value 只在最终值适配 int64 时返回，超界不改变状态。非一般大整数，不输出 128 位格式；不支持持久化/历史记录。add/subtract 可互逆的前提是两步均不超累加范围，与提供历史 undo/redo 不同。

正确性：低字使用无符号模 2^64 运算，高字加符号扩展和进位/借位；转换前同时检查高字与低字符号，避免有符号溢出。为 Fenwick 与森林权重解决“中间超 int64、最终可表示”的抵消问题。

## 复现与验证

最小示例 `examples/repository_assets.cpp`；随机朴素对照、全子集 GF(2) 枚举、小图穷举森林、闭包判环、边界与多翻译单元验证见 `tests/repository_assets_test.cpp` 和 `repository_assets_odr.cpp`。题目入口、可提交单文件、哈希与实际判题记录见 [judge/VALIDATION.md](../judge/VALIDATION.md)。库使用 C++20；线上 AC 不能替代本地异常/空输入/负权/64 位最高位检查。
