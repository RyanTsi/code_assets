# 参数曲面采样与 CAD 来源绑定

资产 ID：`mesh.cad_patch`

类型：recipe；问题：`cad_patch_tessellation`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_cad_patch.json) | [公共头文件](../../include/algorithm_assets/mesh/cad_patch.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/cad_patch.hpp>
```

`sample_cad_patch(u0,u1,v0,v1,nu,nv,evaluate,body,face_id,cad_revision)`；evaluate(u,v)返回Point3。返回网格快照、顶点参数、来源面/体标识和源revision；matches(snapshot,cad_revision)检查绑定。

## 输入与输出

输入：严格递增有限参数区间，端点绝对值≤1e50；nu,nv≥1且顶点/角点计数适配32位；face_id非零。evaluate由调用者提供，不持有其生命周期。

输出：(nu+1)(nv+1)点、2nu*nv三角形，UV为原参数。曲面采样可产生退化或自交，需另调用validation；不保证弦高误差，也不处理修剪、缝合、STEP/B-rep。

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

O(nu*nv*evaluate_cost)，加网格构造检查。

O(nu*nv)拥有输出与来源/参数数组；绑定保留不可变网格指针，编辑产生新snapshot后不再匹配。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

按参数网格固定朝向连接每个小格的两个三角形，逐点保留(u,v)，每面关联源实体。绑定检查指针、网格revision和CAD revision，不把离散编辑误认为原曲面仍有效。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [连续表面网格与面角属性](../representations/mesh.surface.md)（`mesh.surface`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- 本工程独立实现；总体分发许可证尚未选择。
