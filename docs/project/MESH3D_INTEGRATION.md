# 三维网格与 Eigen：首版资产、运行与边界

日期：2026-10-19。实现覆盖五个阶段的可执行最小流程，具体入口见 [独立文档目录](../README.md)。本批新增17项资产、14个头文件（包含一个内部流格式工具）；Eigen仅通过单独目标引入，不改变原默认总头文件。

## 五阶段交付

| 阶段 | 已实现 | 明确边界 |
| --- | --- | --- |
| 1. 基础与存储 | Point3/Vector3/Aabb3、连续面角存储、UV/normal/材质与逻辑ID、结构和组合校验 | 单环面，核心算子以三角形为主；没有独立线边、多重拓扑边和任意属性注册 |
| 2. 查询与几何 | CSR邻接、索引半边视图、顶点link校验、边界环、面积/法向、三角形投影 | 没有全局自交检查或精确浮点谓词；零面积拒绝不代表近退化认证 |
| 3. 数值后端 | 固定Eigen 3.4.0、余切刚度K、重心质量、可复用的固定Dirichlet分解与载荷求解 | 每个连通分量需有约束；相对残差不是解的前向误差证书，不启用外部稀疏求解器 |
| 4. 编辑与历史 | split/flip/collapse、UV与材质特征保护、候选校验、冲突拒绝、只读快照与undo/redo | 每次编辑有复制/邻接重建/全组合校验成本；collapse限制内部且端点非边界，没有任意删除或全局无自交保证 |
| 5. 显示与跨域 | BVH最近点/射线与版本缓存、接缝感知渲染缓冲、OBJ往返、参数曲面来源绑定、四面体表皮及VTU顶点标量场 | 没有图形窗口、GPU上传、STEP/B-rep、带孔/修剪面、体网格生成和高阶有限元 |

## 构建与使用

```sh
cmake -S . -B build -DALGORITHM_ASSETS_BUILD_TESTS=ON -DALGORITHM_ASSETS_BUILD_EXAMPLES=ON
cmake --build build --target algorithm_assets_mesh3d algorithm_assets_mesh3d_tests
ctest --test-dir build -R 'mesh3d|eigen_vendor' --output-on-failure
build/algorithm_assets_mesh3d.exe build/mesh3d-example
```

消费工程通过以下方式接入，同样适用于安装后的 `find_package(AlgorithmAssets CONFIG REQUIRED)`：

```cmake
target_link_libraries(your_target PRIVATE algorithm_assets::eigen)
```

该目标传递本库和 vendored Eigen 的头文件路径及一致宏；原 `algorithm_assets::algorithm_assets` 继续只传递本库路径。无需运行时下载或系统Eigen安装。手动include时，需额外加入 `third_party/eigen3` 路径，并一致启用 `EIGEN_MPL2_ONLY` 和 `EIGEN_DONT_PARALLELIZE`。

```cpp
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <algorithm_assets/backends/eigen/mesh_operators.hpp>
```

具体构造、约束和求解的完整程序是 [examples/mesh3d.cpp](../../examples/mesh3d.cpp)。它生成参数平面网格、OBJ导出再导入，求解带边界值的标量场，执行一次编辑及undo/redo，再做BVH和渲染转换，并输出四面体与场。

生成物包括 surface.obj、stiffness.mtx、solution.csv、analysis.vtu 和 tet_boundary.obj。它们在用户指定输出目录中，不会被作为公共源码提交。没有依赖命令行I/O的核心算法耦合。

## 数据和数值约定

- 坐标为有限double，绝对值≤1e50。三维三重积和浮点构造是近似；不沿用二维整数±1e9的精确性声明。禁止fast-math。
- 角点域保留UV/normal，面域保留材质，顶点和面拥有非零唯一逻辑ID；坐标相等不自动合并顶点。单位是模型字符串，不自动转换。
- 校验区分边和顶点流形性、当前绕向、边界、退化和孤立点；重复面检测仅对三角形。存储允许非流形输入，编辑不接受它，校验没有顺便证明无自交。
- 余切矩阵采用正半定刚度K：非对角为负权，与一些库的负半定L相反；质量是重心lumped mass。右端load是装配后的顶点载荷，未自动乘质量矩阵。固定点上的载荷不参与自由方程。
- 稀疏LDLT的填充、时间和内存依赖网格。病态或非正主对角拒绝；残差测试不能认证条件数或真实前向误差。

## 生命周期、编辑与版本

SurfaceMesh不可变，快照共享完整旧对象；每次文档发布均增加revision，运行期句柄通过document、revision、逻辑ID和稠密索引做O(1)检查。首版采用粗粒度版本：任何提交、undo、redo都会使旧句柄失效，即便对应实体未删除。持久逻辑ID与运行期句柄不可混用。

编辑会话拥有一份草稿，并借用创建它的MeshDocument；文档必须存活至会话结束。一次算子在独立候选上修改、校验后才替换草稿，commit检查原快照仍为当前版本；分配失败和非法操作不会发布半更新文档。flip拒绝跨材质/UV接缝，collapse还要求内部边、非边界端点及link条件。法向在几何/拓扑修改后清空，避免旧属性继续被显示。没有通用属性插值器、质量最优保证或自交修复。

history_limit约束文档保留的undo长度，外部持有快照不受该限制。每个修改版本可以拥有完整数组，因此历史最坏为O(H*S)，不能把snapshot()的O(1)误解成编辑也只使用O(1)内存。新编辑清空redo；已退休逻辑ID不得被普通commit复用。undo恢复旧逻辑实体，但运行期revision仍增加。

MeshQueryCache按快照指针和revision重建BVH。直接持有的旧BVH可以继续查询旧快照，其结果带旧revision；需要当前版本时调用require_current。cache.get返回的引用仅在下次重建前有效。同一文档、编辑会话和缓存均为单写者；不可变快照/BVH可共享读取。Eigen分解对象的并发solve不在本版承诺范围。

## CAD、CAE 与格式

CAD采样接受调用者的矩形参数曲面eval(u,v)，每个顶点保存参数，每面绑定来源Face和Body版本。仅为规则采样，没有修剪环、缝合或弦高误差界。绑定在源CAD版本变化或网格快照变化后失效；示例是解析平面，不声称已经接入OCCT/CGAL。

线性四面体检查有效索引和正的近似体积。外表皮按共享面归并，保留源cell/local face及区域；共享面方向矛盾或超过两个cell拒绝。没有体单元重叠、自交、质量或全局共形认证。内部材料界面不导出为外表皮；高阶几何节点、FE自由度和积分点场仍然单独缺失。

VTU输出保持四面体和区域，字段明确是顶点标量采样。OBJ支持面角UV/normal的完整或全缺省形式，不加载MTL/纹理，忽略的o/g/s/mtllib种类返回给调用者；未支持的记录拒绝。ID和单位通过扩展注释往返，普通软件可以忽略它们。文件/流写入失败可能留有部分输出，调用者需要原子文件保存时应自行使用临时文件加替换。

## Eigen 来源与许可

[逐文件清单](../../provenance/dependencies/eigen-3.4.0.json) 固定官方3.4.0压缩包，SHA-256为 `8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72`。344个上游文件原样保留，约6.2MB；测试拒绝丢失、新增或改写。宏和默认模块只启用本批需要的MPL2兼容路径，不启用FFTW/SuiteSparse/METIS等外部后端。

集成方式与上游条款见 [第三方依赖说明](../../third_party/README.md)。上游每个文件及COPYING文本仍是许可依据；本工程没有将Eigen改成自有许可证。分发时仍需履行相应声明与源码提供义务，用户项目整体许可证尚未选择。

## 实际验证与基准

- C++覆盖结构坏输入、bow-tie、非流形边、边界环、已知面积/法向/刚度、仿射和常量场、分解复用、失败编辑原子性、30次编辑/撤销序列、逻辑ID不复用和冲突提交。
- 空间检查包含400个BVH与逐三角形投影对照、200条平面射线已知答案，以及接缝渲染、OBJ负索引/部分属性拒绝、来源失效、四面体向外法向。
- 独立Python以形函数梯度积分组装刚度，与C++的余切公式对照；再使用SciPy求解。81点/128个非均匀平面三角形的刚度最大绝对差约1.78e-15，求解相对残差约1.77e-17；这些是该测试的观察值，不是通用误差上界。XML解析核对VTU连接、偏移、类型、区域、顶点场和体积。NumPy/SciPy缺失时该额外测试不启用，C++测试仍保留。
- 新增三组测试与当前全量21组CTest已经通过；本批公共头文件独立包含及多翻译单元编译通过。源码和安装后的两个独立消费工程各6/6组通过；64个头文件独立包含、多翻译单元链接和实际三维调用均通过。Debug以 `-O0 -g -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -Wall -Wextra -Wpedantic -Werror` 运行通过；未进行ASan/UBSan、MSVC/Clang或GPU验证。

本机 Intel Core Ultra 7 265K / Windows x64、GCC 15.2.0、C++20、Release `-O3 -DNDEBUG` 基准见 [CSV](../../benchmarks/mesh3d_results.csv)，单位ms，单次观察，不是校准的预算模型。网格为规则参数面，查询1000点；下列容量为SurfaceData数组容量，不是进程峰值或引擎总内存：

| 三角形 | 数组容量bytes | 校验 | BVH构建 | 1000查询 | 组装 | split+提交 |
| --- | --- | --- | --- | --- | --- | --- |
| 2048 | 279048 | 0.4892 | 0.6878 | 1.5023 | 0.2884 | 1.55 |
| 18432 | 2262536 | 3.47 | 8.8073 | 4.6665 | 2.6786 | 18.6213 |
| 73728 | 9047048 | 15.0698 | 32.057 | 9.6172 | 12.0298 | 78.3103 |

这是可重复基线：局部编辑的整网格重建成本明显，后续应按实际负载决定增量拓扑、分块历史和更多索引宽度变体。没有测量实际进程峰值、GPU性能、硬实时或通用最优性。

本批源码、契约与验证事实的绑定见 [验证清单](../../provenance/mesh3d/validation.json)。
