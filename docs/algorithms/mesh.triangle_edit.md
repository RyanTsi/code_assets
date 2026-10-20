# 事务式三角网格 split、flip、collapse

资产 ID：`mesh.triangle_edit`

类型：algorithm；问题：`triangle_mesh_edit`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_triangle_edit.json) | [公共头文件](../../include/algorithm_assets/mesh/edit.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/edit.hpp>
```

`MeshEditSession(document)`；preview()、split_edge(edge,t=0.5)、flip_edge(edge)、collapse_edge(edge,t=0.5)、commit()。edge编号从当前preview重建的 SurfaceTopology 获取，每次成功编辑后必须重新获取。

## 输入与输出

输入：初始网格须 editable_triangles。split 0<t<1可处理边界；flip仅内部边，拒绝已有新对角线、材质边界、UV接缝与翻面。collapse仅内部边且端点不在边界，检查link条件并保守拒绝材质/UV特征。

输出：候选经全网格组合校验后替换会话草稿；失败不改变草稿。commit才发布并写入文档历史，冲突/已提交会话拒绝。法向清空以避免旧数据；UV按每侧插值、材质继承、保留面的ID不变。

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

MeshDocument必须在整个会话期间存活，会话借用它而不拥有它。会话仅单线程修改；preview引用随成功修改/会话销毁失效。失败操作不改变草稿，文档在commit前不变。撤销通过MeshDocument完成，算子自身不保存另一份历史。

可移动类型在移动后的源对象仅保证可销毁或重新赋值；借用视图不能跨所属对象的销毁、赋值、移动或成功修改继续使用。

## 时间与空间

当前每操作复制/压缩并重建邻接与校验，三角网格 O(n+m log(m+1)) 典型路径；不是O(1)局部操作。commit也可能复制草稿。

O(n+m)候选、邻接与临时数据；外加原文档与历史。没有全局无自交或质量界认证。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

使用索引半边视图找局部关联，split保留两侧顺序，flip替换对角线，collapse先验证link并检查保留面方向。所有算子在独立候选中修改，组合验证通过后才替换草稿。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [网格快照、版本句柄与有界撤销](../representations/mesh.document.md)（`mesh.document`）：实际使用的类型、算法或配方。
- [表面网格能力校验与边界环](mesh.validation.md)（`mesh.validation`）：实际使用的类型、算法或配方。
- [网格 CSR 邻接与索引半边视图](../representations/mesh.topology.md)（`mesh.topology`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
