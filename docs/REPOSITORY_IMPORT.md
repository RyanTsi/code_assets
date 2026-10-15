# 个人仓库载入台账

台账覆盖固定提交的 116 个 Markdown 文件。`full` 为完整候选正文审核，`sections` 为章节/代码结构审核，`inventory_only` 仅按目录判断范围；不声称所有正文都已逐字阅读。partial 和 deferred 都不是完整覆盖。

核心新增实现、数值/状态约束、空间成本和正确性说明见 [REPOSITORY_EXPANSION.md](REPOSITORY_EXPANSION.md)。每个映射 ID 都验证存在真实头文件、契约、测试与示例。源仓库无 LICENSE，本库整体许可证尚待选择。

```sh
python tools/repository_import.py
python tools/repository_import.py --report
```

处理状态：deferred=32；excluded=26；imported=30；partial=17；reference_only=10；standard_library=1。

| 来源路径 | 审核范围 | 状态 | 已有代码资产 / 处理理由 |
| --- | --- | --- | --- |
| RyanTsi/Algorithm-notes: `2-SAT&&差分约束.md` | full | imported | logic.two_sat, optimization.difference_constraints；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `Modint.md` | full | imported | algebra.modulus64；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `test.md` | full | reference_only | ；题解/题单供验证选题；不把局部竞赛结论做成通用 API； |
| RyanTsi/Algorithm-notes: `其他/全排列.md` | full | standard_library | ；直接使用 std::next_permutation/std::prev_permutation，无需复制； |
| RyanTsi/Algorithm-notes: `其他/区间合并.md` | full | imported | sequence.merge_intervals；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `其他/快读.md` | full | excluded | ；竞赛 I/O 加速模板不进入算法核心；EOF/符号边界也未完整处理； |
| RyanTsi/Algorithm-notes: `其他/最小区间覆盖数.md` | full | imported | sequence.minimum_interval_cover；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `其他/离散化.md` | full | partial | sequence.coordinate_compression；核心算法已实现或由现有资产覆盖；接口以工程契约为准；仅迁入一维有序压缩；二维网格拓扑/距离保持需单独验证 |
| RyanTsi/Algorithm-notes: `其他/表达式求值.md` | sections | deferred | ；语法、错误定位、深度界和算术域尚未确定，不直接迁入 char* 无校验解析器； |
| RyanTsi/Algorithm-notes: `其他/逆序对.md` | full | imported | sequence.inversions, sequence.inversions_fenwick；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `其他/高精度.md` | sections | deferred | ；需要有符号/除法/大数范围契约；优先评估成熟多精度后端，不能以十进制手写板子声称最优； |
| RyanTsi/Algorithm-notes: `动态规划/DP优化/凸包优化.md` | sections | deferred | ；有实际用途；必须区分斜率/查询单调条件、Li Chao 变体与乘法精确范围； |
| RyanTsi/Algorithm-notes: `动态规划/区间dp/石子合并.md` | full | deferred | ；相邻合并计划有价值；优化所需四边形不等式与成本域需单独明确； |
| RyanTsi/Algorithm-notes: `动态规划/子序列问题/最长上升子序列.md` | full | imported | sequence.increasing_subsequence；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `动态规划/子序列问题/最长公共子序列.md` | full | imported | sequence.common_subsequence；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `动态规划/背包问题/01背包.md` | full | imported | optimization.knapsack_01；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `动态规划/背包问题/二维背包.md` | full | deferred | ；有实际资源分配意义；本轮三种单容量接口不能冒充二维约束； |
| RyanTsi/Algorithm-notes: `动态规划/背包问题/分组背包.md` | full | deferred | ；组互斥语义有价值；当前单容量背包不表达分组约束； |
| RyanTsi/Algorithm-notes: `动态规划/背包问题/多重背包.md` | full | imported | optimization.knapsack_bounded；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `动态规划/背包问题/完全背包.md` | full | imported | optimization.knapsack_unbounded；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/其他/基环树找环.md` | full | deferred | ；题解假设简单基环图，未覆盖重边自环；通用环/无向边分解另行设计； |
| RyanTsi/Algorithm-notes: `图论/其他/拓扑排序.md` | full | imported | graph.topological_sort；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/其他/最近公共祖先.md` | full | imported | graph.lca；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/最小生成树/Kruskal.md` | full | imported | graph.kruskal；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/最短路/Bellman-Ford.md` | full | imported | graph.bellman_ford；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/最短路/Dijkstra.md` | full | imported | graph.dijkstra；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/最短路/Floyd.md` | full | deferred | ；APSP 有实用价值；负环、不可达与数值溢出需独立矩阵状态接口，不能冒充已有单源接口； |
| RyanTsi/Algorithm-notes: `图论/最短路/次短路.md` | sections | deferred | ；必须区分第二短距离、简单路径与允许重复行走；源码没有足够完整契约； |
| RyanTsi/Algorithm-notes: `图论/网络流/Dinic.md` | full | imported | graph.dinic；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/网络流/匈牙利算法.md` | full | imported | graph.hopcroft_karp；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `图论/网络流/费用流.md` | sections | deferred | ；有实用价值；负费用、负环、势函数和总费用溢出需要专门验证； |
| RyanTsi/Algorithm-notes: `图论/连通分量/tarjan.md` | full | partial | graph.condensation, graph.strong_components, graph.unweighted_csr；核心算法已实现或由现有资产覆盖；接口以工程契约为准；仅迁入有向 SCC/缩点；桥、割点和点双需独立无向边 ID 契约与测试 |
| RyanTsi/Algorithm-notes: `圆方树.md` | sections | deferred | ；网络可靠性有价值；依赖尚未迁入的无向点双与边 ID 保持规则； |
| RyanTsi/Algorithm-notes: `字符串/KMP.md` | full | imported | strings.kmp；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `字符串/Manacher.md` | full | imported | strings.manacher；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `数学/其他/整数分块.md` | full | deferred | ；整数商分组有价值；特定求和模板需声明权函数/前缀和与溢出边界； |
| RyanTsi/Algorithm-notes: `数学/博弈.md` | sections | reference_only | ；题解/题单供验证选题；不把局部竞赛结论做成通用 API； |
| RyanTsi/Algorithm-notes: `数学/多项式/FFT.md` | full | imported | numeric.radix2_fft, numeric.real_convolution；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `数学/数论/Lucas卢卡斯定理.md` | full | deferred | ；组合计数有价值；质数验证/预计算预算与大型模数后端需单独设计； |
| RyanTsi/Algorithm-notes: `数学/数论/中国剩余定理.md` | full | imported | algebra.chinese_remainder；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `数学/数论/数论定理.md` | full | reference_only | ；数学性质资料，不是独立可调用算法； |
| RyanTsi/Algorithm-notes: `数学/数论/欧几里得算法.md` | full | partial | algebra.modulus64；核心算法已实现或由现有资产覆盖；接口以工程契约为准；迁入模逆与模运算；gcd/lcm 用 std::gcd/std::lcm；特定丢番图题解未入库 |
| RyanTsi/Algorithm-notes: `数学/数论/欧拉函数.md` | sections | deferred | ；数论组件有价值；素数位筛不提供分解或 phi，保持明确缺口； |
| RyanTsi/Algorithm-notes: `数学/数论/质数筛.md` | full | imported | algebra.prime_sieve；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `数学/线性代数/矩阵快速幂.md` | full | deferred | ；线性递推与有限状态转移有价值；模域、矩阵形状和缓存乘法后端待规划； |
| RyanTsi/Algorithm-notes: `数学/线性代数/线性基.md` | full | imported | algebra.xor_basis64；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `数学/组合数学/组合数.md` | full | deferred | ；计数有价值；n<mod、逆阶乘合法性与内存预算需要单独契约； |
| RyanTsi/Algorithm-notes: `数据结构/ST表.md` | full | imported | range.sparse_table；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `数据结构/Splay.md` | sections | deferred | ；排序统计和序列编辑有价值；内存池/删除/迭代器/摊销负载需专门契约； |
| RyanTsi/Algorithm-notes: `数据结构/分块-由乃打扑克.md` | sections | excluded | ；不迁入 P5356 特定块结构和题目 I/O；通用分块潜力单独保留； |
| RyanTsi/Algorithm-notes: `数据结构/封装过的线段树.md` | sections | partial | range.segment_tree；核心算法已实现或由现有资产覆盖；接口以工程契约为准；普通幺半群树已覆盖；lazy action/composition 是单独能力缺口 |
| RyanTsi/Algorithm-notes: `数据结构/并查集.md` | full | imported | connectivity.dsu；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `数据结构/树状数组.md` | full | partial | range.fenwick_sum；核心算法已实现或由现有资产覆盖；接口以工程契约为准；基础求和已迁入；二维、区间更新和顺序统计未覆盖 |
| RyanTsi/Algorithm-notes: `数据结构/树链剖分.md` | sections | deferred | ；树路径聚合有价值；需先补 lazy 区间操作和方向保持，LCA 不替代此能力； |
| RyanTsi/Algorithm-notes: `数据结构/线段树.md` | sections | partial | range.segment_tree；核心算法已实现或由现有资产覆盖；接口以工程契约为准；点更新已覆盖；区间 lazy/标记永久化尚未迁入 |
| RyanTsi/Algorithm-notes: `数据结构/莫队算法.md` | sections | deferred | ；可用于离线分析；暂未确定聚合的可逆更新语义与工程负载，不视为天生无用； |
| RyanTsi/Algorithm-notes: `算法.md` | sections | partial | graph.dijkstra, graph.dinic, graph.hopcroft_karp, graph.strong_components, logic.two_sat, numeric.radix2_fft, range.fenwick_sum, range.segment_tree, strings.kmp, strings.manacher；核心算法已实现或由现有资产覆盖；接口以工程契约为准；综合稿多个核心已覆盖；章节级审核，不声称逐行验证全部代码；lazy/HLD/持久化/费用流等仍暂缓 |
| RyanTsi/Algorithm-notes: `线段树例题.md` | full | reference_only | ；题解/题单供验证选题；不把局部竞赛结论做成通用 API； |
| RyanTsi/Algorithm-notes: `网络流.md` | sections | partial | graph.dinic, graph.hopcroft_karp；核心算法已实现或由现有资产覆盖；接口以工程契约为准；最大流/最小割与二分匹配已覆盖；最小割必须边/可行边和路径覆盖等推论尚未实现 |
| RyanTsi/Algorithm-notes: `补题.md` | sections | reference_only | ；题解/题单供验证选题；不把局部竞赛结论做成通用 API； |
| RyanTsi/Algorithm-notes: `补题优先队列.md` | sections | reference_only | ；题解/题单供验证选题；不把局部竞赛结论做成通用 API； |
| RyanTsi/Algorithm-notes: `计算几何/Geometry.md` | sections | partial | geometry.circle, geometry.closest_pair.int, geometry.convex_hull.int, geometry.linear, geometry.primitives；核心算法已实现或由现有资产覆盖；接口以工程契约为准；既有几何模块覆盖主要基础；不能认定原模板每个函数完全等价 |
| RyanTsi/Algorithm-notes: `计算几何/三角剖分.md` | full | deferred | ；原文主要概念，无可复用实现；Delaunay 需要鲁棒 incircle 谓词，当前浮点内核不足； |
| RyanTsi/Algorithm-notes: `计算几何/最小圆覆盖.md` | full | imported | geometry.minimum_enclosing_circle；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/Algorithm-notes: `计算几何/闵可夫斯基和.md` | sections | imported | geometry.minkowski_difference.int, geometry.minkowski_sum.int；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/blog: `AGENTS.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `archetypes/default.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/about.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/archives.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/2022CCPC河南省赛题解.md` | sections | reference_only | ；题单/题解按章节浏览；保留测试选题和通用核心来源，不直接迁入整题程序； |
| RyanTsi/blog: `content/posts/2023多校杂题.md` | sections | reference_only | ；题单/题解按章节浏览；保留测试选题和通用核心来源，不直接迁入整题程序； |
| RyanTsi/blog: `content/posts/ArchLinux安装指南.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/CMAKE笔记.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/CPP11语法笔记.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/DataBase Usage.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/Effective C++笔记.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/English.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/Git Usage.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/Linux 常用软件安装及其配置.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/Splay.md` | sections | deferred | ；排序统计和序列编辑有价值；内存池/删除/迭代器/摊销负载需专门契约； |
| RyanTsi/blog: `content/posts/dp题单.md` | sections | partial | sequence.bounded_step_subsequence；已从选定题目沉淀实际可调用组合配方，并进行整体对拍；其余题目与专用业务逻辑未自动迁入。；本文其它题目尚未逐题进行工程化与新库判题 |
| RyanTsi/blog: `content/posts/rust小记.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/三角剖分.md` | full | deferred | ；概念稿无实现；Delaunay 鲁棒 incircle 内核未覆盖； |
| RyanTsi/blog: `content/posts/上海-苏州游记.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/傅里叶变换和谱域.md` | sections | partial | numeric.radix2_fft, numeric.real_convolution；核心算法已实现或由现有资产覆盖；接口以工程契约为准；仅映射已实现核心；SVD/正则化、完整网格 morphing、图谱滤波、lazy 或特定题目的其余应用未自动覆盖 |
| RyanTsi/blog: `content/posts/几何概念.md` | full | deferred | ；三维直线概念不属于当前二维内核；待几何维度扩展； |
| RyanTsi/blog: `content/posts/分块简述.md` | sections | deferred | ；通用 sqrt 分解有价值；需要具体聚合、更新和负载，题目专用块实现不直接移植； |
| RyanTsi/blog: `content/posts/北京游记.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/博弈论.md` | sections | deferred | ；进一步审核领域契约后再迁入； |
| RyanTsi/blog: `content/posts/可持久化数据结构.md` | full | deferred | ；原文为占位草稿，不构建未实现 API； |
| RyanTsi/blog: `content/posts/吸收马尔科夫链.md` | full | partial | probability.absorbing_markov；核心算法已实现或由现有资产覆盖；接口以工程契约为准；仅映射已实现核心；SVD/正则化、完整网格 morphing、图谱滤波、lazy 或特定题目的其余应用未自动覆盖 |
| RyanTsi/blog: `content/posts/图论做题记录.md` | sections | partial | graph.tree_eccentricities；已从选定题目沉淀实际可调用组合配方，并进行整体对拍；其余题目与专用业务逻辑未自动迁入。；本文其它题目尚未逐题进行工程化与新库判题 |
| RyanTsi/blog: `content/posts/圆方树.md` | sections | deferred | ；网络可靠性有价值；依赖尚未迁入的无向点双与边 ID 保持规则； |
| RyanTsi/blog: `content/posts/基环树简述.md` | sections | deferred | ；题解假设简单基环图，未覆盖重边自环；通用环/无向边分解另行设计； |
| RyanTsi/blog: `content/posts/工程思维.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/强化学习.md` | sections | deferred | ；保留 MDP/值迭代等候选；训练与环境接口、收敛条件、随机状态/后端未确定，不新建假 ML API； |
| RyanTsi/blog: `content/posts/数值计算-矩阵论.md` | sections | partial | numeric.dense_lu；核心算法已实现或由现有资产覆盖；接口以工程契约为准；仅映射已实现核心；SVD/正则化、完整网格 morphing、图谱滤波、lazy 或特定题目的其余应用未自动覆盖 |
| RyanTsi/blog: `content/posts/最小圆覆盖.md` | full | imported | geometry.minimum_enclosing_circle；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/blog: `content/posts/深度学习.md` | sections | deferred | ；教学训练流程暂缓；需选成熟张量/自动微分后端并真实训练验证； |
| RyanTsi/blog: `content/posts/线性基.md` | full | imported | algebra.xor_basis64；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/blog: `content/posts/线段树.md` | sections | partial | graph.dinic, range.segment_tree；核心算法已实现或由现有资产覆盖；接口以工程契约为准；仅映射已实现核心；SVD/正则化、完整网格 morphing、图谱滤波、lazy 或特定题目的其余应用未自动覆盖 |
| RyanTsi/blog: `content/posts/线段树题单.md` | full | reference_only | ；题单/题解按章节浏览；保留测试选题和通用核心来源，不直接迁入整题程序； |
| RyanTsi/blog: `content/posts/网络流做题记录.md` | sections | partial | graph.dinic；核心算法已实现或由现有资产覆盖；接口以工程契约为准；仅映射已实现核心；SVD/正则化、完整网格 morphing、图谱滤波、lazy 或特定题目的其余应用未自动覆盖 |
| RyanTsi/blog: `content/posts/莫队.md` | sections | deferred | ；可用于离线分析；暂未确定聚合的可逆更新语义与工程负载，不视为天生无用； |
| RyanTsi/blog: `content/posts/表达式求模板值.md` | sections | deferred | ；语法、错误定位、深度界和算术域尚未确定，不直接迁入 char* 无校验解析器； |
| RyanTsi/blog: `content/posts/计算几何做题记录.md` | sections | reference_only | ；题单/题解按章节浏览；保留测试选题和通用核心来源，不直接迁入整题程序； |
| RyanTsi/blog: `content/posts/计组笔记.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/量化交易.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `content/posts/闵可夫斯基和.md` | full | imported | geometry.minkowski_difference.int, geometry.minkowski_sum.int, geometry.translated_convex_overlap；核心算法已实现或由现有资产覆盖；接口以工程契约为准； |
| RyanTsi/blog: `content/posts/高质量题解（持续更新中）.md` | sections | partial | sequence.disjoint_interval_pair；已从选定题目沉淀实际可调用组合配方，并进行整体对拍；其余题目与专用业务逻辑未自动迁入。；本文其它题目尚未逐题进行工程化与新库判题 |
| RyanTsi/blog: `content/search.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `reports/content-audit.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `reports/frontmatter-diff.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `reports/repair-proposals.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `tools/README.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
| RyanTsi/blog: `文章风格分析结论.md` | inventory_only | excluded | ；开发工具/个人记录/领域应用或站点维护材料；本轮算法核心范围外，未声称正文已阅读； |
