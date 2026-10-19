# 网格快照、版本句柄与有界撤销

资产 ID：`mesh.document`

类型：representation；问题：`mesh_snapshot_history`

[文档目录](../README.md) | [结构化契约](../../contracts/mesh3d_mesh_document.json) | [公共头文件](../../include/algorithm_assets/mesh/document.hpp) | [可编译示例](../../examples/mesh3d.cpp)

## 使用与设计

```cpp
#include <algorithm_assets/mesh/document.hpp>
```

`MeshDocument(mesh_or_snapshot,history_limit=32)`；snapshot()、vertex_handle/face_handle、resolve、set_position、commit(base,replacement)、undo/redo。快照是 shared_ptr<const SurfaceMesh> 与单调revision。

## 输入与输出

输入：结构有效网格；导入的最大逻辑ID须小于UINT64_MAX以保留下一ID。commit要求base完全匹配当前指针和revision；新ID不能复用已退休编号。

输出：快照O(1)共享不可变网格；每次发布、undo、redo均产生新revision，所以所有旧运行期句柄失效，即使实体仍存在。逻辑ID在保留实体/撤销恢复时保留；未公开每实体generation语义。

## 前置条件与约束

- 所有数学输出为近似 binary64，或明确转换的float；本批没有精确浮点谓词、自交认证或统一误差上界。禁止 fast-math 与破坏浮点语义的重结合。
- 输入在借用期间有效且不被并发改写。数量同时受索引位宽、size_t、容器 max_size 和可用内存限制。
- 非法形状/索引/域/ID以及数值失败抛标准异常，分配失败正常传播；具体边界以上述接口约定为准。

## 状态与生命周期

- 持久化：有条件支持
- 只读快照：支持
- 检查点：不支持
- 回滚到checkpoint：不支持
- 撤销：支持
- 重做：支持
- 任意删除：有条件支持
- 同实例并发写：不支持
- 并发读取/调用：有条件支持

只读快照保留旧版本；同一个文档不能同时读写。先取得的不可变快照可在其他线程只读使用。全量快照历史与结构共享的持久化数据结构不同；任意删除仅能由合法整体replacement提交，不提供逐元素delete API。

可移动类型在移动后的源对象仅保证可销毁或重新赋值；借用视图不能跨所属对象的销毁、赋值、移动或成功修改继续使用。

## 时间与空间

snapshot/句柄/resolve O(1)；提交按值复制或转移完整网格并做期望线性ID检查，清理历史可能释放多个快照；undo/redo引用切换O(1)加容器/释放成本。

历史最多H个undo状态，但外部保留快照不受H限制。最坏O((H+外部活跃版本)*S)；本版不是分块持久化结构。

内存下界只可用于排除明显超预算，不能认证峰值或时限。实际保留容量、返回值、临时数组、共享源和用户保留的历史版本要合并计账。

## 正确性依据

新状态构造和可能失败的历史分配发生在发布之前，提交冲突不改变当前对象。undo/redo切换已验证不可变对象，revision永不倒退；退休ID计数不随undo回退。

这是实现的不变量/归约说明，不是形式化证明；测试与基准的适用范围分开记录。

## 关联资产

- [连续表面网格与面角属性](mesh.surface.md)（`mesh.surface`）：实际使用的类型、算法或配方。

组合前仍需核对数值保证、单位、属性域、状态与资源，而不是只比较类型。

## 验证与来源

- [C++ 对照与状态测试](../../tests/mesh3d_test.cpp)，多翻译单元接入。
- [独立 FEM/SciPy/VTU 对照](../../tests/mesh3d_reference_test.py)（需NumPy/SciPy）。
- [本批实现与验证边界](../project/MESH3D_INTEGRATION.md)。
- 本工程独立实现；总体分发许可证尚未选择。
