# 连续表面网格与面角属性

资产 ID：`mesh.surface`

类型：representation；问题：`surface_mesh_storage`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_surface.json) | [公共头文件](../../include/algorithm_assets/mesh/surface_mesh.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/surface_mesh.hpp>
```

`mesh::SurfaceData` 按值传给 `SurfaceMesh`；`from_triangles(positions,faces)` 构造三角网格。访问 positions()/face(i)/triangle(i)/data()、vertex_count/face_count/corner_count、all_triangles。SurfaceData 中 face_offsets 初始为 {0}，每面至少3角点。

## 输入与输出

输入：坐标、单环面偏移与角点顶点索引；可选完整 Corner UV/normal；每 Face 一个材质编号。顶点/面逻辑 ID 可省略后自动从1编号；提供时须非零唯一。n、m、总角点数 C 均<UINT32_MAX；length_unit 非空且无控制字符。

输出：不可变、拥有全部数组的 SurfaceMesh。只读 span/reference 在对象销毁、赋值或移动后失效；复制是完整值复制。空网格合法。重复坐标或几何退化不在结构构造阶段自动修复。

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

结构检查 O(n+C+m)；ID 唯一性使用哈希，期望线性，碰撞最坏可二次。

坐标24n、角点4C、偏移8(m+1)、ID 8(n+m)、材质4m；UV16C、normal24C可选，capacity/容器/构造临时哈希另计。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

偏移覆盖全部角点且逐面递增至少3，所有角点索引落在顶点域，属性长度与域一致，ID唯一。所有可变数组封装在只读对象内；构造失败不产生可用半成品。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [三维点、向量与包围盒](geometry3d.primitives.md)（`geometry3d.primitives`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
