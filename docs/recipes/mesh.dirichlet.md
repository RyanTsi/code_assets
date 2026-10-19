# 固定约束的可复用 Eigen Dirichlet 求解

资产 ID：`mesh.dirichlet`

类型：recipe；问题：`mesh_dirichlet_solve`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_dirichlet.json) | [公共头文件](../../include/algorithm_assets/backends/eigen/mesh_operators.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/backends/eigen/mesh_operators.hpp>
```

`DirichletSystem(mesh,fixed_indices)` 组装并分解约化系统；`solve(fixed_values,load={},tolerance=1e-9)` 可重复用于同一几何和固定顶点集合，返回 values、relative_residual、free_dofs。矩阵对象不可复制。

CMake 使用 `target_link_libraries(app PRIVATE algorithm_assets::eigen)`。此目标附带固定 Eigen 3.4.0 include 与 EIGEN_MPL2_ONLY/EIGEN_DONT_PARALLELIZE；不得在同一程序中混用其他不兼容配置。

## 输入与输出

输入：每个连通分量至少一个固定顶点，包括孤立顶点。重复/越界固定点拒绝。load为空表示零装配载荷，非空须长n且有限；固定值长度与固定索引一致。tolerance有限且0<t≤1e-2。

输出：解 K_ff x_f=b_f-K_fp x_p；固定点上的load不参与自由方程。不是自动将连续右端函数积分成载荷，也不保证近奇异系统的前向误差。空网格返回空，全部固定时不分解。

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

装配同余切模块；复用分解。稀疏LDLT取决于填充，最坏时间 O(n^3)、空间 O(n^2)；每次solve另有约化右端/回代成本。

拥有完整K、质量、约化矩阵、约束映射和Eigen因子；solve另分配 O(n) 输出与自由向量。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

固定值消元保持自由方程等价；并查集检查各分量有锚点。约化刚度在满足数值条件时正定，LDLT求解，并检查正主对角及相对残差。几何改变后必须重建系统。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [Eigen 余切刚度与重心质量矩阵](../algorithms/mesh.cotangent.md)（`mesh.cotangent`）：实际使用的类型、算法或配方。
- [DisjointSet](../algorithms/connectivity.dsu.md)（`connectivity.dsu`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- [Eigen固定来源及逐文件哈希](../../provenance/dependencies/eigen-3.4.0.json)；原始许可证位于third_party/eigen3/COPYING.*，不得抹去或另行授权第三方代码。
- 本工程独立实现；总体分发许可证尚未选择。
