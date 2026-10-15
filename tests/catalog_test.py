"""Test selection, rejection, resource uncertainty and the catalog's scope boundary."""

import copy
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("asset_catalog", ROOT / "tools/catalog.py")
catalog = importlib.util.module_from_spec(spec)
spec.loader.exec_module(catalog)


class CatalogTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assets = catalog.load_assets()
        catalog.validate(cls.assets)

    def query(self, **fields):
        return catalog.match(fields, self.assets)

    def connectivity(self, **extra):
        fields = dict(problem="connectivity", n=100, directed=False, rollback=False,
                      execution_mode="online", updates=["add", "query"])
        fields.update(extra)
        return self.query(**fields)

    def test_single_and_rollback(self):
        result = self.connectivity()
        self.assertEqual(result["status"], "found_algorithm")
        self.assertEqual(result["candidates"][0]["id"], "connectivity.dsu")
        self.assertEqual(self.connectivity(rollback=True)["candidates"][0]["id"],
                         "connectivity.rollback_dsu")

    def test_recipe_and_online_gap(self):
        fields = dict(updates=["add", "remove", "query"], q=200)
        offline = self.connectivity(execution_mode="offline", **fields)
        self.assertEqual(offline["status"], "found_recipe")
        online = self.connectivity(**fields)
        self.assertEqual(online["status"], "not_covered")
        self.assertTrue(online["evaluations"])
        self.assertIn("no novel composition", online["scope"])

    def test_problem_combinations_and_affine_gap(self):
        import json
        for name, expected in [('disjoint_interval_pair', 'sequence.disjoint_interval_pair'),
                               ('bounded_step_subsequence', 'sequence.bounded_step_subsequence'),
                               ('tree_eccentricities', 'graph.tree_eccentricities')]:
            fields = json.loads((ROOT / 'examples/queries' / (name + '.json')).read_text())
            result = self.query(**fields)
            self.assertEqual(result['status'], 'found_recipe')
            self.assertEqual([item['id'] for item in result['candidates']], [expected])
            if name == 'tree_eccentricities':
                fields['edge_lengths'] = 'unequal_positive'
            elif name == 'bounded_step_subsequence':
                fields['adjacent_difference'] = 'rank_distance'
            else:
                fields['pair_count'] = 3
            self.assertEqual(self.query(**fields)['status'], 'not_covered')
        self.assertEqual(self.query(problem='dynamic_affine_minimum', n=100000, q=100000)['status'], 'not_covered')

    def test_missing_and_unknown_requirement(self):
        self.assertEqual(self.query(problem="connectivity", n=100)["status"], "needs_information")
        self.assertEqual(self.connectivity(must_preserve_labels=True)["status"], "not_covered")
        self.assertEqual(self.connectivity(capabilities={"redo": "supported"})["status"],
                         "not_covered")
        self.assertEqual(self.connectivity(capabilities={"unregistered_property": "supported"})["status"],
                         "needs_information")

    def test_budget_is_not_a_fit_certificate(self):
        self.assertEqual(self.connectivity(max_memory_bytes=100)["status"], "budget_exceeded")
        result = self.connectivity(max_memory_bytes=1000000, max_time_ms=1)
        self.assertEqual(result["status"], "found_algorithm")
        self.assertEqual(result["resource_status"], "unknown")

    def test_wrong_domain_and_properties(self):
        self.assertEqual(self.query(problem="ml.classification")["status"], "not_covered")
        self.assertEqual(self.query(problem="shortest_paths", n=10, m=20,
                                   weights="negative_int64", graph_updates="none",
                                   sources="single", result_guarantee="exact")["status"], "not_covered")
        self.assertEqual(self.query(problem="range_fold", n=10, operation_associative=False,
                                   identity_available=True, updates=["set", "query"])["status"],
                         "not_covered")
        self.assertEqual(self.connectivity(directed=0)["status"], "not_covered")

    def test_invalid_metadata(self):
        duplicate = self.assets + [copy.deepcopy(self.assets[0])]
        with self.assertRaises(ValueError):
            catalog.validate(duplicate)
        broken = copy.deepcopy(self.assets)
        broken[0]["header"] = "../oi-wiki-reference/README.md"
        with self.assertRaises(ValueError):
            catalog.validate(broken)

    def test_saved_queries(self):
        import json
        expected = {
            "incremental_connectivity": ("found_algorithm", "connectivity.dsu"),
            "rollback_connectivity": ("found_algorithm", "connectivity.rollback_dsu"),
            "range_fold": ("found_algorithm", "range.segment_tree"),
            "shortest_paths": ("found_algorithm", "graph.dijkstra"),
            "compensated_sum": ("found_algorithm", "numeric.compensated_sum"),
            "offline_connectivity": ("found_recipe", "connectivity.offline"),
            "online_deletion_gap": ("not_covered", None),
            "geometry_closest_integer": ("found_algorithm", "geometry.closest_pair.int"),
            "geometry_hull_float": ("found_algorithm", "geometry.convex_hull.float"),
            "geometry_diameter_recipe": ("found_recipe", "geometry.point_set_diameter.int"),
            "geometry_rectangle_recipe": ("found_recipe", "geometry.point_set_rectangle"),
            "geometry_convex_location": ("found_algorithm", "geometry.convex_location.int"),
            "geometry_exact_float_gap": ("not_covered", None),
            "byte_search": ("found_algorithm", "strings.kmp"),
            "unicode_search_gap": ("not_covered", None),
            "inversions": ("found_algorithm", "sequence.inversions"),
            "inversions_fenwick_recipe": ("found_recipe", "sequence.inversions_fenwick"),
            "fenwick_sum": ("found_algorithm", "range.fenwick_sum"),
            "absorbing_markov": ("found_recipe", "probability.absorbing_markov"),
            "dense_lu": ("found_algorithm", "numeric.dense_lu"),
            "fft_exact_gap": ("not_covered", None),
            "knapsack_bounded": ("found_algorithm", "optimization.knapsack_bounded"),
            "translated_overlap": ("found_recipe", "geometry.translated_convex_overlap"),
        }
        for name, (status, asset_id) in expected.items():
            with self.subTest(query=name):
                query = json.loads((ROOT / "examples/queries" / f"{name}.json").read_text())
                result = catalog.match(query, self.assets)
                self.assertEqual(result["status"], status)
                if asset_id:
                    self.assertEqual(result["candidates"][0]["id"], asset_id)

    def test_repository_assets_fail_closed(self):
        bounded=dict(problem="knapsack",n=10,capacity=0,multiplicity="bounded",
                     objective="maximum_value_at_most_capacity",input_type="int64",output="value")
        self.assertEqual(self.query(**bounded)["status"],"found_algorithm")
        self.assertEqual(self.query(**bounded,max_memory_bytes=0)["status"],"budget_exceeded")
        bounded["capacity"]=-1
        with self.assertRaises(ValueError): self.query(**bounded)
        fields = dict(problem="range_sum", n=100, input_type="int64",
                      updates=["add", "query"], stored_blocks_fit_int64=True, result_guarantee="exact")
        self.assertEqual(self.query(**fields)["status"], "found_algorithm")
        fields["updates"] = ["range_add", "query"]
        self.assertEqual(self.query(**fields)["status"], "not_covered")
        fields["updates"] = ["add", "query"]
        fields["capabilities"] = {"undo": "supported"}
        self.assertEqual(self.query(**fields)["status"], "not_covered")
        inversions = dict(problem="inversion_count",n=100,input_type="int64",
                          comparison="strict",execution_mode="batch",strategy="any")
        result = self.query(**inversions)
        self.assertEqual([x["id"] for x in result["candidates"]], ["sequence.inversions"])
        inversions["n"] = 6074001001
        self.assertEqual(self.query(**inversions)["status"], "not_covered")
        self.assertEqual(self.query(problem="topological_order", n=10, m=10, directed=True,
                                   graph_updates="none",representation="csr_nonnegative_weights",
                                   cycle_output="witness")["status"],"not_covered")

    def test_geometry_bounds_state_and_input_shape(self):
        fields = dict(problem="geometry.closest_pair", n=100,
                      coordinates="int64_abs_le_1e9", result_guarantee="exact_squared_distance",
                      input_structure="unordered_points", updates="none")
        self.assertEqual(self.query(**fields)["status"], "found_algorithm")
        fields["coordinates"] = "full_range_int64"
        self.assertEqual(self.query(**fields)["status"], "not_covered")
        fields["coordinates"] = "int64_abs_le_1e9"
        fields["updates"] = "online_insert"
        self.assertEqual(self.query(**fields)["status"], "not_covered")
        fields["updates"] = "none"
        fields["capabilities"] = {"rollback": "supported"}
        self.assertEqual(self.query(**fields)["status"], "not_covered")
        self.assertEqual(self.query(problem="geometry.minimum_rectangle", n=100,
                                   coordinates="float64_abs_le_1e100", result_guarantee="approximate",
                                   input_structure="unordered_points", updates="none")["status"], "found_recipe")


if __name__ == "__main__":
    unittest.main()
