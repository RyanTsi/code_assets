# 三维点到三角形最近点

资产 ID：`geometry3d.closest_point_triangle`

类型：algorithm；问题：`point_triangle_closest`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_geometry3d_closest_point_triangle.json) | [公共头文件](../../include/algorithm_assets/geometry3d/primitives.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry3d/primitives.hpp>
```

`geometry3d::closest_point_on_triangle(p,a,b,c)` 返回 TriangleClosestPoint；包含 point、三个 barycentric 权重和 distance_squared。退化三角形退回三条闭线段，包括零线段；并列任取。

## 输入与输出

输入：四个有限且绝对值≤1e50的 Point3；退化允许。

输出：拥有的最近点、重心坐标和平方距离；全部为近似值，不返回原始输入索引。

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

O(1)。

O(1)，固定值和数组，无堆分配。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

按顶点、边和面内部的 Voronoi 区域分类，分别投影到相应闭区域；所有区域覆盖三角形。零面积时三角形退化到其线段并集，比较三条线段候选。非有限或未解决的数值条件抛异常，不给认证误差界。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [三维点、向量与包围盒](../representations/geometry3d.primitives.md)（`geometry3d.primitives`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- 本工程独立实现；总体分发许可证尚未选择。
