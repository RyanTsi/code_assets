#pragma once
#include <algorithm_assets/graph/unweighted_csr.hpp>
#include <algorithm>
#include <limits>

namespace algorithm_assets {
struct StrongComponents {
  std::vector<std::uint32_t> component;
  std::size_t count = 0;
};
// Iterative Tarjan; component IDs increase in REVERSE topological order.
inline StrongComponents strong_components(const UnweightedCsrGraph& graph) {
  using V = UnweightedCsrGraph::Vertex;
  constexpr V none = std::numeric_limits<V>::max();
  const auto n = graph.vertex_count();
  StrongComponents result{std::vector<V>(n, none), 0};
  std::vector<V> discovery(n, none), low(n), active;
  std::vector<bool> on_stack(n, false);
  struct Frame { V vertex; std::size_t next; };
  std::vector<Frame> frames;
  V clock = 0;
  auto enter = [&](V u) {
    discovery[u] = low[u] = clock++;
    active.push_back(u); on_stack[u] = true;
    frames.push_back({u, 0});
  };
  for (std::size_t root = 0; root < n; ++root) {
    if (discovery[root] != none) continue;
    enter(static_cast<V>(root));
    while (!frames.empty()) {
      const V u = frames.back().vertex;
      const auto neighbors = graph.neighbors(u).targets;
      if (frames.back().next < neighbors.size()) {
        const auto v = neighbors[frames.back().next++];
        if (discovery[v] == none) enter(v);
        else if (on_stack[v]) low[u] = std::min(low[u], discovery[v]);
        continue;
      }
      if (low[u] == discovery[u]) {
        for (;;) {
          const auto v = active.back(); active.pop_back(); on_stack[v] = false;
          result.component[v] = static_cast<V>(result.count);
          if (v == u) break;
        }
        ++result.count;
      }
      frames.pop_back();
      if (!frames.empty()) {
        const auto parent = frames.back().vertex;
        low[parent] = std::min(low[parent], low[u]);
      }
    }
  }
  return result;
}
struct Condensation {
  StrongComponents partition;
  UnweightedCsrGraph dag;
};
// Preserve parallel inter-component arcs; topology algorithms accept them.
inline Condensation condense_graph(const UnweightedCsrGraph& graph) {
  auto partition = strong_components(graph);
  std::vector<UnweightedCsrGraph::Edge> edges;
  edges.reserve(graph.edge_count());
  for (std::size_t u = 0; u < graph.vertex_count(); ++u)
    for (auto v : graph.neighbors(static_cast<std::uint32_t>(u)).targets)
      if (partition.component[u] != partition.component[v])
        edges.push_back({partition.component[u], partition.component[v]});
  UnweightedCsrGraph dag(partition.count, edges);
  return {std::move(partition), std::move(dag)};
}
} // namespace algorithm_assets
