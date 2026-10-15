# 个人仓库载入：图、序列、数值与优化

本轮从固定提交的两仓库筛选，并重新编写可直接 include 的实现。完整文件处理台账见 [REPOSITORY_IMPORT.md](REPOSITORY_IMPORT.md)，机器记录见 `provenance/repository-assets/decisions.json`。不是将每道题的 main 函数加入库；尚未实现的候选保持 deferred，不把暂缓说成已覆盖。来源未发现 LICENSE；本工程分发许可证尚未选定。外部资料只用于核对数学定义与复杂度，没有复制其实现。

所有接口在 `algorithm_assets`；几何在 `algorithm_assets::geometry`。默认借用只读 span，拥有返回值；容器分配可能抛出。索引 0-based、区间半开。输入在调用期间必须有效且不修改。除特别说明，输出不借用输入；普通对象复制是完整独立副本，没有结构共享的持久化、checkpoint、历史 rollback、undo/redo 或删除。不可变对象允许并发 const 访问；可变对象不能读写并发。移动后源对象仅用于销毁/重新赋值。

空间描述使用本机 64 位 size_t、32 位图顶点、64 位值；忽略容器头和分配器，容量可能大于逻辑长度。资源下界只用于排除明显超预算，未校准时间/峰值内存，不能给出可行性保证。

## 图与约束

### UnweightedCsrGraph

`graph/unweighted_csr.hpp`。构造 `(n,span<Edge{uint32 from,to}>)` 拷贝到不可变压缩邻接表；`neighbors(v).targets` 是借用视图，`vertex_count/edge_count/storage_bytes` 可查询。n ≤ UINT32_MAX-1；顶点/端点不合法抛 out_of_range。空图、自环、重边合法，保留邻接输入顺序。构建 O(n+m)，常驻 8(n+1)+4m，临时构建游标 8(n+1)。不含权值、不支持增删边，视图随对象存储有效。

原 `topological_sort` 同时支持此表示和原带权 CSR，算法仍 O(n+m) 且局部载荷最多 12n（含输出）。它遇环返回已处理前缀，不返回环见证。无权路径去掉了每条边 8 字节无用权值；带权路径继续可用。

### StrongComponents 与 Condensation

`graph/strong_components.hpp`。`strong_components(graph)` 返回拥有的 `component[v]`、`count`。迭代 Tarjan 用 discovery、low、活动栈和显式 DFS 帧，避免调用栈深度依赖 n；树边传播 low，活动返祖边使用 discovery[v]。O(n+m)，辅助 O(n)，本机每帧 16 字节，三个 uint32 数组（含输出）、活动栈和位标记另计。空图 count=0；每点都分配分量，含单点自环；ID 为逆拓扑序，跨分量边始终由大 ID 指向小 ID，不保证其它稳定编号语义。

`condense_graph(graph)` 是实际 SCC + 边归约配方，返回分区与新的无权 DAG。保留分量间平行弧，删除分量内部弧；不是对原图可逆的压缩。O(n+m)；除了 SCC 工作区，转换边临时最多 8m，新 DAG 8(c+1)+4m'，m'≤m，构建游标另计。重边不会破坏拓扑处理。不能用有向 SCC 代替无向割点/桥分析。

### TwoSat

`graph/two_sat.hpp`。`TwoSat(variable_count)`；`Literal{variable,value}` 表示“变量取 value”；`add_clause(a,b)` 记录 a∨b；`force(a)`；`implies(a,b)`；`solve()` 返回 `{satisfiable,assignment}`。无解时 assignment 空，不是部分解；0 变量且无子句有解。变量数 ≤ (UINT32_MAX-1)/2，非法变量抛异常且不添加子句。每个 Clause 为两 Literal，本机 16 字节，add 摊销 O(1)，solve O(v+q)。构图临时 16q，CSR 8(2v+1)+8q，SCC O(v)，输出 uint8[v]。每次 solve 重建工作区，原子句不修改，可重复/并发 solve；无 UNSAT 核或自动删子句。

正确性：a∨b 等价于 ¬a→b 与 ¬b→a；变量与否定同 SCC 时无解，否则根据逆拓扑分量序选较早的 literal。组合复用真实无权 CSR 和 SCC。

### Dinic

`graph/dinic.hpp`。`Dinic(n)`，n≤INT32_MAX；`add_edge(from,to,uint64 capacity)` 返回稳定逻辑边 ID；`edge(id)` 返回 `{from,to,capacity,flow}`；`max_flow(s,t,limit=UINT64_MAX)` 返回 **本次新增流量**及 `maximal`；`reset_flow()` 将所有边流清零；`residual_reachable(source)` 返回残量可达标记。s≠t，自环、平行弧、反向逻辑边、容量 0 均合法。两条残量弧使用 uint32 链表索引，总弧数最多 UINT32_MAX-1。

每条逻辑边是两个 16 字节弧；初始容量由成对剩余容量之和恢复，不多存一份 capacity/flow 数组。成员顶点工作区本机约 20n，弧载荷 32m，扩容冗余另计；显式增广路径没有递归栈。加边摊销 O(1)，一般图最坏 O(n²m)，reset O(m)，残量可达 O(n+m) 加本地 O(n) 空间。层次图允许的弧逐步被当前弧游标或死端剪枝消耗，每个阻塞流阶段结束后最短增广路变长。

调用会改变残量网络；同一对端点可以继续增广，仍可加新边。改变端点前必须 reset；端点错误不改变流。limit=0 只检查是否已最大。真实最大流超过 uint64 时可分多次调用，单次严格不超过 limit，**达到 UINT64_MAX 且 maximal=false 不是溢出截断后的“最大流”**。只有 maximal=true 时源侧残量可达集才是最小割证据。不支持单边删除/容量缩小、历史撤销或费用流。加边与增广会影响此前最小割证据；普通值复制可备份全状态但成本 O(n+m)。

### HopcroftKarp

`graph/bipartite_matching.hpp`。`hopcroft_karp(left_size,right_size,graph)` 返回左右配对数组、匹配数和最小点覆盖位图。左右 ID 是独立空间；graph 的顶点数必须等于 max(left,right)，只有 `[0,left)` 行可有弧，目标在 `[0,right)`。输入不可变，重边允许，空侧合法；未匹配项是 `BipartiteMatching::unmatched`（UINT32_MAX）。它处理无权二分图，不是最小费用指派或一般图匹配。

BFS 找最短增广长度，显式栈 DFS 同时增广一批不相交路径；最坏 O((n+m)√n)，辅助 O(n)，不递归。Konig 最小点覆盖为交替路径可达左点的补集加可达右点；结果配对载荷 4(left+right)，覆盖使用位存储。DFS 游标 size_t[left]、层次 uint32[left]、队列和路径 O(left)，输入 CSR 另计。与流的单位容量建模在条件上有关联，但实现直接运行 HK，未伪造对 Dinic 的依赖。

### BellmanFord 与 DifferenceConstraints

`graph/bellman_ford.hpp`。边 `{uint32 from,to;int64 weight}`；`bellman_ford(n,edges,optional<uint32>source)`。source=nullopt 是向所有点连零权边的虚拟源。返回每点 `{value,status}`，status 为 unreachable、finite、negative_infinity；只有 finite 的 value 有语义。可达负环影响后继，不把其它点误标为负无穷；没有路径见证。空图仅允许虚拟源。n≤UINT32_MAX-1；每条权重必须在 ±floor(INT64_MAX/max(n,1)) 内，超界明确拒绝。

同步双缓冲第 k 轮只使用最多 k 条边的距离，n 边权界确保包含负环检测轮的加法精确。第 n 轮改善的点及其可达后继是负无穷。最坏 O(nm)，每轮复制 O(n)，因此完整界 O(n(n+m))；提前稳定退出。两份 SignedDistance[n]（本机共 32n），负环标记 O(n) 位；传播重复边扫描避免第二份邻接存储。输入不变。

`solve_difference_constraints(n,constraints)` 将 v→u,w 解释为 x[u]≤x[v]+w，虚拟源求解，返回 optional<vector<int64>>。无解返回 nullopt；有解给出一组非正可行势，不表示任意目标下的最优解，也不唯一。整体仍 O(n(n+m))，附加输出 8n；同权界和错误行为。与非负 Dijkstra 的替代须核对输入表示、权界与负环状态。

### LowestCommonAncestor

`graph/lowest_common_ancestor.hpp`。构造只读 uint32 parent 数组，根指向自身；支持森林，非根循环与非法父亲明确拒绝。`query(a,b)`、`ancestor(v,steps)`、`distance(a,b)` 返回 optional；不同树无 LCA/距离，超过根的 ancestor 无值。空数组合法但不能查询。保存 depth/root 和层优先跳表，层数按实际最大深度而不是固定 20，树根链不递归。构建 O(n log(d+1))，查询 O(log(d+1))，载荷 4n(2+max(1,bit_width(d)))；构建状态和路径临时 O(n)。距离 uint64 是边数，不是带权距离；无动态重挂父亲。

## 序列、区间与静态查询

| 资产 / 独立头文件 | 输入输出与边界 | 最坏成本与理由 |
| --- | --- | --- |
| CoordinateCompression / `sequence/coordinate_compression.hpp` | int64 数组按值转移或 span 拷贝；keys 借用视图，rank 值缺失返回 nullopt，lower_rank 返回插入秩，value_at 越界抛异常；空/重复值合法 | O(n log n) 构建，O(log u) 查询；排序初始 8n，常驻 O(u)，过大容量压缩时有临时副本。保持顺序，不保持距离、面积或二维邻接 |
| IncreasingSubsequence / `sequence/subsequences.hpp` | int64；length 与 indices 两接口；nondecreasing=false 严格，true 允许相等；indices 是原序列位置，空返回空；不保证字典序最小见证 | O(n log(L+1))；length 只保存 O(L) 个值，见证保存 n 个父指针、O(L) 尾位置及 L 输出索引。每个长度的最小末尾为后续扩展保留最大空间 |
| CommonSubsequence / 同上 | string_view 字节序列，允许 NUL/空串；只返回 size_t 长度，不提供编辑脚本或 traceback | O(nm)，一行 8(min(n,m)+1)，不再存 n×m 表；对角旧值加上当前行/上一行维持 DP 不变量 |
| Manacher / `strings/manacher.hpp` | 字节字符串，允许任意字节，不插入 `$`/`#` 哨兵；odd[i] 含中心，even[i] 以 i 为右中心；longest `[first,last)`，并列选最早，空 `[0,0)`；n≤SIZE_MAX/2 | O(n)，两组 size_t 半径共16n，本机；镜像复制只在已知窗口内，真正向右扩展总数 O(n) |
| MergeIntervals / `sequence/intervals.hpp` | int64 半开区间；first>last 拒绝，空段丢弃；merge_touching 决定端点相接是否并合；按值允许转移，span 保持输入 | O(n log n)，排序载荷16n，原数组上压缩，稀疏输出压缩容量；输出拥有有序互不相交区间，原分段信息丢失 |
| MinimumIntervalCover / 同上 | 目标半开区间，空目标零段；无覆盖 nullopt；返回最少原始区间索引，需保留原始顺序解释；不处理加权最小费用 | O(n log n)，索引排序8n+O(k)输出；每步从能接当前前沿的段中选最远者，交换论证可替换任意最优解第一段 |
| SparseTable / `range/sparse_table.hpp` | T、纯 Combine 与只读数组；要求结合律和幂等性；非空 `[l,r)` fold，不需要幺元；空表可建，空/越界查询拒绝；无修改 | O(n log n) build，O(1) 两次载荷读取+一次合并；只存有效块，总元素 Σ(n-2^k+1)，不是每层 n 个填充。重叠块可因幂等性合并，普通 sum 不适用 |

CoordinateCompression 已被真实逆序对 Fenwick 配方复用。原配方说明的键容量更新为 O(u) 常驻，初始化排序仍需要 n 值拷贝；目录优先单算法规则保持。

## 数值、概率与信号

### Radix2Fft / RealConvolution

`numeric/fft.hpp`。`Radix2Fft(N)` 必须是非零 2 次幂，构建 O(N)，预计算 N/2 个 complex<double> 根（本机 8N 字节），不存位逆序数组。`transform(span<complex<double>>,inverse=false)` 长度严格等于 N；原地 O(N log N)、O(1) 额外空间。正变换使用 exp(-i2πkn/N)，逆变换除以 N。输入非有限值拒绝且不修改；计算出现非有限结果抛异常，此时数组已修改。数学上的逆变换不等于无损 undo，round-trip 有误差，不能启用破坏 IEEE 行为的 fast-math。

`convolve_real(a,b)` 是补零 + 两次 FFT + 逐点乘 + 逆 FFT 的真实配方；有限 double 输入，任一为空返回空，长度 n+m-1、N=bit_ceil(length)，长度/补零溢出拒绝。O(N log N)，两个复数工作区32N、根8N、实输出8length。输入不变，结果为近似实数，不自动四舍五入为“精确整数乘法”，没有整数精确性证书、任意长度 DFT、流式滤波或 GPU 加速。根表一次计算后 transform 可重复/并发用于不同数组。

数学定义与归一化对照 [FFTW 文档](https://www.fftw.org/fftw3_doc/What-FFTW-Really-Computes.html)。这是一条无外部依赖的可读基线，没有宣称性能达到 FFTW/MKL。

### DenseLu

`numeric/dense_lu.hpp`。输入 n×n 行优先有限 double 矩阵，按值可转移，span 会复制。部分行主元 LU 分解 O(n³)，常驻8n²+4n；`solve(rhs)` 长度 n，返回拥有的 x，O(n²)及8n输出，可重用分解、并发 solve。原矩阵/RHS 不修改。空矩阵可用；`log_determinant()` 返回符号与 log|det|，空矩阵为 `{+1,0}`，避免直接连乘上/下溢；求和复用 CompensatedSum。

relative_tolerance 在 `[0,1)` 且有限，默认64ε；阈值为 tolerance·输入无穷范数。零主元或小于阈值拒绝并抛 domain_error，**表示精确奇异或数值上不宜接受，不是证明矩阵奇异**。矩阵/RHS/中间计算非有限明确拒绝；零阈值仍是浮点近似，未提供条件数、秩、最小二乘、SVD、认证误差界或动态矩阵更新。矩阵行和溢出也拒绝。准确性依赖条件数，强缩放/病态输入宜交给成熟数值库。

分解复用与求解依据见 [LAPACK 线性方程文档](https://www.netlib.org/lapack/lug/node38.html)。本实现是小中规模的可读基线，不替代 BLAS/LAPACK 的块运算与诊断能力。

### AbsorbingMarkov 配方

`recipes/absorbing_markov.hpp`。`analyze_absorbing_markov(n,transient_count,transition,probability_tolerance,pivot_tolerance)` 要求瞬态在前、吸收态在后，吸收行严格为单位行；概率有限且 `[0,1]`，行和距1不超过 tolerance（默认1e-12，范围[0,1e-6]），不隐式归一化。先在正概率边上检查每个瞬态可达某个吸收态，不符合抛 domain_error；n=0/t=0 允许。不能把一般状态自动标成瞬态，也不接受只有“行和≤1”的次随机矩阵。

返回 t×a 行优先吸收概率和 t 个期望步数。令 A=I-Q，复用同一 DenseLu 解 AX=R 和 Aτ=1，**不显式求逆、不计算 P 的巨大幂**。最坏 O(n²+t³+at²)；常驻/峰值取决于8t² LU、4t排列、8ta输出、O(n)图可达工作区、O(t) RHS/解。容差接受近似概率、浮点病态可能拒绝；不会裁剪结果到[0,1]掩盖误差。未来几何 morphing/MDP 可复用此线性系统路径，但那些完整应用未实现。

## 背包与整数代数

`optimization/knapsack.hpp` 输入 `{size_t cost,int64 value}`；bounded 额外 size_t count。三个自由函数返回在 cost 总和**不超过 capacity**下的最大 int64 值，允许不取，负/零价值可忽略，不提供选取见证、恰好装满、多维约束或回滚；空物品返回0。容量表 C+1 必须可分配；真实可行值超 int64 抛 overflow_error，调用者输入不变。

- `knapsack_01`：预算从大到小更新，一件至多一次；允许零 cost，正价值仅加一次。O(n(C+1))、8(C+1) 工作区。
- `knapsack_unbounded`：预算从小到大更新；正价值零 cost 是无界目标，抛 domain_error；零/负价值零 cost 忽略。O(n(C+1))、8(C+1)。
- `knapsack_bounded`：按 cost 的余数分组维护单调候选队列，O(n(C+1))；dp8(C+1)，队列16(floor(C/cost)+1)，复用队列保留最大容量。候选以旧价值和位置保存，只算窗口内真实可行的数量差，避免 `k*value` 巨大但最优可表示时误报。零 cost 正价值取全部 count；count=0 忽略。

`algebra/modular.hpp`：`Modulus64(m)` 要求 m≥2，所有操作参数 uint64，add/subtract/multiply/power 自动按 m 归约，inverse 仅 gcd=1 时返回 optional 逆元，不要求 m 是质数。无堆存储，8字节模数。支持 `__uint128_t` 的编译器乘法用128位中间值，保持标准接口；其它后端用最多64步无溢出加倍法。power O(log exponent)次乘法，inverse O(log m)次 Euclid 迭代，各次含模乘成本；不可称所有平台 O(1) 乘法。负数需调用者定义归约方式。

同头 `chinese_remainder(span<Congruence{uint64 residue,modulus}>)` 支持不互质模数；modulus≥1，余数归约，空系统返回 `{0,1}`。不一致返回 nullopt，最终 lcm 超 uint64 抛 overflow_error；返回 `[0,lcm)` 唯一最小非负代表。结合 gcd 与模逆元，无大整数或浮点。每方程 O(log modulus)乘法/除法迭代，O(1)空间，结果不能支持任意长的大模数。

设置 `ALGORITHM_ASSETS_FORCE_PORTABLE_MODULAR` 可测试/使用无128位扩展的后端，所有翻译单元必须一致设置；不要取消编译器自己的特性宏而破坏系统头文件。

`algebra/prime_sieve.hpp`：`PrimeSieve(uint32 limit)` 构建奇数位压缩 Eratosthenes；`is_prime(x)` 仅 x≤limit，越界拒绝；`for_each_prime(callback)` 和 `primes()` 递增枚举，0/1非素数，2单独处理。构建 O(N log log N)，奇数标记约 N/16字节，无常驻质数表；primes 输出另有4π(N)及容量冗余，callback 成本另计。它比原线性筛降低标记存储，未提供 SPF/分解/欧拉函数，也未称时间优于所有筛法。

## 几何：覆盖与碰撞

`geometry/minkowski.hpp`：`minkowski_sum(a,b)`、`minkowski_difference(a,b)` 接受已验证 IntegerConvexPolygon，每个坐标限±5×10⁸，输出限±10⁹以保持现有整数内核。空输入得空；单点、线段、多边形均支持。严格凸边界按极角合并，平行同方向边同时推进，再旋转到最小字典序顶点；最坏 O(h+k)，输出最多 h+k 点（16字节/点），不额外排序/重建凸包；与 O(hk) 所有点对求和后凸包做独立对照。借内部工厂保留已证明的 canonical hull 不变量，未增加公开 unchecked 构造。

`TranslatedConvexOverlap(a,b)` 持有 A-B；`intersects(translation)` 在 O(log(h+k)) 判断 B 平移该向量后是否与 A 相交，包含边界接触。平移坐标也限±10⁹。差并非和的可逆运算，不支持旋转、缩放、非凸碰撞或穿透深度。需要输入点集时先复用已有凸包构建，成本另计。

`geometry/minimum_enclosing_circle.hpp`：`minimum_enclosing_circle(span<Point>,seed)` 返回 optional<Circle>；空为 nullopt，重复/单点/共线允许，坐标有限且限±10⁵⁰。输入不变，缩放至单位范围，用 long double 临时坐标和有种子 shuffle，三层增量构造；三点行列式为0时取最远直径，不除零。最坏 O(n³)，均匀随机排列、精确运算下期望 O(n)；固定种子与浮点实现没有最坏线性保证。工作区 n 个临时点（2*sizeof(long double)*n）及固定 PRNG 状态，没有递归。

输出中心舍入后重新扫描全部点求覆盖半径，必要时向上舍入，确保不会仅因中心转换而直接缩小半径；**仍不是认证精确最小圆**。近共线/病态构造可能失准或拒绝非有限结果，long double 位数依平台，不提供绝对误差上界。种子复现以相同标准库/平台为条件。复杂度核对参考 [最小覆盖圆作者资料](https://www.nayuki.io/page/smallest-enclosing-circle)；实现按用户笔记的增量流程独立重写，未复制该外部库。

## 验证入口

`examples/repository_expansion.cpp` 为可编译最小示例；`tests/repository_expansion_test.cpp` 包含 Floyd 闭包/SCC对照、枚举全部布尔赋值、穷举割与匹配、负环影响对照、父链 LCA、二次 DP/中心扩展、所有区间子集、DFT/朴素卷积、已知解 LU、枚举背包、CRT 穷举与试除、点对 Minkowski 凸包及全部 1/2/3 点候选圆。另含 30000 点长链、满 uint64 流、最高位模运算、NUL 和空输入；这些是测试证据，不是形式化证明或跨平台性能结论。
# 后续题目驱动组合

2026-10-15 从博客题目进一步验证并登记三个真实可调用配方：区间对最低费用、有限步差子序列、树离心率。它们的独立接口、输入输出、证明和成本见 [PROBLEM_COMBINATIONS.md](PROBLEM_COMBINATIONS.md)，正式 CF/AtCoder 判题与源码快照见 [COMBINATION_VALIDATION.md](../judge/COMBINATION_VALIDATION.md)。本文第二批的数量和验收为当时记录，当前总目录为 74 个资产、50 个公共头文件。
