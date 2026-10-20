# Eigen 余切刚度与重心质量矩阵

资产 ID：`mesh.cotangent`

类型：algorithm；问题：`triangle_mesh_operators`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_cotangent.json) | [公共头文件](../../include/algorithm_assets/backends/eigen/mesh_operators.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/backends/eigen/mesh_operators.hpp>
```

`cotangent_operators(mesh)` 返回 CotangentOperators：Eigen::SparseMatrix<double> stiffness 和 lumped_mass。必须链接 `algorithm_assets::eigen`，或手动添加 vendored Eigen include 路径和一致编译宏。

CMake 使用 `target_link_libraries(app PRIVATE algorithm_assets::eigen)`。此目标附带固定 Eigen 3.4.0 include 与 EIGEN_MPL2_ONLY/EIGEN_DONT_PARALLELIZE；不得在同一程序中混用其他不兼容配置。

## 输入与输出

输入：非零面积三角形；n≤INT_MAX、m≤floor(INT_MAX/12)，仍受内存限制。允许钝角，个别余切边权可以为负。

输出：K 的非对角为负余切权、对角为行和的相反数，即有限元正半定符号；它与某些库的负半定 Laplacian 相反。mass 是每三角形面积/3贡献的对角质量，不是 Voronoi 质量。

## 前置条件与约束

- 所有数学输出为近似 binary64，或明确转换的float；本批没有精确浮点谓词、自交认证或统一误差上界。禁止 fast-math 与破坏浮点语义的重结合。
- 输入在借用期间有效且不被并发改写。数量同时受索引位宽、size_t、容器 max_size 和可用内存限制。
- 非法形状/索引/域/ID以及数值失败抛标准异常，分配失败正常传播；具体边界以上述接口约定为准。

## 状态与生命周期

- 持久化：不支持
- 只读快照：不支持
- 检查点：不支持
- 回滚到checkpoint：不支持
- 撤销：不支持
- 重做：不支持
- 任意删除：不支持
- 同实例并发写：不支持
- 并发读取/调用：有条件支持

自由函数的独立调用可并发，但不承诺同一个 Eigen factorization 对象的并行 solve；使用独立系统或外部串行化。同一物理网格位置改变后旧分解不能自动更新。

可移动类型在移动后的源对象仅保证可销毁或重新赋值；借用视图不能跨所属对象的销毁、赋值、移动或成功修改继续使用。

## 时间与空间

生成12m个triplet并由Eigen聚合，构造路径 O(n+m)；浮点误差和后端布局另计。

O(n+m)，包含质量、triplet与Eigen聚合工作区；输入借用，返回矩阵拥有存储。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

每个角的 cot/2 贡献到对边两点的2x2刚度块，等价于线性三角有限元的面积乘形函数梯度内积。面积/3的顶点质量累计保持总质量等于面积。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [连续表面网格与面角属性](../representations/mesh.surface.md)（`mesh.surface`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- [Eigen固定来源及逐文件哈希](../../provenance/dependencies/eigen-3.4.0.json)；原始许可证位于third_party/eigen3/COPYING.*，不得抹去或另行授权第三方代码。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
