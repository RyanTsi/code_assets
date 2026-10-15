#include <algorithm_assets/recipes/disjoint_interval_pair.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  std::size_t n; std::int64_t duration;
  if (!(std::cin >> n >> duration)) return 0;
  std::vector<algorithm_assets::PricedInterval> intervals(n);
  for (auto& interval : intervals) {
    std::cin >> interval.begin >> interval.end >> interval.cost;
    ++interval.end; // Inclusive integer days -> half-open; CF bounds make this safe.
  }
  const auto answer = algorithm_assets::cheapest_disjoint_interval_pair(intervals, duration);
  std::cout << (answer ? answer->cost : -1) << '\n';
}
