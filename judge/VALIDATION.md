# 提交与验证记录

2026-10-14：库与单文件均为 C++20。`judge/adapters/` 仅转换题目 I/O；`generated/` 由真实库头文件展开而成，不维护另一套算法。相邻 JSON 记录每个源文件与最终提交的 SHA-256；修改公共头文件后重新打包并验证。

2026-10-19格式统一后，`generated/` 已从4空格缩进的当前代码重新生成，源码哈希相应改变。下列线上AC仅绑定 `history/` 中的原始提交及其哈希；新生成版本需要重新提交才能获得对应版本的线上结论。本次执行本地回归，不将历史AC自动转移到新版本。

```sh
python tools/bundle_submission.py
python tools/bundle_submission.py --check
cmake --build build
ctest --test-dir build --output-on-failure
```

生成器去重项目头文件，保留标准头；遇到条件项目 include 或路径越界会拒绝。当前适配器使用的头文件没有这种结构。未来扩展预处理器能力需单独验证，不把此脚本当通用 C++ 预处理器。

| 题目 | 资产 | 平台验证 |
| --- | --- | --- |
| [P3375 KMP](https://www.luogu.com.cn/problem/P3375) | KmpPattern | [R303024143](https://www.luogu.com.cn/record/303024143)：Accepted，100，C++20 O2 |
| [P3374 树状数组 1](https://www.luogu.com.cn/problem/P3374) | FenwickSum | [R303025461](https://www.luogu.com.cn/record/303025461)：Accepted，100，C++20 O2 |
| [P3366 最小生成树](https://www.luogu.com.cn/problem/P3366) | Kruskal + DisjointSet | [R303026225](https://www.luogu.com.cn/record/303026225)：Accepted，100，C++20 O2 |
| [P3812 线性基](https://www.luogu.com.cn/problem/P3812) | XorBasis64 | [R303073023](https://www.luogu.com.cn/record/303073023)：Accepted，100，C++20 O2 |

已完成本地核心随机/穷举对照与边界验证；提交程序还通过样例及 100 组随机 stdin/stdout 对照，覆盖 1-based 到 0-based 转换。topological_sort、归并逆序对、Fenwick 逆序对组合与宽累加器的额外语义目前由本地测试验证。空模式、嵌入 NUL、负权森林、满 64 位空间、溢出拒绝等不由这些模板题充分覆盖。

本机 GCC 15.2.0 MinGW：Release 全工程带 `-Werror` 构建，8 组 CTest 全过；新批次在 `-O0 -g -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -Wall -Wextra -Wpedantic -Werror` 下另行通过；27 个公共头文件独立编译，多翻译单元链接通过。源码 `add_subdirectory` 和安装包 `find_package` 两个独立消费工程，各通过算法、几何、新资产三组实际测试。未测新资产的跨平台性能；当前 MinGW 不具备可用 ASan/UBSan 链接库，本批不声称通过 sanitizer。

线上题目的时间与内存只反映对应数据和评测环境，不能作为本库全负载的性能最优证明。界面截图保存在 `judge/evidence/`。
