# 三角网格面积与法向

资产 ID：`mesh.surface_geometry`

类型：algorithm；问题：`triangle_mesh_geometry`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_surface_geometry.json) | [公共头文件](../../include/algorithm_assets/mesh/geometry.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/geometry.hpp>
```

`triangle_geometry(mesh,face)` 返回一个三角形的面积与单位面法向；`surface_geometry(mesh)` 返回全部面面积/法向、面积加权顶点法向与 total_area。

## 输入与输出

输入：有限坐标三角网格。零面积三角形抛 domain_error；多边形抛 invalid_argument。孤立顶点或相消的顶点法向返回零向量。

输出：拥有数组；总面积通过已有 CompensatedSum 累加。顶点法向是面积加权，不是角度加权，也不是角点分裂法向。

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

单面 O(1)；全部 O(n+m)。

全部输出约8m面积+24m面法向+24n顶点法向，另外O(1)工作区。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

叉积长度的一半为面积，叉积方向给出绕向法向。每点累加 incident 面的面积向量后归一化；补偿求和改善总面积累计，但不是精确或统一误差界。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [连续表面网格与面角属性](../representations/mesh.surface.md)（`mesh.surface`）：实际使用的类型、算法或配方。
- [Neumaier compensated sum](numeric.compensated_sum.md)（`numeric.compensated_sum`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- 本工程独立实现；总体分发许可证尚未选择。
