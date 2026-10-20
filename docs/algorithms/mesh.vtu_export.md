# 四面体与顶点标量场 VTU 导出

资产 ID：`mesh.vtu_export`

类型：algorithm；问题：`tetrahedral_vtu_export`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_vtu_export.json) | [公共头文件](../../include/algorithm_assets/mesh/vtu_io.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/vtu_io.hpp>
```

`write_vtu(ostream,tet,vertex_scalar={},field_name="value")`；输出VTK XML UnstructuredGrid，单元type=10、区域放CellData、可选标量放PointData。

## 输入与输出

输入：TetrahedralMesh；标量为空或长度n且有限；字段名须为有效文本且不含控制字符。

输出：流式ASCII XML，Float64点、UInt32连接、UInt64偏移；保留四面体而非导出表皮。字段是顶点采样标量，不冒充一般FE场或积分点应力。

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

O(n+m+字段值数)文本格式化。

O(字段名称长度)额外工作区，不缓存完整文件；IO失败可能部分写入。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

按VTK单元约定写4个连接、偏移4(i+1)、类型10，点/单元属性按各自域组织；字段名做XML转义，测试用独立XML解析器核对结构与体积。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [线性四面体网格与区域](../representations/mesh.tetrahedral.md)（`mesh.tetrahedral`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
