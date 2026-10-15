# 正确性验证

音频消费端新增 `audio_consumer_composition`，验证分块不变性、窗加权能量、DC/Nyquist、幅度标定、非均匀/空 bin 频段、非法块与 reset。可选 `audio_scipy_reference` 通过实际 CLI 对照 SciPy periodogram，60组共240帧；缺少 NumPy/SciPy 时不启用该组。运行 `ctest --test-dir build -R 'audio_consumer_composition|audio_scipy_reference' --output-on-failure`。这两组验证不等于真实设备或硬实时验证，详情见 `docs/AUDIO_SPECTRUM_ASSESSMENT.md`。

`algorithms_test.cpp` 覆盖朴素对照、关键性质、边界输入、回滚和撤销序列、数值精度与整体配方。`catalog_test.py` 覆盖单算法/配方选择、条件不足、不支持的需求、错误领域、资源下界拒绝和元数据错误。

通过 CMake 开启 `ALGORITHM_ASSETS_BUILD_TESTS`，然后执行 CTest。公共头文件还会独立包含编译并进行多翻译单元链接。测试使用固定种子，可复现；它们不代替正确性论证或跨平台性能验证。

`geometry_test.cpp` + `geometry_odr.cpp` 覆盖几何退化/回归、整数边界、浮点尺度、随机点集的 O(n²) 最近/最远对照、矩形全边扫描对照、定位与交点/切线性质；源码和安装消费工程都会运行该测试。`graph_test.py` 校验目录到图谱的关系、条件、转义和生成产物一致性。

## 个人仓库资产补充验证

`repository_assets_test.cpp` 与 `repository_assets_odr.cpp` 验证 KMP、Fenwick、Kruskal、拓扑序、GF(2)、逆序对与实际 Fenwick 组合；包括随机朴素对照、穷举空间/森林、64 位边界、失败更新原子性和多翻译单元。`judge_adapter_test.py` 对生成后的四个真实可执行文件运行 stdin/stdout 样例及随机对照；`judge_bundles` 检查提交源码与当前公共头文件一致。真实线上判题独立记录在 `judge/VALIDATION.md`。

第二批 `repository_expansion_test.cpp` 与 `repository_expansion_odr.cpp` 验证图/约束、序列、数值、优化和几何；`expansion_judge_test.py` 对8个新提交程序做160组随机 I/O 对照。`import_ledger_test.py` 拒绝缺失来源、错误 SHA 和假资产。Release、Debug 容器检查及强制 portable 模乘路径分别运行；线上判题见 `judge/EXPANSION_VALIDATION.md`。

`codeforces_510e_test.py` 对素数筛 + Dinic + 环恢复的生成程序做 507 组检查，以一般兼容图的子集环覆盖为独立对照，并逐项验证特殊输出。包括官方样例、穷举、小规模随机和最大规模边界；线上记录见 `judge/CODEFORCES_VALIDATION.md`。它不代表其它 CF 题目已通过。

`problem_combinations_test.cpp` + `problem_combinations_odr.cpp` 验证三个实际配方：区间对、有限步差子序列和树最远距离，1200 组随机朴素对照，重要边界与 30000 点长链。`combination_judge_test.py` 对三个生成单文件做 496 个输入检查。源码/安装消费工程均新增实际组合测试，线上证据见 `judge/COMBINATION_VALIDATION.md`。
