"""Regress page ownership, migrated document navigation, and broken anchors."""

import copy
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import catalog
import check_docs


class DocumentationTests(unittest.TestCase):
    def test_complete_migration_and_local_links(self):
        count, _ = check_docs.check()
        self.assertEqual(count, len(catalog.load_assets()))

    def test_batch_overview_cannot_return_to_docs_root(self):
        scratch = ROOT / ".cache"
        scratch.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=scratch) as directory:
            root = Path(directory).resolve()
            self.assertTrue(root.is_relative_to(scratch.resolve()))
            (root / "docs").mkdir()
            (root / "docs/README.md").write_text("# Index\n", encoding="utf-8")
            (root / "docs/batch_overview.md").write_text("# Duplicate index\n", encoding="utf-8")
            with patch.object(check_docs, "ROOT", root):
                with self.assertRaisesRegex(ValueError, "project material belongs"):
                    check_docs.check()

    def test_shared_page_is_rejected(self):
        assets = copy.deepcopy(catalog.load_assets())
        algorithms = [a for a in assets if a["kind"] == "algorithm"]
        algorithms[1]["documentation"] = algorithms[0]["documentation"]
        with self.assertRaisesRegex(ValueError, "already owned"):
            catalog.validate(assets)

    def test_overview_and_section_are_rejected(self):
        for path in ["docs/project/CATALOG.md", "docs/algorithms/numeric.radix2_fft.md#使用与设计"]:
            assets = copy.deepcopy(catalog.load_assets())
            assets[0]["documentation"] = path
            with self.subTest(path=path), self.assertRaises(ValueError):
                catalog.validate(assets)

    def test_broken_document_links_are_rejected(self):
        page = ROOT / "docs/README.md"
        for path in ["missing.md", "algorithms/numeric.radix2_fft.md#missing-anchor"]:
            with self.subTest(path=path), self.assertRaises(ValueError):
                check_docs.check_link(page, path)
        check_docs.check_link(page, "algorithms/numeric.radix2_fft.md#使用与设计")


if __name__ == "__main__":
    unittest.main()
