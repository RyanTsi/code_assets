#pragma once

#include <algorithm_assets/connectivity/disjoint_set.hpp>
#include <algorithm_assets/numeric/wide_int64_sum.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {

struct UndirectedWeightedEdge {
    std::uint32_t first{}, second{};
    std::int64_t weight{};
};
struct MinimumSpanningForest {
    std::vector<std::size_t> edge_indices; // Original input positions, not references.
    std::int64_t total_weight = 0;
    std::size_t component_count = 0;
    bool is_spanning_tree = false; // Empty graph is an empty forest, not a tree.
};

inline MinimumSpanningForest
kruskal_minimum_spanning_forest(std::size_t vertex_count,
                                std::span<const UndirectedWeightedEdge> edges) {
    for (const auto& edge : edges)
        if (edge.first >= vertex_count || edge.second >= vertex_count)
            throw std::out_of_range("Kruskal edge endpoint out of range");
    DisjointSet components(vertex_count);
    std::vector<std::size_t> order(edges.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::sort(order.begin(), order.end(), [&](auto a, auto b) {
        return edges[a].weight < edges[b].weight || (edges[a].weight == edges[b].weight && a < b);
    });
    MinimumSpanningForest result;
    // Isolated graphs must not reserve n output slots when m is small.
    result.edge_indices.reserve(std::min(edges.size(), vertex_count ? vertex_count - 1 : 0));
    WideInt64Sum sum;
    for (const auto index : order) {
        const auto& edge = edges[index];
        if (components.merge(edge.first, edge.second)) {
            result.edge_indices.push_back(index);
            sum.add(edge.weight);
            if (components.component_count() == 1)
                break;
        }
    }
    result.total_weight = sum.value();
    result.component_count = components.component_count();
    result.is_spanning_tree = vertex_count != 0 && result.component_count == 1;
    return result;
}

} // namespace algorithm_assets
