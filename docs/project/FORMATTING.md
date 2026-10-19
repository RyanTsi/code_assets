# 格式规范与维护

自有代码统一使用4个空格缩进、UTF-8、LF和文件末尾换行，不使用Tab缩进。默认行宽100列，按语法需要换行，不压缩非空函数或控制分支，不在一行中串接多个独立语句。C++指针和引用符号靠近类型；当前格式整理不批量改名、改变接口、增加控制分支花括号或调整算法。

## 范围与工具

| 范围 | 格式工具与配置 |
| --- | --- |
| C++公共头文件、示例、测试、基准、题目适配器 | clang-format 19.1.5；`.clang-format` |
| Python工具与测试 | Black 24.10.0；`pyproject.toml`；格式化同时检查AST等价性 |
| JavaScript、CSS、HTML模板、项目JSON | Prettier 3.6.2；`.prettierrc.json` |
| CMake源文件 | cmakelang 0.6.13；`.cmake-format.json` |
| Markdown中的C++示例 | 由同一clang-format处理代码围栏，正文不做批量重排 |
| 编辑器默认规则 | `.editorconfig` |

基本缩进为4空格；续行也可能按参数、运算符或括号对齐。长URL、不可拆分字符串以及原始字符串中的数据不强行截断。

以下内容保持其证据或上游边界：

- `third_party/`：原始Eigen头文件与许可证不改写，继续逐文件哈希验证。
- `provenance/`：来源快照、历史审核、基准与验证记录保留原始字节或既定生成格式。
- `judge/history/`：线上实际提交的源码与清单不改写，其AC仍绑定原始哈希。
- `judge/generated/`：从当前适配器与头文件重建，不直接格式化；JSON生成缩进为4空格。
- `visualization/index.html`：从格式化后的模板、CSS、JavaScript和当前契约重建，不直接格式化嵌入数据。结构化载荷紧凑编码，源码阅读内容保留实际头文件格式。
- 工作区的 `oi-wiki-reference/` 是另一个仓库，不属于本工程格式修改范围。

## 安装与使用

安装clang-format 19.1.5并加入PATH，或使用 `--clang-format` 指定可执行文件。Windows脚本也会查找Visual Studio附带的LLVM工具。

在工程根目录准备其余工具。缓存目录已被Git忽略，不加入运行时依赖：

```sh
python -m pip install black==24.10.0
python -m pip install --target .cache/format-python cmakelang==0.6.13
npm install --prefix .cache/format-node --no-audit --no-fund --ignore-scripts prettier@3.6.2
```

只检查、不修改文件：

```sh
python tools/format_code.py
```

应用格式并同步派生产物：

```sh
python tools/format_code.py --write
python tools/bundle_submission.py
python tools/build_graph.py
python tools/format_code.py
```

格式脚本只选择Git管理或尚未忽略的新代码文件，排除上述原始证据与生成目录。它在检查失败时返回非零退出码，可以直接用于提交前检查或CI。格式工具版本不符时拒绝运行，避免不同机器反复产生格式差异。

格式工具仅用于开发，不是库的编译或运行前置依赖。构建、完整CTest及源码/安装消费验证仍按README执行；格式检查不能代替功能测试。

## 本轮审查

2026-10-19，基线提交 `f6aa0b8`。发现C++既有配置为2空格，并存在单行多语句、紧缩函数、缺少运算符间距；Python、JavaScript与CSS存在相同的紧缩书写。整理对象覆盖自有代码、构建配置、项目JSON和文档示例，保留上述上游与历史边界。

首次三维验证清单与音频实验清单中的哈希继续指向各自历史版本；本轮字节变化可从Git格式差异追溯。题目生成源码与清单重新建立当前绑定，历史线上AC不自动继承到新生成版本。

本轮检查覆盖103个自有C++文件、18个Python文件、135个前端/JSON文件、2个CMake文件，以及93份文档中的C++示例。相对基线实际变更的100个C++文件经忽略空白和注释的词法序列对比一致，15个仅格式化的既有Python文件AST一致，130个既有项目JSON内容值一致。提交生成器另有明确的JSON输出缩进调整（2改4），新增格式脚本另行检查。

格式检查和重复格式化通过；完整Release构建及21/21项CTest通过。源码与安装包独立消费工程分别6/6项测试通过，同时覆盖64个头文件的独立包含与多翻译单元接入。JavaScript语法检查、生成图谱一致性和Eigen原始哈希校验通过。本轮未重新线上提交题目，也未重新测性能；词法/AST对比不是形式化证明。
