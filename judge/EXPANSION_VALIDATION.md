# 第二批载入验证

2026-10-14，C++20。所有提交由当前公共头文件生成；JSON 保存 SHA-256，适配器只处理题目 I/O。实际平台判题独立于本地对照验证；AC 不证明完整接口或数值精确性。

| 题目 | 覆盖的资产 | 状态 |
| --- | --- | --- |
| [P3376](https://www.luogu.com.cn/problem/P3376) | Dinic | [R303090337](https://www.luogu.com.cn/record/303090337)，Accepted 100 |
| [P4782](https://www.luogu.com.cn/problem/P4782) | TwoSat + CSR + 迭代 SCC | [R303090646](https://www.luogu.com.cn/record/303090646)，Accepted 100 |
| [P3386](https://www.luogu.com.cn/problem/P3386) | Hopcroft-Karp | [R303091719](https://www.luogu.com.cn/record/303091719)，Accepted 100 |
| [P3803](https://www.luogu.com.cn/problem/P3803) | FFT + 实卷积 | [R303092141](https://www.luogu.com.cn/record/303092141)，Accepted 100；有限整数系数由适配器取整，库不声称整数精确 |
| [P4557](https://www.luogu.com.cn/problem/P4557) | 凸包 + Minkowski 差 + 平移碰撞 | [R303092609](https://www.luogu.com.cn/record/303092609)，Accepted 100 |
| [P1742](https://www.luogu.com.cn/problem/P1742) | 最小圆覆盖 | [R303093055](https://www.luogu.com.cn/record/303093055)，Accepted 100；近似数值，题目误差门限1e-9 |
| [P3389](https://www.luogu.com.cn/problem/P3389) | DenseLu | [R303093550](https://www.luogu.com.cn/record/303093550)，Accepted 100；近似数值，No Solution 使用数值主元拒绝 |
| [P1776](https://www.luogu.com.cn/problem/P1776) | 单调队列多重背包 | [R303094555](https://www.luogu.com.cn/record/303094555)，Accepted 100 |

核心对照见 `tests/repository_expansion_test.cpp`：枚举割、匹配、布尔赋值，Floyd 负环可达与强连通闭包、父链 LCA、二次 DP、子集区间覆盖、DFT/朴素卷积、已知解矩阵、背包枚举、CRT/试除和 Minkowski 点对凸包、候选圆穷举。30000 点长链证明当前测试路径没有递归调用栈依赖。`tests/expansion_judge_test.py` 为8个生成程序另做边界与160组随机 stdin/stdout 对照。

没有为未入库的费用流、lazy、持久化、SVD、完整 ML 训练编造通过记录。完整来源处理状态见 `docs/REPOSITORY_IMPORT.md`。

本机 GCC 15.2.0 MinGW Release 以 `-Werror` 构建：12组 CTest 全过，47个公共头文件独立编译和多翻译单元链接通过；源码/安装包两个消费工程各4组实际算法测试通过。新批次在 Debug 的 `_GLIBCXX_DEBUG` / `_GLIBCXX_ASSERTIONS` 下通过，并强制启用 `ALGORITHM_ASSETS_FORCE_PORTABLE_MODULAR` 测试无128位扩展的模乘路径。未在其它编译器、ABI 或 BLAS/GPU 后端测量性能；没有可用 MinGW ASan/UBSan 链接库，不声称 sanitizer 通过。

`judge/results.json` 关联本批8次及首批4次 AC 与源码 SHA-256；`judge/history/R*.cpp` 和相邻 manifest 固定已提交版本，未来重新生成当前 bundle 不覆盖历史证据。AC 只适用于对应快照，未提交的新版本不得继承当前版本 AC 结论。截图包含各题明细及 `evidence/expansion-ac-summary.jpg` 汇总。
