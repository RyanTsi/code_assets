# 表面网格能力校验与边界环

资产 ID：`mesh.validation`

类型：algorithm；问题：`surface_mesh_validation`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_validation.json) | [公共头文件](../../include/algorithm_assets/mesh/topology.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/topology.hpp>
```

`validate_surface(mesh)` 返回 SurfaceValidation 的边界边、非流形边/顶点、方向冲突、退化、重复三角形与孤立顶点计数。`oriented_manifold()` 与 `editable_triangles()` 是不同门槛；`boundary_loops(mesh)` 返回顶点环，不重复首点。

## 输入与输出

输入：结构合法的 SurfaceMesh。允许开口、退化、孤立点和非流形输入以便诊断。

输出：结构/组合能力报告；重复面检测仅针对三角形按顶点集合判断。退化面积为浮点零测试，不认证近退化。没有自交、闭实体或全局嵌入合法性证书。

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
- 并发读取/调用：支持

纯只读对象可共享；构建、流I/O、缓存重建和可变缓冲不得在同一实例上并发。普通值复制不是历史撤销。返回span/reference的生命周期以所属对象为准。

可移动类型在移动后的源对象仅保证可销毁或重新赋值；借用视图不能跨所属对象的销毁、赋值、移动或成功修改继续使用。

## 时间与空间

三角形 O(n+C log(C+1))；一般多边形另有 O(sum(face_arity^2)) 顶点 link 遍历。边界环先校验再重建邻接。

O(n+C+m+E)，报告是固定大小；输出边界环另计。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

边关联数量与两侧起点检查边流形/当前方向；逐顶点检查关联面连通分量和边界度数，识别仅做边度数检查遗漏的 bow-tie。流形边界每个顶点只有唯一后继，所以可遍历闭环。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [网格 CSR 邻接与索引半边视图](../representations/mesh.topology.md)（`mesh.topology`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
