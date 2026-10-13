# AlgorithmAssets

逐步建设的现代 C++ 算法资产库。核心交付是可直接 `#include` 的代码，辅以明确契约、可运行示例、正确性依据与资源成本；人类可以阅读、理解和组合这些实现。

## 当前状态

初始化版本 `0.1.0`。已有 C++20 header-only 工程、独立 Git 仓库、公共头文件、CMake 接入与安装支持、调用示例及建设计划。**尚未收录算法实现，也尚未实现自动检索与组合规划。**

## 接入已有项目

不使用 CMake 时，把本库的 `include/` 加入头文件搜索路径，并启用 C++20：

```cpp
#include <algorithm_assets/algorithm_assets.hpp>

// 当前初始化版本仅提供版本信息；后续算法将有独立头文件。
auto library_version = algorithm_assets::version;
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

## 构建示例与安装

以下命令使用环境中可用的默认 CMake 生成器：

```sh
cmake -S . -B build -DALGORITHM_ASSETS_BUILD_EXAMPLES=ON
cmake --build build --config Release
cmake --install build --config Release --prefix install
```

Windows 下若使用 MinGW，可在配置时指定 `-G "MinGW Makefiles"`。示例目标为 `algorithm_assets_include_example`；可执行文件位置取决于生成器。

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
contracts/                 算法契约与能力目录，后续建设
recipes/                   经验证的组合配方，后续建设
tests/                     正确性与整体组合测试，后续建设
benchmarks/                性能与空间评测，后续建设
gaps/                      无匹配问题与能力缺口，后续建设
docs/                      设计约定、规划与初始化记录
```

后续优先 include 各算法的独立头文件，便利总头文件不替代模块边界。大型外部依赖只由相应模块显式引入。

参见 [设计约定](docs/DESIGN.md)、[建设计划](docs/ROADMAP.md) 和 [初始化记录](docs/INITIALIZATION.md)。原 OI Wiki 在工作区的 `../oi-wiki-reference/`，默认作为参考材料。
