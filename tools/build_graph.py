"""Generate a standalone, offline graph from the validated asset catalog."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import catalog

ROOT = Path(__file__).resolve().parents[1]
LABELS = {
    "connectivity.dsu": "并查集",
    "connectivity.rollback_dsu": "可回滚并查集",
    "connectivity.offline": "离线动态连通性",
    "range.segment_tree": "线段树",
    "graph.csr": "CSR 图存储",
    "graph.dijkstra": "Dijkstra",
    "numeric.compensated_sum": "补偿求和",
    "numeric.wide_int64_sum": "双字精确累加器",
    "range.fenwick_sum": "树状数组求和",
    "strings.kmp": "KMP 字节搜索",
    "algebra.xor_basis64": "GF(2) 线性基",
    "graph.kruskal": "Kruskal 最小生成森林",
    "graph.topological_sort": "拓扑排序",
    "sequence.inversions": "归并逆序对",
    "sequence.inversions_fenwick": "树状数组逆序对配方",
}


def build_data(assets: list[dict]) -> dict:
    catalog.validate(assets)
    nodes = []
    edges = {}
    for asset in sorted(assets, key=lambda item: item["id"]):
        node = dict(asset)
        node["label"] = asset.get("display_name", LABELS.get(asset["id"], asset["name"]))
        node["source"] = catalog.local_file(asset["header"]).read_text(encoding="utf-8")
        nodes.append(node)
        for relation in asset["relations"]:
            source, target = asset["id"], relation["target"]
            pair = tuple(sorted((source, target)))
            edge = edges.setdefault(pair, {"source": pair[0], "target": pair[1], "relations": []})
            # Alternatives are symmetric; directional declarations remain distinct.
            if relation["type"] == "alternative":
                source, target = pair
            declaration = {"source": source, "target": target, "type": relation["type"]}
            if "condition" in relation:
                declaration["condition"] = relation["condition"]
            if declaration not in edge["relations"]:
                edge["relations"].append(declaration)
    return {"nodes": nodes, "edges": [edges[pair] for pair in sorted(edges)]}


def render(data: dict) -> str:
    directory = ROOT / "visualization"
    payload = json.dumps(data, ensure_ascii=False, separators=(",", ":"))
    # JSON lives in an HTML script element, where even a quoted closing tag ends it.
    payload = payload.replace("&", "\\u0026").replace("<", "\\u003c").replace(">", "\\u003e")
    template = (directory / "template.html").read_text(encoding="utf-8")
    replacements = {
        "/* GRAPH_STYLE */": (directory / "graph.css").read_text(encoding="utf-8"),
        "/* GRAPH_SCRIPT */": (directory / "graph.js").read_text(encoding="utf-8"),
        "GRAPH_DATA": payload,
    }
    # Single pass: asset source text must never be treated as a template token.
    import re

    return re.sub(
        "|".join(re.escape(key) for key in replacements),
        lambda match: replacements[match.group()],
        template,
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "visualization" / "index.html")
    args = parser.parse_args()
    data = build_data(catalog.load_assets())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render(data), encoding="utf-8")
    print(
        f"Generated {args.output}: {len(data['nodes'])} assets, {len(data['edges'])} linked pairs"
    )


if __name__ == "__main__":
    main()
