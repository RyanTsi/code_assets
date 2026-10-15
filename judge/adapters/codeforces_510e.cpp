#include <algorithm_assets/algebra/prime_sieve.hpp>
#include <algorithm_assets/graph/dinic.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

// CF510E-specific reduction, not a general prime-ring API.
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::uint32_t n;
  if (!(std::cin >> n)) return 0;
  std::vector<std::uint32_t> ages(n);
  for (auto& age : ages) std::cin >> age;
  const auto odd_count = std::count_if(ages.begin(), ages.end(), [](auto age) { return age % 2 != 0; });
  if (2 * odd_count != n) {
    std::cout << "Impossible\n";
    return 0;
  }
  // Ages >= 2 exclude the exceptional even prime 2: all legal edges cross parity.
  const algorithm_assets::PrimeSieve primes(2 * *std::max_element(ages.begin(), ages.end()));
  const auto source = n, sink = n + 1;
  algorithm_assets::Dinic network(n + 2);
  for (std::uint32_t i = 0; i < n; ++i) {
    if (ages[i] % 2) network.add_edge(source, i, 2);
    else network.add_edge(i, sink, 2);
  }
  const auto first_candidate = network.edge_count();
  for (std::uint32_t i = 0; i < n; ++i)
    if (ages[i] % 2)
      for (std::uint32_t j = 0; j < n; ++j)
        if (ages[j] % 2 == 0 && primes.is_prime(ages[i] + ages[j]))
          network.add_edge(i, j, 1);
  if (network.max_flow(source, sink).added != n) {
    std::cout << "Impossible\n";
    return 0;
  }
  // A saturated integral flow selects a simple degree-two graph.
  // Fixed two-neighbor storage avoids a vector allocation at each vertex.
  std::vector<std::array<std::uint32_t, 2>> neighbors(n);
  std::vector<unsigned char> degree(n, 0);
  for (auto id = first_candidate; id < network.edge_count(); ++id) {
    const auto edge = network.edge(id);
    if (edge.flow != 0) {
      neighbors[edge.from][degree[edge.from]++] = edge.to;
      neighbors[edge.to][degree[edge.to]++] = edge.from;
    }
  }
  std::vector<unsigned char> visited(n, 0);
  std::vector<std::uint32_t> order;
  std::vector<std::size_t> boundaries{0};
  order.reserve(n);
  for (std::uint32_t start = 0; start < n; ++start) {
    if (visited[start]) continue;
    auto previous = n, current = start;
    do {
      visited[current] = 1;
      order.push_back(current);
      const auto next = neighbors[current][neighbors[current][0] == previous ? 1 : 0];
      previous = current;
      current = next;
    } while (current != start);
    boundaries.push_back(order.size());
  }
  std::cout << boundaries.size() - 1 << '\n';
  for (std::size_t table = 1; table < boundaries.size(); ++table) {
    std::cout << boundaries[table] - boundaries[table - 1];
    for (auto i = boundaries[table - 1]; i < boundaries[table]; ++i)
      std::cout << ' ' << order[i] + 1;
    std::cout << '\n';
  }
}
