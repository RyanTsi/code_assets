# 工程资料

这里集中维护库的建设规则、来源审计、验证证据和应用评估；算法正文统一从 [资产文档目录](../README.md) 进入。

| 文档 | 保留用途 |
| --- | --- |
| [设计约定](DESIGN.md) | 代码、契约、数值与组合的设计边界 |
| [建设计划](ROADMAP.md) | 已完成工作、当前缺口和下一步 |
| [目录检索](CATALOG.md) | 查询方式、排除理由和能力范围 |
| [几何公共约定](GEOMETRY2D.md) | 多个几何资产共同依赖的坐标、精度、容差与来源说明 |
| [关系图维护](GRAPH.md) | 离线图谱的生成、关系语义与交互边界 |
| [来源载入台账](REPOSITORY_IMPORT.md) | 固定版本下116个来源文件的取舍、审核范围与资产映射 |
| [工程历史与早期验证](HISTORY.md) | 合并初始化与首批验证事实，保留未通过或未完成的检查 |
| [几何验证与基准](GEOMETRY_VALIDATION.md) | 数值边界、独立对照与实际基准的适用范围 |
| [音频应用评估](AUDIO_SPECTRUM_ASSESSMENT.md) | 一次从问题拆解、摘要检索到消费端验证的完整案例 |

来源台账由 `python tools/repository_import.py --report` 从 [机器台账](../../provenance/repository-assets/decisions.json) 生成；机器台账是处理状态的依据。正式判题证据与源码快照继续保留在 [judge/](../../judge/VALIDATION.md)，原始来源继续保留在 `provenance/`。

原 FIRST_ASSETS、REPOSITORY_ASSETS、REPOSITORY_EXPANSION 和 PROBLEM_COMBINATIONS 批次导航已经移除。它们重复的算法列表由唯一的资产目录替代，独有的检索说明、候选边界与来源记录已归入这里。
