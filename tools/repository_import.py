"""Validate complete source dispositions and render the human-readable ledger."""
from collections import Counter
import argparse
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import catalog

def validate(ledger=None):
    assets=catalog.load_assets();catalog.validate(assets);ids={a['id'] for a in assets}
    expected={}
    for path in (ROOT/'provenance/repository-assets').glob('*inventory.json'):
        inventory=json.loads(path.read_text(encoding='utf-8'))
        for item in inventory['inventory']:expected[(inventory['repository'],item['path'])]=(inventory['commit'],item['sha'])
    if ledger is None:
        ledger=json.loads((ROOT/'provenance/repository-assets/decisions.json').read_text(encoding='utf-8'))
    seen=set()
    for row in ledger['decisions']:
        key=(row['repository'],row['path'])
        if key in seen or key not in expected or expected[key]!=(row['commit'],row['blob_sha']):raise ValueError(f'Unpinned, duplicate or unknown source: {key}')
        seen.add(key)
        if row['disposition'] not in {'imported','partial','deferred','excluded','reference_only','standard_library'}:raise ValueError('Invalid disposition')
        if row['review_scope'] not in {'full','sections','inventory_only'}:raise ValueError('Invalid review scope')
        if not row['reason'] or any(asset not in ids for asset in row['assets']):raise ValueError(f'Missing implementation/reason: {key}')
        if row['disposition'] in {'imported','partial'} and not row['assets']:raise ValueError(f'No actual asset: {key}')
        if row['disposition']=='partial' and not row['deferred_scopes']:raise ValueError(f'Missing partial boundary: {key}')
    if seen!=expected.keys():raise ValueError('Unprocessed source inventory entries')
    return ledger

def report(ledger):
    count=len(ledger['decisions'])
    rows=['# 个人仓库载入台账','',f'台账覆盖固定提交的 {count} 个 Markdown 文件。`full` 为完整候选正文审核，`sections` 为章节/代码结构审核，`inventory_only` 仅按目录判断范围；不声称所有正文都已逐字阅读。partial 和 deferred 都不是完整覆盖。','','每项资产的接口、数值/状态约束、空间成本和正确性说明见 [独立文档目录](../README.md)。每个映射 ID 都验证存在真实头文件、契约、测试与示例。源仓库无 LICENSE，本库整体许可证尚待选择。','','```sh','python tools/repository_import.py','python tools/repository_import.py --report','```','']
    counts=Counter(row['disposition'] for row in ledger['decisions'])
    rows+=['处理状态：'+ '；'.join(f'{k}={v}' for k,v in sorted(counts.items()))+'。','','| 来源路径 | 审核范围 | 状态 | 已有代码资产 / 处理理由 |','| --- | --- | --- | --- |']
    for row in ledger['decisions']:
        reason=', '.join(row['assets'])+'；'+row['reason']+'；'+'；'.join(row['deferred_scopes'])
        rows.append(f"| {row['repository']}: `{row['path']}` | {row['review_scope']} | {row['disposition']} | {reason.replace('|','/')} |")
    (ROOT/'docs/project/REPOSITORY_IMPORT.md').write_text('\n'.join(rows)+'\n',encoding='utf-8')

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',action='store_true')
    args=parser.parse_args()
    ledger=validate()
    if args.report:report(ledger)
    print(f"Validated {len(ledger['decisions'])} pinned source dispositions and actual asset mappings")
