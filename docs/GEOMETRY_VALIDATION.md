# 二维几何验证记录

验证以实际可调用实现为对象，不将模板改名或增加元数据视为验收。2026-10-14 于 Windows x64 / GCC 15.2.0 完成以下检查：

- Release 构建和 CTest **5/5**：原算法、几何、契约校验、目录匹配、关系图生成。
- 所有 **19 个公共头文件**独立包含编译，几何使用两个翻译单元链接验证 ODR。
- 几何 Debug 使用 `-O0 -g -D_GLIBCXX_ASSERTIONS -D_GLIBCXX_DEBUG -Wall -Wextra -Wpedantic -Werror` 编译运行。
- 两个外部消费工程分别使用源码和安装包，各自运行原算法和几何测试 **2/2**；不是只检查能 include。
- 示例实际运行：凸包 4 点、直径平方 25、矩形面积 12、重复点最近平方 0、裁剪面积 6、圆交点及切点数符合预期。
- 契约匹配 **8 项测试**，图生成 **6 项测试**；查询能找到两种点集配方，精确浮点凸包需求返回 `not_covered`。
- 本地浏览器实际查看新增节点、整数/浮点依赖条件、一跳邻居和旋转卡壳源码；无控制台错误。图谱已同步至 33 个资产、43 对关联。

几何覆盖 700 个固定随机点集（0..54 点）与 O(n²) 最近/最远点对、全边全点矩形枚举对照；每组附 20 个定位查询，与普通多边形定位对照。另有 1,000 组整数/浮点线段相交对照、400 组圆交点/切线的几何约束、重合/同心/相切/点圆/空与共线集合、±10⁹ 整数边界、浮点缩放和移至 10¹² 的面积相消回归。

新增凸包检查包含大重复输入/共线输入的常驻容量限制，最近点对显式 release 后重新求解。原模板的点到线段递归、EPS 排序、直线 C=0 退化、矩形输出未分配与共线卡壳等已被改写并覆盖关键回归。

本机 GCC 安装未提供可链接的 ASan/UBSan 库，未运行 sanitizer；没有形式化证明，也未进行 MSVC/Clang 或任意精确浮点坐标的跨平台认证。Debug STL 检查与随机对照不能替代这些验证。

```sh
cmake -S . -B build -G "MinGW Makefiles" -DALGORITHM_ASSETS_BUILD_TESTS=ON -DALGORITHM_ASSETS_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
python tools/verify_integration.py --generator "MinGW Makefiles"
python tools/build_graph.py
```

## 可复现基准

`benchmarks/geometry_benchmark.cpp` 保留用户最近点对的“复制输入、inplace_merge、条带 hypot”结构作为比较基线，只替换原模板不满足严格弱序的 EPS 排序。它不是原始模板所有函数的正确性证明，也不是外部高性能几何库比较。

```sh
g++ -std=c++20 -O3 -DNDEBUG -Wall -Wextra -Wpedantic -Werror -I include benchmarks/geometry_benchmark.cpp -o build/geometry_benchmark.exe
build/geometry_benchmark.exe
```

固定 seed=20261014，均匀随机点（[-10⁶,10⁶]）、共线点、含重复点三种分布；n=5,000/50,000/100,000。每项运行 5 次取中位数；同时报告新建工作区和预热后复用工作区。复用容量只统计向量载荷，**不是进程峰值内存**。CPU 为 Intel Core Ultra 7 265K，Windows x64，MinGW GCC 15.2.0；常规浮点参数，无 fast-math。

原始 CSV 保存为 `benchmarks/geometry_results.csv`。微基准有调度和缓存噪声，未构成稳定置信区间；不能据此承诺所有点集都更快。首次实现的随机点结果没有稳定提速，双缓冲交替归并进一步消除每层整段复制后重新测量，保留这个限制。

| 10 万点分布 | 模板式基线 ms | 新建工作区 ms | 复用工作区 ms | 基线/冷/复用分配次数 | 工作区容量载荷 |
|---|---:|---:|---:|---|---:|
| 均匀随机 | 21.6665 | 20.8351 | 21.0404 | 100001 / 2 / 0 | 3,200,000 B |
| 共线 | 8.3365 | 7.3693 | 5.7563 | 100001 / 2 / 0 | 3,200,000 B |
| 含重复点 | 23.1636 | 10.9174 | 9.6305 | 100001 / 1 / 0 | 1,600,000 B |

分配计数通过基准进程的 replaceable `operator new`（含 nothrow）统计，计数运行与计时运行分开；计时运行关闭计数，但替代分配入口仍存在，因此比较只适用于这个基准环境。原 `inplace_merge` 在这套 STL 中会在递归层申请临时缓冲，分配次数很多。新实现正常冷调用仅两个点缓冲，复用无新增分配。重复检测位于第二缓冲分配之前，故首次重复输入只需 16n 载荷。

随机点提速小，部分早期测量的冷调用甚至稍慢；长双精度中间计算和初始双缓冲复制均有成本。最明确的收益是消除隐式反复分配、固定归并空间、复用工作区，以及共线/重复输入上的改善。未测真实峰值内存，未比较 CGAL 等成熟库，也未声称最优实现。
