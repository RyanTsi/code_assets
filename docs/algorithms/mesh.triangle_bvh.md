# 快照三角网格 BVH 查询与缓存

资产 ID：`mesh.triangle_bvh`

类型：algorithm；问题：`mesh_spatial_queries`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_triangle_bvh.json) | [公共头文件](../../include/algorithm_assets/mesh/bvh.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/bvh.hpp>
```

`TriangleBvh(snapshot,leaf_size=4)`；closest_point(p)、raycast(origin,direction,max_distance=inf)、matches/require_current。`MeshQueryCache::get(snapshot)` 在版本不匹配时重建；返回引用仅在下一次重建前有效。

## 输入与输出

输入：非空mesh指针，允许0个三角形；1≤leaf_size≤64，F<UINT32_MAX/2。query点按坐标域校验；射线方向有限非零，将归一化，max_distance非NaN且≥0。

输出：optional最近面/点/重心/平方距离，或最近射线命中/归一化方向下的距离；结果带revision。无命中返回nullopt。共面/近乎平行射线可被容差分支忽略，不是精确射线谓词。

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

纯只读对象可共享；构建、流I/O、缓存重建和可变缓冲不得在同一实例上并发。普通值复制不是历史撤销。返回span/reference的生命周期以所属对象为准。

可移动类型在移动后的源对象仅保证可销毁或重新赋值；借用视图不能跨所属对象的销毁、赋值、移动或成功修改继续使用。

## 时间与空间

中位数分割建树平均 O(F log F)，nth_element不承诺无条件最坏线性选择。单查询最坏O(F)，平衡树DFS工作栈O(log(F+1))。

至多2F-1个64字节节点及4F排序索引容量，构建栈O(logF)；源mesh共享，不复制。缓存单写者，独立不可变BVH可并发查询。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

每个节点包围其全部叶三角形；距离下界超过当前最优即剪枝，叶上调用闭三角形投影。射线先做盒slab测试再做三角相交；快照持有源对象，版本检查不允许把旧结构当成当前结构。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [连续表面网格与面角属性](../representations/mesh.surface.md)（`mesh.surface`）：实际使用的类型、算法或配方。
- [三维点到三角形最近点](geometry3d.closest_point_triangle.md)（`geometry3d.closest_point_triangle`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
