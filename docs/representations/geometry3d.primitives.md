# 三维点、向量与包围盒

资产 ID：`geometry3d.primitives`

类型：representation；问题：`geometry3d_primitives`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_geometry3d_primitives.json) | [公共头文件](../../include/algorithm_assets/geometry3d/primitives.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/geometry3d/primitives.hpp>
```

`geometry3d::Point3`、`Vector3`、`Aabb3`；dot/cross/length/normalized、interpolate、oriented_volume6。位置与位移分型，Aabb3 默认空。基础算术为低层函数；高层查询和 Mesh 构造校验坐标，调用者自行使用低层算术时须保证有限中间值。

## 输入与输出

输入：有限 double 坐标，绝对值≤1e50；normalized 要求有限非零向量。低层 oriented_volume6 是浮点三重积，不是精确谓词。

输出：值类型点/向量、近似量与包围盒；Point3/Vector3 各24字节。

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

每次基础操作 O(1)，无堆分配。

O(1)；Aabb3 为两个 Point3。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

点与位移的合法算术由类型区分；点积、叉积与逐轴包围盒直接对应欧氏几何定义。归一化直接逐分量除长度，避免极小长度的倒数先溢出。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

当前未登记依赖；不据此推断不存在潜在组合。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- 本工程独立实现；总体分发许可证尚未选择。
