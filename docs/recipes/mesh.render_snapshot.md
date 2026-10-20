# 保留接缝与挑选映射的渲染快照

资产 ID：`mesh.render_snapshot`

类型：recipe；问题：`mesh_render_buffers`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_render_snapshot.json) | [公共头文件](../../include/algorithm_assets/mesh/render.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/render.hpp>
```

`make_render_snapshot(snapshot)` 返回 origin、float positions/normals/uv、indices、每绘制角点的source_corners、每三角形的source_faces/materials及revision。

## 输入与输出

输入：非退化三角形；提供corner_normals时必须可归一化，否则使用平面法向；UV可缺省为0。局部坐标或属性不能表示为float时拒绝。

输出：位置相对double原点；按几何顶点、float法向、float UV和材质元组哈希去重。每绘制索引仍保留原角点映射，避免共享GPU顶点后丢失挑选信息。

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

期望 O(n+C)，哈希碰撞最坏可O(C^2)；输出按原面角顺序确定。

最多C个GPU顶点，每顶点32字节；索引与角点映射8C，来源/材质12F，另有哈希工作区。不包含GPU上传或渲染器。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

只在所有参与绘制的属性相同且引用同一几何点时合并，故UV/硬法向/材质接缝保持；选择映射按索引出现位置记录，转换不改变权威网格。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [三角网格面积与法向](../algorithms/mesh.surface_geometry.md)（`mesh.surface_geometry`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
