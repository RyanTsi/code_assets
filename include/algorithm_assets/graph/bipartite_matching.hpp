#pragma once
#include <algorithm_assets/graph/unweighted_csr.hpp>
#include <algorithm>
#include <limits>

namespace algorithm_assets {
struct BipartiteMatching {
    static constexpr std::uint32_t unmatched = std::numeric_limits<std::uint32_t>::max();
    std::vector<std::uint32_t> left_to_right, right_to_left;
    std::vector<bool> cover_left, cover_right;
    std::size_t size = 0;
};
// Left and right IDs are separate namespaces. graph rows [0,left_size) are left
// adjacency; graph.vertex_count() must be max(left_size,right_size).
inline BipartiteMatching hopcroft_karp(std::size_t left_size, std::size_t right_size,
                                       const UnweightedCsrGraph& graph) {
    using V = std::uint32_t;
    constexpr V none = BipartiteMatching::unmatched;
    if (graph.vertex_count() != std::max(left_size, right_size))
        throw std::invalid_argument("Bipartite graph size mismatch");
    for (std::size_t u = 0; u < graph.vertex_count(); ++u)
        for (auto v : graph.neighbors(static_cast<V>(u)).targets)
            if (u >= left_size || v >= right_size)
                throw std::out_of_range("Invalid bipartite edge");
    BipartiteMatching result{std::vector<V>(left_size, none), std::vector<V>(right_size, none),
                             std::vector<bool>(left_size), std::vector<bool>(right_size), 0};
    std::vector<V> distance(left_size), queue, path_left, path_right;
    std::vector<std::size_t> cursor(left_size);
    for (;;) {
        std::fill(distance.begin(), distance.end(), none);
        queue.clear();
        for (std::size_t u = 0; u < left_size; ++u)
            if (result.left_to_right[u] == none) {
                distance[u] = 0;
                queue.push_back(static_cast<V>(u));
            }
        V shortest = none;
        for (std::size_t i = 0; i < queue.size(); ++i) {
            const auto u = queue[i];
            if (distance[u] >= shortest)
                continue;
            for (auto v : graph.neighbors(u).targets) {
                const auto mate = result.right_to_left[v];
                if (mate == none)
                    shortest = std::min(shortest, distance[u] + 1);
                else if (distance[mate] == none) {
                    distance[mate] = distance[u] + 1;
                    queue.push_back(mate);
                }
            }
        }
        if (shortest == none)
            break;
        std::fill(cursor.begin(), cursor.end(), 0);
        for (std::size_t root = 0; root < left_size; ++root) {
            if (result.left_to_right[root] != none)
                continue;
            path_left.assign(1, static_cast<V>(root));
            path_right.clear();
            bool augmented = false;
            while (!path_left.empty() && !augmented) {
                const auto u = path_left.back();
                const auto neighbors = graph.neighbors(u).targets;
                bool descended = false;
                while (cursor[u] < neighbors.size()) {
                    const auto v = neighbors[cursor[u]++], mate = result.right_to_left[v];
                    if (mate == none && distance[u] + 1 == shortest) {
                        path_right.push_back(v);
                        for (std::size_t i = 0; i < path_left.size(); ++i) {
                            result.left_to_right[path_left[i]] = path_right[i];
                            result.right_to_left[path_right[i]] = path_left[i];
                        }
                        ++result.size;
                        augmented = true;
                        break;
                    }
                    if (mate != none && distance[mate] == distance[u] + 1) {
                        path_right.push_back(v);
                        path_left.push_back(mate);
                        descended = true;
                        break;
                    }
                }
                if (!augmented && !descended) {
                    distance[u] = none;
                    path_left.pop_back();
                    if (!path_right.empty())
                        path_right.pop_back();
                }
            }
        }
    }
    // Konig cover: complement of reachable left, plus reachable right.
    std::vector<bool> seen_left(left_size), seen_right(right_size);
    queue.clear();
    for (std::size_t u = 0; u < left_size; ++u)
        if (result.left_to_right[u] == none) {
            seen_left[u] = true;
            queue.push_back(static_cast<V>(u));
        }
    for (std::size_t i = 0; i < queue.size(); ++i) {
        const auto u = queue[i];
        for (auto v : graph.neighbors(u).targets)
            if (v != result.left_to_right[u] && !seen_right[v]) {
                seen_right[v] = true;
                const auto mate = result.right_to_left[v];
                if (mate != none && !seen_left[mate]) {
                    seen_left[mate] = true;
                    queue.push_back(mate);
                }
            }
    }
    for (std::size_t u = 0; u < left_size; ++u)
        result.cover_left[u] = !seen_left[u];
    result.cover_right = std::move(seen_right);
    return result;
}
} // namespace algorithm_assets
