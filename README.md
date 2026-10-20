# AlgorithmAssets

逐步建设的现代 C++ 算法资产库。核心交付是可直接 `#include` 的代码，辅以明确契约、可运行示例、正确性依据与资源成本；人类可以阅读、理解和组合这些实现。

自有代码与文档采用 [MIT](LICENSE)，欢迎商业使用与社区贡献。Eigen等第三方内容保留原许可，范围见 [第三方声明](THIRD_PARTY_NOTICES.md)。欢迎从一个Bug、反例或真实问题开始参与，见 [贡献指南](CONTRIBUTING.md)。

## 当前状态

版本 `0.1.0` 当前有 91 项登记资产、64 个头文件，覆盖图与连通性、序列与区间、字符串、整数代数、数值计算、背包、二维几何与三维网格。

从 [算法文档目录](docs/README.md) 选择算法、数据表示或组合配方；每项均有独立正文、结构化契约、可编译示例和测试。整数与浮点等变体分别说明。

目录工具能匹配已登记的实现/配方并报告排除理由；**还没有任意自然语言检索、新组合搜索或已校准的时延/峰值内存模型**。用法见 [目录检索](docs/project/CATALOG.md)。

[工程资料](docs/project/README.md) 集中维护设计约定、计划、验证与应用案例。个人博客和笔记的116个来源文件、审核范围与取舍见 [来源台账](docs/project/REPOSITORY_IMPORT.md)；正式判题与可复现单文件见 [判题证据](judge/VALIDATION.md)。

自有代码统一使用4个空格缩进，格式检查运行 `python tools/format_code.py`，自动修复加 `--write`。工具安装、范围与生成产物维护见 [格式规范](docs/project/FORMATTING.md)。

三维首版提供存储/校验、几何/BVH、Eigen求解、有限编辑与undo/redo、渲染/OBJ、参数曲面与四面体交换。通过 `algorithm_assets::eigen` 显式启用本地固定Eigen，原核心目标不增加依赖。运行与边界见 [三维集成说明](docs/project/MESH3D_INTEGRATION.md)。

## 交互式关系图

真实音频问题的摘要检索与组合验证见 [AUDIO_SPECTRUM_ASSESSMENT.md](docs/project/AUDIO_SPECTRUM_ASSESSMENT.md)：复用 FFT 与补偿求和，提供可编译的 PCM 流频谱消费端 `examples/audio_spectrum.cpp`。它验证数值核心与分块处理，尚未登记为生产音频资产，也未接入录音设备。

打开 [visualization/index.html](visualization/index.html) 可查看类似 Obsidian 的本地关系图：节点拖拽、画布平移/缩放、搜索、关系筛选、一跳邻居聚焦，以及契约和实际源码阅读。页面自包含，可在浏览器中离线打开，无需安装前端依赖。

修改契约、头文件或界面后重新生成：

```sh
python tools/build_graph.py
python tests/graph_test.py
```

图谱自动读取 `contracts/*.json`，目前有 91 个资产、102 对关联。每对节点的多个关系合并为一条线；悬停连线或选中节点可读具体类型、方向及条件。算法、数据表示、组合配方使用不同形状，孤立节点不会被隐藏。图谱帮助浏览已有关系，组合可行性仍须检查完整契约。实现说明见 [关系图维护](docs/project/GRAPH.md)。

## 接入已有项目

不使用 CMake 时，把本库的 `include/` 加入头文件搜索路径，并启用 C++20：

```cpp
#include <algorithm_assets/algorithm_assets.hpp>

algorithm_assets::DisjointSet components(10);
components.merge(0, 1);
bool connected = components.same(0, 1);
```

使用 CMake 源码接入：

```cmake
add_subdirectory(path/to/algorithm-assets)
target_link_libraries(your_target PRIVATE algorithm_assets::algorithm_assets)
```

安装后接入：

```cmake
find_package(AlgorithmAssets 0.1 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE algorithm_assets::algorithm_assets)
```

配置 `CMAKE_PREFIX_PATH` 指向安装目录即可找到包。接口目标会传递头文件路径与 C++20 要求；当前无需链接二进制库。

## 二维几何接入

几何可以单独接入，无需把整个几何模块引入原便利总头文件：

```cpp
#include <algorithm_assets/geometry/geometry2d.hpp>
#include <cstdint>
#include <vector>

namespace g = algorithm_assets::geometry;
std::vector<g::IntegerPoint> points{{0, 0}, {4, 0}, {0, 3}};
auto hull = g::convex_hull<std::int64_t>(points);
auto farthest = g::convex_diameter(hull); // 平方距离 25，整数精确
```

`IntegerPoint` 限 ±10⁹；浮点输入限有限 ±10¹⁰⁰，浮点谓词与交点构造仍为近似。完整调用示例 `examples/geometry2d.cpp` 可编译运行。需要更小包含范围时 include 各独立头文件。

## 构建示例与安装

以下命令使用环境中可用的默认 CMake 生成器：

```sh
cmake -S . -B build -DALGORITHM_ASSETS_BUILD_EXAMPLES=ON
cmake --build build --config Release
cmake --install build --config Release --prefix install
```

Windows 下若使用 MinGW，可在配置时指定 `-G "MinGW Makefiles"`。示例目标为 `algorithm_assets_include_library`、`algorithm_assets_first_algorithms`、`algorithm_assets_geometry2d`、`algorithm_assets_repository_assets` 和 `algorithm_assets_repository_expansion`；可执行文件位置取决于生成器。

## 首批算法与验证

| 资产 | 独立头文件（相对 include/） | 状态能力 |
| --- | --- | --- |
| 普通并查集 | algorithm_assets/connectivity/disjoint_set.hpp | 增量合并；查询会压缩路径 |
| 可回滚并查集 | algorithm_assets/connectivity/rollback_disjoint_set.hpp | 祖先 checkpoint、rollback、撤销最近成功合并 |
| 线段树 | algorithm_assets/range/segment_tree.hpp | 单点赋值、有序区间聚合；不要求交换律 |
| CSR 图存储 | algorithm_assets/graph/csr_graph.hpp | 不可变、非负整数权有向图 |
| Dijkstra | algorithm_assets/graph/dijkstra.hpp | 精确整数距离、路径恢复、溢出检测 |
| 补偿求和 | algorithm_assets/numeric/compensated_sum.hpp | 流式近似求和；非有限输入与溢出拒绝 |
| 离线连通性 | algorithm_assets/recipes/offline_connectivity.hpp | 批量加边/删边/查询，支持重边计数 |

开启测试需要 Python 3.10+（仅目录工具使用；C++ 库和示例没有 Python 依赖）：

```sh
cmake -S . -B build -DALGORITHM_ASSETS_BUILD_TESTS=ON -DALGORITHM_ASSETS_BUILD_EXAMPLES=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python tools/catalog.py validate
python tools/catalog.py query examples/queries/offline_connectivity.json
python tools/catalog.py query examples/queries/online_deletion_gap.json
python tools/catalog.py query examples/queries/geometry_diameter_recipe.json
python tools/catalog.py query examples/queries/geometry_exact_float_gap.json
python tools/verify_integration.py
```

后一个查询会报告当前库未覆盖在线删边，不能把离线配方当作在线算法。目录结果只验证已登记条件；资源状态明确保留 unknown。保守内存载荷下界只能用于拒绝明显超预算的方案，不能证明方案满足峰值内存和时间限制。

`tests/algorithms_test.cpp` 与朴素划分、逐项区间计算、Floyd 和逐时刻 BFS 对照，并覆盖边界、回滚序列、非交换聚合、数值取消及溢出。CMake 同时独立编译公共头文件并将多个翻译单元链接到测试程序。

`tests/geometry_test.cpp` 对照 O(n²) 最近/最远点对和逐边全扫描矩形，检查相交/切线性质及退化输入。几何查询能区分无序点集的配方与已有凸包的单算法；精确浮点需求明确返回未覆盖。可选 `ALGORITHM_ASSETS_BUILD_BENCHMARKS=ON` 构建 `algorithm_assets_geometry_benchmark`，当前仅测最近点对。

`verify_integration.py` 使用已配置的 build 目录安装头文件，并让两个独立消费工程通过源码和安装包运行实际算法测试。Windows MinGW 环境附加 `--generator "MinGW Makefiles"`。安装包分发 C++ 接口；目录工具和契约当前从源码工程使用。

## 核心目标

1. 给定具体问题，优先精确匹配满足全部约束的单算法实现。
2. 单算法不适用时，优先复用已有组合配方，再评估允许的组合与时空成本。
3. 每个实现明确输入输出、前置条件、精度、资源、状态及 undo/redo 等能力。
4. 通过有类型的关联组织替代、依赖、归约、组合与不兼容关系。
5. 公共契约支持领域扩展，未来接入 ML、优化与几何算法。
6. 实现、说明和示例面向人类阅读，解释关键不变量和设计取舍。
7. 当前库不能满足问题时，先明确报告能力缺口和搜索范围，再考虑新增资产。

效率与空间利用率是硬性设计关注点。只有在限定输入、操作负载和计算模型并有依据时，才宣称最优；其余记录理论界、存储成本和实测结果。

## 目录

```text
include/algorithm_assets/   可直接调用的公共 C++ 头文件
examples/                  可编译的最小使用示例
cmake/                     安装包配置
contracts/                 91 份结构化资产契约（含实现变体与配方）
recipes/                   组合说明；实现位于公共头文件中
tests/                     正确性、整体组合与目录匹配测试
benchmarks/                最近点对时间/分配/容量基准；峰值内存后续建设
gaps/                      无匹配问题与能力缺口记录
tools/                     最小目录校验与匹配工具
visualization/             自包含关系图及其 HTML/CSS/JS 源文件
docs/                      算法文档目录与独立资产正文
docs/project/              集中的工程约定、来源台账、验证与应用记录
provenance/                几何模板原文与固定提交的仓库文件清单
judge/                     题目适配器、生成单文件、源码哈希与判题证据
```

后续优先 include 各算法的独立头文件，便利总头文件不替代模块边界。大型外部依赖只由相应模块显式引入。

参见 [设计约定](docs/project/DESIGN.md)、[建设计划](docs/project/ROADMAP.md) 和 [初始化记录](docs/project/HISTORY.md#初始化记录)。原 OI Wiki 在工作区的 `../oi-wiki-reference/`，默认作为参考材料。
