"""Missing sources, wrong snapshots and invented assets cannot close the ledger."""

import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import repository_import


class ImportLedgerTests(unittest.TestCase):
    def test_real_ledger(self):
        self.assertEqual(len(repository_import.validate()["decisions"]), 116)

    def test_missing_duplicate_unpinned_and_fake_assets(self):
        original = repository_import.validate()
        mutations = [
            lambda x: x["decisions"].pop(),
            lambda x: x["decisions"].append(copy.deepcopy(x["decisions"][0])),
            lambda x: x["decisions"][0].update(blob_sha="0" * 40),
            lambda x: x["decisions"][0].update(assets=["invented.placeholder"]),
        ]
        for mutate in mutations:
            candidate = copy.deepcopy(original)
            mutate(candidate)
            with self.assertRaises(ValueError):
                repository_import.validate(candidate)


if __name__ == "__main__":
    unittest.main()
