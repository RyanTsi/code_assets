# 算法契约

已有 91 份 JSON v1 契约，包含算法、数据表示、实现变体与组合配方。字段包含入口、使用条件、历史与并发能力、数值约束、成本、关系及验证证据。

契约与对应公共头文件一同维护。`python tools/catalog.py validate` 从工程根校验必要字段、路径、能力状态、引用和依赖环；这是最小格式检查，不是完整 JSON Schema 验证器。匹配语义与边界见 `../docs/project/CATALOG.md`。

每份契约的 `documentation` 必须指向 `docs/algorithms/`、`docs/representations/` 或 `docs/recipes/` 中独占的一份 Markdown 正文，不能共用文件或指向章节。入口见 [文档目录](../docs/README.md)。`python tools/check_docs.py` 另校验目录覆盖、文档标识、内容结构与本地链接。

几何的整数/浮点实现各自记录 coordinates 和 result_guarantee，不能自动互换。可选 `display_name` 用于人类导航；关系可选 `condition` 为非空条件说明，当前只供阅读，不是可执行的通用组合规则。`uses` 的条件说明用于模板实际选择内核，避免把两种互斥坐标变体理解为同时依赖。具体几何条件见 `../docs/project/GEOMETRY2D.md`。
## 载入批次与扩展参数

第二批契约见 `docs/README.md`：71 个已实现资产中，包含实际 SCC/2-SAT、FFT/卷积、LU/马尔科夫链与凸形平移碰撞组合。来源 disposition 不是算法契约，单独登记于 `provenance/repository-assets/decisions.json`；deferred 不进入可调用资产目录。

`parameters` 允许声明领域的非负整数规模（例如 `capacity`、`transient_count`），资源下界因子必须属于本资产声明参数或 `value_bytes`。模型没有自动验证任意交叉条件或推导新组合的能力，完整运行时约束仍须阅读文档。时延与峰值内存可行性保持 unknown。
