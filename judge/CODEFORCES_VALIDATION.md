# Codeforces 组合验证

2026-10-14，验证已有资产能否完成真实题目的归约、求解与答案恢复。当前只补测 CF510E，不能据此声称博客中的所有 CF 题目都已测试。

[CF510E — Fox And Dinner](https://codeforces.com/problemset/problem/510/E) 的题目入口来自 `RyanTsi/blog` 固定提交 `6ea0ef09d8479c8775654c1627710af6da1d5392` 的 `content/posts/网络流做题记录.md`。适配器独立编写，复用 `PrimeSieve` 和 `Dinic`；题目专用年龄/I/O 逻辑留在 `judge/adapters/codeforces_510e.cpp`，没有因此增加竞赛专用公共 API。

## 输入、归约与正确性

输入限官方条件：3 ≤ n ≤ 200，2 ≤ age ≤ 10000。人物按位置区分，允许相同年龄；输出编号转换为 1-based。输出 `Impossible` 或覆盖所有人物恰好一次的环集合，每环至少三人，相邻及首尾年龄和都须是素数。适配器依赖合法完整输入；它不是任意整数或不完整输入的通用解析器。

所有年龄和 ≥ 4，排除偶素数 2，因此合法边一定连接奇数年龄与偶数年龄。环交替经过两组，组数不同立即无解。组数相等时建立：源到奇数容量 2，奇数到合法偶数邻居容量 1，偶数到汇容量 2。库使用整数容量；最大流为 n 当且仅当每个人恰有两个不同邻居。

合法环集合给出 n 单位可行流，证明必要性。反向取流量为 1 的候选边，每点度数为 2；每对人物只建一条边，不存在重边或自环。这是简单二分图，各连通分量为至少四点的环，证明充分性并满足题目的最少三人要求。利用两个邻居的定长数组，记住上一点即可线性还原每个环，不使用递归或逐点动态邻接数组。

## 整体资源与状态

令 M = 2 max(age) ≤ 20000，q 为合法奇偶候选边数，q ≤ n²/4。筛表时间 O(M log log M)，奇偶配对枚举 O(n²)，网络 n+2 点、n+q 条原始边。使用 Dinic 的一般最坏界 O(n²(n+q))，没有把容量 2 的网络误标为单位网络；恢复扫描 O(n+q)，输出 O(n)。这是保守理论界，不是平台耗时预测。

筛表 payload 为 `8 * ceil((floor(M/2)+1)/64)` 字节；流图已有实现每原始边两条 16-byte 残量弧，向量容量与扩容峰值还需单独考虑。其它流工作区与人物、两邻居、标记、输出序列和环边界均 O(n)。没有复制候选边清单、稠密邻接矩阵或逐人 vector。整体额外空间 O(M/8 + n + q) 字节量级，实际筛表只存奇数位；未测独立进程峰值内存，不能用 payload 代替实测峰值。

素数查询只读；最大流会改变内部残量网络，`edge(id)` 恢复选中边。该适配器每次新建图且只调用一次最大流，故 `FlowResult.added` 就是总流量；若将来复用同一图连续调用，不能将 added 当总量。库的 reset 能力不等于任意局部 undo/redo，本题也不需要持久化。

## 可复现本地验证

`tests/codeforces_510e_test.py` 用试除判素数，以一般兼容图的子集路径枚举找出所有可行环，再做子集覆盖；对照程序不使用奇偶分组、Dinic 或匹配。输出校验器核对环长、编号、唯一覆盖、每条边和首尾连接，也用故意损坏的答案检验校验器能否拒绝。

- 4 个官方样例（按合法性比较，允许输出不同环）。
- 年龄 2..5 的所有 256 个长度 4 输入。
- 固定随机种子 510 的 240 个 3..9 人输入。
- 7 组定向边界：度数不足、重复年龄、奇数人数、最大 n 的稠密/无边图、接近最大年龄及 200 人连续年龄。

全部 507 组通过 Release 与 GCC 15.2.0 MinGW Debug (`-D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -Wall -Wextra -Wpedantic -Werror`)。未声明其它编译器或 sanitizer 覆盖。

```sh
python tools/bundle_submission.py
cmake -S . -B build -DALGORITHM_ASSETS_BUILD_TESTS=ON
cmake --build build --target judge_cf510e
ctest --test-dir build -R 'codeforces_510e|judge_bundles' --output-on-failure
```

生成程序 `judge/generated/codeforces_510e.cpp`，manifest 保存两份公共头文件与适配器 SHA-256。正式提交 [394575207](https://codeforces.com/problemset/submission/510/394575207)，语言 GNU G++20 13.2 (64 bit, winlibs)，提交时在普通 textarea 中核对内容与生成文件完全一致。官方最终结果 **Accepted**，报告 62 ms、28 KB；这些数字只描述平台测量，不能视为全负载最优或独立实测的进程峰值内存。

提交源码 SHA-256：`d0396b3681da4ec843d22d279b07d0066e272a4d2297209835ae4d0e113b4780`。冻结文件 `judge/history/CF394575207.cpp` 与同名 JSON 保存该版本；正式结果进入 `judge/results.json`，截图 `judge/evidence/cf510e-ac.jpg`。本轮全部 13 组 CTest 通过。以后修改公共头文件或适配器，新生成版本不得自动继承此 AC。
