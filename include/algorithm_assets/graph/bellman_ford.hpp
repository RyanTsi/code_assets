#pragma once
#include <algorithm_assets/numeric/wide_int64_sum.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {
struct SignedDirectedEdge {
    std::uint32_t from, to;
    std::int64_t weight;
};
enum class DistanceStatus : std::uint8_t { unreachable, finite, negative_infinity };
struct SignedDistance {
    std::int64_t value = 0;
    DistanceStatus status = DistanceStatus::unreachable;
};
// null source is a virtual zero-cost source reaching every vertex.
inline std::vector<SignedDistance> bellman_ford(std::size_t n,
                                                std::span<const SignedDirectedEdge> edges,
                                                std::optional<std::uint32_t> source) {
    if (n >= std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("Too many vertices");
    if (source && *source >= n)
        throw std::out_of_range("Invalid source");
    const auto bound = std::numeric_limits<std::int64_t>::max() /
                       static_cast<std::int64_t>(std::max(n, std::size_t{1}));
    for (auto e : edges) {
        if (e.from >= n || e.to >= n)
            throw std::out_of_range("Invalid signed edge");
        if (e.weight < -bound || e.weight > bound)
            throw std::invalid_argument("Weight exceeds exact n-edge bound");
    }
    std::vector<SignedDistance> distance(n);
    if (source)
        distance[*source].status = DistanceStatus::finite;
    else
        for (auto& d : distance)
            d.status = DistanceStatus::finite;
    auto next = distance;
    std::vector<bool> affected(n);
    for (std::size_t pass = 0; pass < n; ++pass) {
        next = distance;
        bool changed = false;
        for (auto e : edges)
            if (distance[e.from].status == DistanceStatus::finite) {
                const auto candidate = checked_int64_add(distance[e.from].value, e.weight);
                if (next[e.to].status == DistanceStatus::unreachable ||
                    candidate < next[e.to].value) {
                    next[e.to] = {candidate, DistanceStatus::finite};
                    changed = true;
                    if (pass + 1 == n)
                        affected[e.to] = true;
                }
            }
        distance.swap(next);
        if (!changed)
            break;
    }
    // Reachability propagation by scans avoids a second adjacency representation.
    for (std::size_t pass = 0; pass < n; ++pass) {
        bool changed = false;
        for (auto e : edges)
            if (affected[e.from] && !affected[e.to]) {
                affected[e.to] = true;
                changed = true;
            }
        if (!changed)
            break;
    }
    for (std::size_t i = 0; i < n; ++i)
        if (affected[i])
            distance[i] = {0, DistanceStatus::negative_infinity};
    return distance;
}
// Each edge v->u with w encodes x[u] <= x[v]+w. Not an optimality objective.
inline std::optional<std::vector<std::int64_t>>
solve_difference_constraints(std::size_t variables,
                             std::span<const SignedDirectedEdge> constraints) {
    auto distances = bellman_ford(variables, constraints, std::nullopt);
    std::vector<std::int64_t> solution;
    solution.reserve(variables);
    for (auto d : distances) {
        if (d.status == DistanceStatus::negative_infinity)
            return std::nullopt;
        solution.push_back(d.value);
    }
    return solution;
}
} // namespace algorithm_assets
