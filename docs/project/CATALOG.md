# 目录检索与能力边界

本页说明检索工具如何使用以及结果的含义。具体算法均从 [独立文档目录](../README.md) 进入。

## 目录检索

运行 `python tools/catalog.py validate` 校验字段、能力声明、文件链接及依赖关系。运行 `python tools/catalog.py query examples/queries/offline_connectivity.json` 查看候选和排除理由。

格式 v1 位于 `contracts/*.json`：每份包含稳定 ID、类别、头文件、文档、问题 ID、参数、必要事实、历史/并发能力、输入输出、约束、成本、保守内存载荷下界、关联和证据。当前仅支持相等、one_of、subset_of 等有限谓词，并非完整 JSON Schema 验证器。

检索先匹配单实现，再匹配已登记配方；不进行新组合搜索。缺字段返回 needs_information；无法验证的额外要求不会静默忽略；在线删边返回 not_covered，并列出排除理由。

内存载荷下界超过预算可以确定拒绝；下界未超预算不能证明峰值内存满足。无已校准性能模型，max_time_ms 也不能得到完成时间保证。故候选始终明确 `resource_status: unknown`；调用者仍需确认文档中的数学、生命周期、数值与分配前提。

这些状态检验了接口和元数据能协同工作，但还未实现完整自动证明、任意问题归约或自动组合规划。
