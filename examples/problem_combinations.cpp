#include <algorithm_assets/recipes/disjoint_interval_pair.hpp>
#include <algorithm_assets/recipes/bounded_step_subsequence.hpp>
#include <algorithm_assets/recipes/tree_eccentricities.hpp>
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
  const std::vector<algorithm_assets::PricedInterval> slots{{0, 3, 4}, {4, 6, 1}, {0, 2, 9}};
  const auto pair = algorithm_assets::cheapest_disjoint_interval_pair(slots, 5);
  std::cout << "Disjoint slots: " << pair->first << ", " << pair->second << "; cost " << pair->cost << '\n';
  const std::vector<std::int64_t> signal{1, 5, 4, 3, 8, 6, 9, 7, 2, 4};
  std::cout << "Bounded step subsequence length: " << algorithm_assets::bounded_step_subsequence_length(signal, 3) << '\n';
  const std::vector<algorithm_assets::UnweightedCsrGraph::Edge> links{{0, 1}, {1, 2}, {1, 3}};
  const auto tree = algorithm_assets::tree_eccentricities(4, links);
  std::cout << "Tree diameter: " << tree.diameter_length << "; farthest distances:";
  for (const auto distance : tree.eccentricities) std::cout << ' ' << distance;
  std::cout << '\n';
}
