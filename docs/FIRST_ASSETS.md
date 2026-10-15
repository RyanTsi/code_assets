# 首批算法：使用与设计依据

代码统一位于 `include/algorithm_assets/`，使用 C++20。完整可编译调用在 `examples/first_algorithms.cpp`，结构化契约在 `contracts/*.json`。

本页说明是正确性论证，不是形式化证明。所有实现为本工程新写；尚未进行跨平台性能基准，因此不宣称时间与空间同时最优。

## DisjointSet

头文件：`connectivity/disjoint_set.hpp`。构造 `DisjointSet(n)`；随后调用 `merge(u,v)`、`same(u,v)`、`leader(v)`、`component_size(v)` 和 `component_count()`。

固定顶点集合，初始每个顶点独立。顶点 ID 为 `[0,n)`，`n <= INT32_MAX`；空集合合法，但没有合法顶点。越界抛 `out_of_range`，过大规模抛 `length_error`，分配也可能失败。

`merge` 返回是否真正合并两个集合。代表元只是实现选择，不能作为跨操作保持不变的业务标识。查询可能修改内部路径，不能同时并发调用。没有删除、checkpoint、undo、redo 或历史版本 API；普通对象复制是 O(n) 全量复制。

根节点用负数保存集合大小，非根保存父节点编号，因此只需要一个 `int32_t` 数组。合并小集合到大集合；路径压缩只改变到根的路径，不改变集合划分。负大小相加仍在声明的整数界内。

初始化 Theta(n)，操作序列均摊 O(alpha(n))，单次最坏 O(log(n+1))。逻辑数组载荷 4n 字节，容器头、capacity 和分配器额外计入；查询使用迭代循环，无递归栈。

需要撤销时选择 `RollbackDisjointSet`。单纯需要在线删边不能套用这两个结构。

## RollbackDisjointSet

头文件：`connectivity/rollback_disjoint_set.hpp`。API 与普通并查集相近，但查询为 const；另有 `checkpoint()`、`rollback(token)` 和 `undo_last_merge()`。

不做路径压缩，仅按大小合并，使一次有效合并只修改两个根。每次记录父根、子根和父根原来的负大小；撤销时由合并后的负大小减去父根原大小恢复子根大小。按后进先出顺序撤销可恢复之前的森林和集合数。

只有成功合并写日志。`undo_last_merge()` 撤销最近一次成功合并，而不是最近一次 API 调用；没有历史时返回 false。`rollback` 撤销至指定日志深度，未来深度抛异常。

**token 只适用于同一实例的当前祖先历史**，不是持久化版本 ID。回滚后丢弃的未来 token、其他对象的 token 即使数值恰好相同，也不能使用；这一误用无法由深度值检测。不要将 token 跨对象复制、分支或任意调用序列传递。读者应采用嵌套的进入/退出模式。

查根 O(log(n+1))；有效合并还包含 vector 日志追加的均摊 O(1)，一次扩容可能 O(h)。撤销 k 次合并需 Theta(k)，单次 undo 为 O(1)。当前有效日志 h <= n-1；逻辑载荷为 4n+12h 字节，capacity 与扩容峰值另计。仅无写入时允许并发只读查询。

可用于离线连通性配方；不提供任意删除、redo 或历史分支查询。

## SegmentTree

头文件：`range/segment_tree.hpp`。构造 `SegmentTree<T,Combine>(values, identity, combine)`；支持 `get(i)`、`set(i,value)`、`fold(l,r)` 和 `all_fold()`。

Combine 必须是纯、确定、可通过 const 对象调用的结合运算，identity 必须是左右单位元。concept 只检查调用形式与返回 T，不能验证结合律。无需交换律，因此矩阵乘法、字符串拼接等有序运算也可使用。

输入复制到自有存储。范围为 `[l,r)`，空范围返回 identity；空数组合法。坏索引/范围抛异常且不改变数据。2n 个 T 必须可由 vector 分配；整数运算不得溢出。普通浮点加法不能当作精确满足结合律的运算。

使用紧凑 2n 迭代布局。查询分别维护左侧前缀聚合与右侧后缀聚合，最后按顺序合并；被取出的节点覆盖目标叶子的互不相交部分，因此得到正确的有序区间结果。修改叶子后重建祖先。

长度不是二次幂时，物理根的叶子顺序可能不是逻辑顺序，所以 `all_fold` 调用完整区间查询，而不直接返回根。这是空间紧凑与全区间 O(1) 查询之间的明确取舍。

建树 O(n*C)，修改/查询/全区间查询 O(log(n+1)*C)，C 包含 T 的复制与一次 Combine 成本。逻辑存储为 2n 个 T 和额外 identity/Combine；T 内部动态分配也必须计入。字符串拼接的 C 并非常数。

`get` 返回借用的 const 引用，依赖对象存储；set 可能改变它看到的值。无写入且 Combine 自身线程安全时允许并发读取。若 T 赋值或 Combine 在 set 中抛异常，聚合缓存可能只更新了一部分，须重建/丢弃对象后再用。没有区间修改、undo 或持久化 API。

## CsrGraph

头文件：`graph/csr_graph.hpp`。`CsrGraph(n, edges)` 将 `{from,to,int64 weight}` 的边列表复制成不可变有向图，顶点为 uint32，权重非负且不超过 INT64_MAX，`n < UINT32_MAX`。空图、重边、自环合法；无向边需显式传入两个方向。

`neighbors(v)` 返回两个等长 const span：targets 与 weights；每个顶点内保留输入边的顺序。借用视图依赖图存储，不跨销毁、赋值或移动使用；移动后的原对象只用于销毁或重新赋值。

先计数并求前缀和，再分发边，故每个顶点对应一段连续存储。目标与权重分成两个数组，避免每条边结构体的对齐填充。构造 Theta(n+m)，邻接视图 O(1)；逻辑载荷 `sizeof(size_t)*(n+1)+12m` 字节，构建游标另需 `sizeof(size_t)*(n+1)`，输入边列表另计。

不可变图可供并发读取或多个独立 Dijkstra 调用。负权拒绝构造：这个首批存储变体有意服务非负整数最短路，不能直接作为所有图算法的通用图类型。

## Dijkstra

头文件：`graph/dijkstra.hpp`。`dijkstra(graph, source)` 返回自有的 `ShortestPaths`，支持 `distance_to(v)`、`predecessor(v)` 和 `path_to(v)`。source 必须合法，空图不能调用。

距离是 optional<uint64>，合法有限值为 `[0,UINT64_MAX-1]`，nullopt 严格表示不可达。源点路径为单元素序列，不可达路径为空；源点和不可达点的 predecessor 为 nullopt。多个最短路中返回其中一条，不承诺字典序最小。

非负权保证每次取出的最小暂定距离已是最短距离；只在严格改进时设置父节点，避免零权环形成父指针循环。索引堆为每个顶点最多保留一项，减少键值时上浮，不保留过时重复项。

权值及加法经过范围处理：太长的候选途径不可能改善可表示的距离，可跳过。但如果确有溢出候选，搜索完成后检查图可达性，确保不会把超出表示范围的可达点误报为不可达；出现这种点时抛 overflow_error。仅有超长绕路而最短路可表示时仍正常返回。

令 r 为成功松弛次数，r <= m：时间 O(n+m+(n+r)*log(n+1))。结果逻辑载荷 12n 字节，堆及位置数组最多 8n；溢出分支释放堆后改用可达性工作区，最多约 5n 逻辑字节。图存储、vector 实际容量、返回路径分配另计。`path_to` 时间与输出空间均为 O(路径长度)。

结果不依赖图继续存活，允许并发 const 读取。当前没有负权、动态图增量更新、多源 API 或浮点权值变体。

## CompensatedSum

头文件：`numeric/compensated_sum.hpp`。流式 `CompensatedSum::add/value/reset`，或直接 `compensated_sum(span<const double>)`。这里实现 Neumaier 补偿求和。

当普通加法遗漏了低位贡献，通过误差项累计补偿，再加到总和上。根据当前和与新值的绝对值大小选择误差计算顺序，处理新值较大的情况；例如 `[1e16,1,-1e16]` 普通逐项求和得到 0，本实现得到 1。

要求有限 double 输入、通常的 IEEE 浮点计算及默认舍入环境，禁止 fast-math 或会重结合表达式的优化。NaN/无穷抛 invalid_argument；工作和、补偿或最终值溢出抛 overflow_error，失败的 add 不改变累加器。即使最终真实和有限，中间溢出也可能导致拒绝。

输出仍是近似浮点数，不能保证任意数据上精确或正确舍入，没有无条件固定误差上限。极端消去、下溢及不同浮点环境仍需领域验证。空输入为零。

每次操作 O(1)，数组求和 Theta(n)，状态仅两个 double。对象复制 O(1) 可手动保存状态，但没有 checkpoint、undo 或 redo API。仅无 add/reset 时允许并发 value 读取。

## Offline connectivity

头文件：`recipes/offline_connectivity.hpp`。传入顶点数和完整 `ConnectivityEvent` 列表，事件为 add/remove/query，返回按 query 出现顺序排列的 `vector<bool>`。

初始图为空、无向；边两端顺序无关，支持自环和多次添加。同一边按引用计数处理，remove 只减少一份；最后一份删除前边仍有效。删除不存在的边或未知事件类型抛 invalid_argument；坏顶点抛 out_of_range。合法空事件列表返回空结果。

步骤：先将每条边转换为 `[添加时刻,最后一份删除时刻)` 的生存区间，再放入时间树节点。DFS 进入节点时合并其覆盖期间有效的边，叶子回答当前时刻查询，离开时恢复 checkpoint。根到叶的边恰好构成该时刻有效图，因此答案等于该图的连通性。

时间树采用紧凑索引与 CSR 风格的事件数组，避免每个节点单独分配 vector。它的职责是分发时间区间，并不等同于普通 SegmentTree 的幺半群聚合，因此没有强行复用那个类。当前只把这段结构留在配方内；出现第二个实际使用者再考虑抽出组件。

q 为事件数，K 为边生存区间数：预处理 O(q*log(q+1)+K*log(q+1))，处理 O(n+q+K*log(q+1)*log(n+1))，空间 O(n+q+K*log(q+1))。map 节点、构建游标、中间区间、输出和分配峰值需另外细化；回滚日志任一时刻最多 n-1 次有效合并。

全部事件必须事先已知，不能回答实时在线删边问题。内部回滚不等于对调用者暴露 undo/redo；它也不提供持久化版本。多个独立调用可并行，返回的 vector<bool> 不可随意并发写不同元素。

## 目录检索试验

运行 `python tools/catalog.py validate` 校验字段、能力声明、文件链接及依赖关系。运行 `python tools/catalog.py query examples/queries/offline_connectivity.json` 查看候选和排除理由。

格式 v1 位于 `contracts/*.json`：每份包含稳定 ID、类别、头文件、文档、问题 ID、参数、必要事实、历史/并发能力、输入输出、约束、成本、保守内存载荷下界、关联和证据。当前仅支持相等、one_of、subset_of 等有限谓词，并非完整 JSON Schema 验证器。

检索先匹配单实现，再匹配已登记配方；不进行新组合搜索。缺字段返回 needs_information；无法验证的额外要求不会静默忽略；在线删边返回 not_covered，并列出排除理由。

内存载荷下界超过预算可以确定拒绝；下界未超预算不能证明峰值内存满足。无已校准性能模型，max_time_ms 也不能得到完成时间保证。故候选始终明确 `resource_status: unknown`；调用者仍需确认文档中的数学、生命周期、数值与分配前提。

这些状态检验了接口和元数据能协同工作，但还未实现完整自动证明、任意问题归约或自动组合规划。
