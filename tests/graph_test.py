"""Verify graph fidelity and safe standalone serialization."""

import copy
import json
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import build_graph
import catalog


class GraphTests(unittest.TestCase):
    def setUp(self):
        self.assets = catalog.load_assets()

    def test_exact_assets_and_directional_relations(self):
        data = build_graph.build_data(self.assets)
        self.assertEqual({node["id"] for node in data["nodes"]}, {a["id"] for a in self.assets})
        expected = set()
        for asset in self.assets:
            for relation in asset["relations"]:
                a, b = asset["id"], relation["target"]
                if relation["type"] == "alternative":
                    a, b = sorted((a, b))
                expected.add((a, b, relation["type"]))
        actual = {(r["source"], r["target"], r["type"]) for e in data["edges"] for r in e["relations"]}
        self.assertEqual(expected, actual)
        self.assertEqual(len(data["edges"]), len({tuple(sorted((a, b))) for a, b, _ in expected}))
        for node in data["nodes"]:
            self.assertEqual(node["source"], catalog.local_file(node["header"]).read_text(encoding="utf-8"))

    def test_isolated_assets_and_future_labels(self):
        assets = copy.deepcopy(self.assets)
        for asset in assets:
            asset["relations"] = []
        assets[0]["id"] = "future.domain.asset"
        data = build_graph.build_data(assets)
        self.assertEqual(len(data["nodes"]), len(assets))
        self.assertEqual(data["edges"], [])
        future = next(n for n in data["nodes"] if n["id"] == "future.domain.asset")
        self.assertEqual(future["label"], future["name"])

    def test_invalid_relation_fails_before_generation(self):
        self.assets[0]["relations"].append({"type": "uses", "target": "missing.asset"})
        with self.assertRaises(ValueError):
            build_graph.build_data(self.assets)

    def test_conditional_relations_and_display_names(self):
        self.assets[0]["display_name"] = "Readable display name"
        self.assets[0]["relations"] = [{"type": "uses", "target": self.assets[1]["id"], "condition": "Checked for this coordinate kernel only"}]
        data = build_graph.build_data(self.assets)
        self.assertEqual(next(n for n in data["nodes"] if n["id"] == self.assets[0]["id"])["label"], "Readable display name")
        declaration = next(r for edge in data["edges"] for r in edge["relations"]
                           if r["source"] == self.assets[0]["id"] and r["type"] == "uses")
        self.assertEqual(declaration["condition"], "Checked for this coordinate kernel only")
        self.assets[0]["relations"][0]["condition"] = {}
        with self.assertRaises(ValueError):
            build_graph.build_data(self.assets)
        self.assets[0]["relations"][0]["condition"] = "Valid condition"
        self.assets[0]["display_name"] = 42
        with self.assertRaises(ValueError):
            build_graph.build_data(self.assets)

    def test_safe_standalone_html_roundtrip(self):
        data = build_graph.build_data(self.assets)
        data["nodes"][0]["source"] += '\n</script><script>alert("unsafe")</script> GRAPH_DATA /* GRAPH_SCRIPT */ &'
        html = build_graph.render(data)
        payload = re.search(r'<script id="catalog-data" type="application/json">(.*?)</script>', html, re.S).group(1)
        self.assertNotIn('<', payload)
        self.assertEqual(json.loads(payload), data)
        self.assertEqual(html.count('<script'), 2)
        self.assertNotIn('src="http', html)
        self.assertNotIn('href="http', html)
        self.assertNotIn('/* GRAPH_STYLE */', html)

    def test_delivered_page_matches_current_catalog(self):
        expected = build_graph.render(build_graph.build_data(self.assets))
        self.assertEqual((ROOT / "visualization/index.html").read_text(encoding="utf-8"), expected,
                         "Regenerate with python tools/build_graph.py")


if __name__ == "__main__":
    unittest.main()
