#include <algorithm_assets/algebra/xor_basis.hpp>
#include <algorithm_assets/graph/kruskal.hpp>
#include <algorithm_assets/graph/topological_sort.hpp>
#include <algorithm_assets/recipes/inversions_fenwick.hpp>
#include <algorithm_assets/strings/kmp.hpp>

#include <cstdint>
#include <iostream>
#include <vector>

int main() {
  using namespace algorithm_assets;
  const std::vector<std::int64_t> values{3, 1, 2};
  FenwickSum sums(values); sums.add(1, 4);
  std::cout << "range sum: " << sums.range_sum(0, 3) << '\n';
  KmpPattern pattern("ABA");
  pattern.for_each_match("ABABABA", [](std::size_t position) { std::cout << "match: " << position << '\n'; });
  XorBasis64 basis; basis.insert(5); basis.insert(3);
  std::cout << "maximum xor: " << basis.maximize() << '\n';
  const std::vector<UndirectedWeightedEdge> edges{{0, 1, -1}, {1, 2, 2}, {0, 2, 3}};
  std::cout << "minimum tree: " << kruskal_minimum_spanning_forest(3, edges).total_weight << '\n';
  const std::vector<CsrGraph::Edge> dependencies{{0, 1, 0}, {1, 2, 0}};
  const auto order = topological_sort(CsrGraph(3, dependencies));
  std::cout << "DAG: " << order.acyclic << ", inversions: " << count_inversions(values)
            << ", recipe: " << count_inversions_fenwick(values) << '\n';
}
