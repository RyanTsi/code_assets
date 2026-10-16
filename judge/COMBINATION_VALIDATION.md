# 新组合题目验证

2026-10-15。源自个人博客中的三道题；当前题目条件只在各自适配器使用，通用配方与理论成本见 [配方文档目录](../docs/README.md#组合配方15)。

| 题目 | 新组合 | 生成单文件 | 官方状态 |
| --- | --- | --- | --- |
| [CF822C](https://codeforces.com/problemset/problem/822/C) | 起止扫描 + 长度压缩 + 最低费用 | `judge/generated/codeforces_822c.cpp` | [394593349](https://codeforces.com/problemset/submission/822/394593349)，Accepted，281 ms、7400 KB |
| [CF1822F](https://codeforces.com/problemset/problem/1822/F) | CSR + 树直径端点 + 三次 BFS | `judge/generated/codeforces_1822f.cpp` | [394593548](https://codeforces.com/problemset/submission/1822/394593548)，Accepted，171 ms、5600 KB |
| [ABL D](https://atcoder.jp/contests/abl/tasks/abl_d) | 压缩 + 范围最大值 DP | `judge/generated/atcoder_abl_d.cpp` | [80046831](https://atcoder.jp/contests/abl/submissions/80046831)，AC 400，172 ms、12524 KiB |

本地 `tests/problem_combinations_test.cpp` 每项 400 组随机对照，共 1200 组：区间两两枚举、O(n²) 原值差子序列 DP、Floyd 全点对树距离；还检查返回区间编号、INT64_MIN/MAX、真正最优费用溢出与非最佳候选溢出、空输入、重复值、非树拒绝及 30000 点长链。ODR 文件另行包含全部三头。

`tests/combination_judge_test.py` 对真正的生成单文件可执行程序做 496 个样例/随机/最大规模输入：CF822C 165，CF1822F 165，ABL D 166。最大输入分别为 200000 个区间、200000 点长链、300000 个序列元素。CF1822F 的大树另起一份输入，保持每次总 n<=200000。测试不把不合法输入当官方覆盖；库额外支持的负坐标、零费用等另在 C++ 契约测试验证。

```sh
python tools/bundle_submission.py
cmake -S . -B build -DALGORITHM_ASSETS_BUILD_TESTS=ON -DALGORITHM_ASSETS_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
python tools/build_graph.py
python tests/graph_test.py
```

所有正式提交都需在结果页看到最终 verdict；生成 manifest 与 `judge/history/` 绑定确切源码，未来改动不得继承旧版本 AC。本批尚未补测其它 CF/AtCoder/Gym 题目，不将历史博客提交算作本次验证。

提交前核对普通 textarea 的完整源码与生成文件完全相同。CF 使用 GNU G++20 13.2 (64 bit, winlibs)，AtCoder 使用 C++23 (GCC 15.2.0)，库语法仍仅要求 C++20。正式耗时和内存为平台报告，不能视为独立实测峰值或最优性证明。两平台 KB/KiB 字段保留原单位。

冻结快照为 `judge/history/CF394593349.*`、`CF394593548.*`、`AT80046831.*`，每个 JSON 内有完整 SHA-256；`judge/results.json` 连接源码哈希、官方链接和截图。截图 `judge/evidence/combination-cf-ac.jpg` 包含两条新 CF AC，`abl-d-ac.jpg` 包含本次 AtCoder AC。所有 16 个历史提交源码与 manifest 哈希均已核对。

本轮 GCC 15.2.0 MinGW Release：15/15 CTest 通过，50 个公共头文件独立包含检查通过，组合测试通过多翻译单元链接。新增三配方在 Debug 的 `_GLIBCXX_DEBUG`、`_GLIBCXX_ASSERTIONS` 和 `-Werror` 下亦通过。源码和安装两个独立消费工程分别 5/5 测试通过，包含新增实际组合测试。未声称 sanitizer、其它 ABI 或性能基准通过。
