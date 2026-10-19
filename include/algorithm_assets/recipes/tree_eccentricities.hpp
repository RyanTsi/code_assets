#pragma once
#include <algorithm_assets/graph/unweighted_csr.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace algorithm_assets {
struct TreeEccentricities {
    std::vector<std::uint32_t> source_distances, eccentricities;
    std::uint32_t diameter_first, diameter_second, diameter_length;
};

// Each input edge is undirected and listed once; counts are in unit edges.
inline TreeEccentricities tree_eccentricities(std::size_t n,
                                              std::span<const UnweightedCsrGraph::Edge> edges,
                                              std::uint32_t source = 0) {
    if (n == 0 || n >= std::numeric_limits<std::uint32_t>::max())
        throw std::invalid_argument("Tree vertex count must be in [1, UINT32_MAX)");
    if (source >= n)
        throw std::out_of_range("Invalid source");
    if (edges.size() != n - 1)
        throw std::invalid_argument("Tree must have n-1 edges");
    const UnweightedCsrGraph graph = [&] {
        std::vector<UnweightedCsrGraph::Edge> arcs;
        if (edges.size() > arcs.max_size() / 2)
            throw std::length_error("Too many tree edges");
        arcs.reserve(2 * edges.size());
        for (const auto edge : edges) {
            arcs.push_back(edge);
            arcs.push_back({edge.to, edge.from});
        }
        return UnweightedCsrGraph(n, arcs);
    }(); // The doubled input and construction cursor are no longer live.
    std::vector<std::uint32_t> queue;
    queue.reserve(n);
    auto distances = [&](std::uint32_t origin) {
        constexpr auto unseen = std::numeric_limits<std::uint32_t>::max();
        std::vector<std::uint32_t> result(n, unseen);
        queue.clear();
        queue.push_back(origin);
        result[origin] = 0;
        for (std::size_t i = 0; i < queue.size(); ++i) {
            const auto u = queue[i];
            for (const auto v : graph.neighbors(u).targets)
                if (result[v] == unseen) {
                    result[v] = result[u] + 1;
                    queue.push_back(v);
                }
        }
        if (queue.size() != n)
            throw std::invalid_argument("Tree must be connected");
        return result;
    };
    const auto farthest = [](const auto& distances) {
        return static_cast<std::uint32_t>(std::max_element(distances.begin(), distances.end()) -
                                          distances.begin());
    };
    auto from_source = distances(source);
    const auto first = farthest(from_source);
    auto from_first = distances(first);
    const auto second = farthest(from_first);
    const auto diameter = from_first[second];
    const auto from_second = distances(second);
    for (std::size_t i = 0; i < n; ++i)
        from_first[i] = std::max(from_first[i], from_second[i]);
    return {std::move(from_source), std::move(from_first), first, second, diameter};
}
} // namespace algorithm_assets
