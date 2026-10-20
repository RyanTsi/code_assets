# 网格 CSR 邻接与索引半边视图

资产 ID：`mesh.topology`

类型：representation；问题：`surface_mesh_adjacency`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_topology.json) | [公共头文件](../../include/algorithm_assets/mesh/topology.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/topology.hpp>
```

`SurfaceTopology(mesh)` 构造 edges、corner_face/corner_edge/next/twin、edge_offsets/edge_corners、vertex_offsets/vertex_corners。`incident_corners(e)` 返回借用视图，依赖 topology 对象存活。

## 输入与输出

输入：合法 SurfaceMesh；边身份按无向端点对归并，不表示同端点的多条独立拓扑边，也不表示无面的独立线边。

输出：拥有的连续邻接数组；单个边关联恰好两个角点时填 twin，其他为 invalid_index；twin 本身不证明两侧绕向相反。

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

构建 O(n+C log(C+1))；edge incidence span O(1)，遍历 O(边度数)。

O(n+C+E)；排序边记录、构建游标与最终数组可能同时存活，输入不计入 owned 空间。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

排序规范端点对后相等记录恰好组成一个边组，CSR覆盖每个角点一次；顶点计数前缀和将角点分发到对应顶点，面内next按偏移闭合。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [连续表面网格与面角属性](mesh.surface.md)（`mesh.surface`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
