#pragma once

#include <algorithm_assets/graph/csr_graph.hpp>

#include <cstddef>
#include <vector>

namespace algorithm_assets {

struct TopologicalOrder {
  std::vector<CsrGraph::Vertex> vertices;
  bool acyclic = true;
};

template<class Graph> inline TopologicalOrder topological_sort(const Graph& graph) {
  const auto n = graph.vertex_count();
  std::vector<std::size_t> indegree(n, 0);
  for (std::size_t u = 0; u < n; ++u)
    for (const auto v : graph.neighbors(static_cast<CsrGraph::Vertex>(u)).targets) ++indegree[v];
  TopologicalOrder result;
  result.vertices.reserve(n);
  for (std::size_t u = 0; u < n; ++u)
    if (indegree[u] == 0) result.vertices.push_back(static_cast<CsrGraph::Vertex>(u));
  // The output vector is also the FIFO queue: no separate queue allocation.
  for (std::size_t cursor = 0; cursor < result.vertices.size(); ++cursor)
    for (const auto v : graph.neighbors(result.vertices[cursor]).targets)
      if (--indegree[v] == 0) result.vertices.push_back(v);
  result.acyclic = result.vertices.size() == n;
  return result;
}

} // namespace algorithm_assets
