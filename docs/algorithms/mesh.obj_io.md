# 保留面角与逻辑 ID 的 OBJ 交换

资产 ID：`mesh.obj_io`

类型：algorithm；问题：`mesh_obj_exchange`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_obj_io.json) | [公共头文件](../../include/algorithm_assets/mesh/obj_io.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/obj_io.hpp>
```

`read_obj(istream)` 返回 ObjImport（mesh、材质名称表、忽略记录种类）；`write_obj(ostream,mesh)` 输出17位double精度、按角点的UV/法向，以及aa_vid/aa_fid/aa_unit扩展注释。

## 输入与输出

输入：支持v、两维vt、vn、单环f、usemtl；支持负索引，不支持正向引用。UV/normal必须全角点完整或全部缺省。o/g/s/mtllib的忽略种类会返回；其他不支持记录拒绝。

输出：拥有的网格和材质标签；不加载MTL、纹理或场景。写入用aa_material_ID保留数值ID，名称/外观不保证往返。扩展ID和unit只在本读写器之间保证，普通OBJ软件可忽略注释。

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

与文本大小和构造校验相关；材质表O(log材质数)查找，ID哈希期望线性。

O(n+C+m+材质数+最长行)；writer流式输出，无完整文件副本。I/O失败可能已经输出部分文本。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

OBJ每角点的独立位置/纹理/法向索引转换到Corner域，输出为每角点属性索引，保留接缝。完成解析后再构造SurfaceMesh校验索引、属性和ID。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [连续表面网格与面角属性](../representations/mesh.surface.md)（`mesh.surface`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- **许可状态**：项目自有实现采用 [MIT](../../LICENSE)；第三方与来源材料保留原条款，范围见 [许可说明](../project/LICENSING.md)。
