#pragma once

#include <algorithm_assets/graph/csr_graph.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {

class ShortestPaths {
public:
    using Vertex = CsrGraph::Vertex;
    using Distance = std::uint64_t;
    static constexpr auto infinity = std::numeric_limits<Distance>::max();

    [[nodiscard]] std::size_t size() const noexcept {
        return distances_.size();
    }
    [[nodiscard]] Vertex source() const noexcept {
        return source_;
    }

    [[nodiscard]] std::optional<Distance> distance_to(Vertex vertex) const {
        check_vertex(vertex);
        return distances_[vertex] == infinity ? std::nullopt
                                              : std::optional<Distance>(distances_[vertex]);
    }

    [[nodiscard]] std::optional<Vertex> predecessor(Vertex vertex) const {
        check_vertex(vertex);
        return vertex == source_ || distances_[vertex] == infinity
                   ? std::nullopt
                   : std::optional<Vertex>(parents_[vertex]);
    }

    [[nodiscard]] std::vector<Vertex> path_to(Vertex vertex) const {
        check_vertex(vertex);
        if (distances_[vertex] == infinity) {
            return {};
        }
        std::vector<Vertex> path;
        for (;;) {
            path.push_back(vertex);
            if (vertex == source_) {
                break;
            }
            vertex = parents_[vertex];
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

private:
    friend ShortestPaths dijkstra(const CsrGraph&, Vertex);

    ShortestPaths(std::size_t size, Vertex source)
        : source_(source), distances_(size, infinity), parents_(size, no_vertex) {
    }

    void check_vertex(Vertex vertex) const {
        if (vertex >= size()) {
            throw std::out_of_range("ShortestPaths vertex is out of range");
        }
    }

    static constexpr auto no_vertex = std::numeric_limits<Vertex>::max();
    Vertex source_;
    std::vector<Distance> distances_;
    std::vector<Vertex> parents_;
};

namespace detail {

// One entry per vertex, in contrast to a lazy heap with O(m) duplicate entries.
class DijkstraHeap {
public:
    using Vertex = CsrGraph::Vertex;

    explicit DijkstraHeap(const std::vector<std::uint64_t>& distances)
        : distances_(distances), positions_(distances.size(), absent) {
        heap_.reserve(distances.size());
    }

    [[nodiscard]] bool empty() const noexcept {
        return heap_.empty();
    }

    void push_or_decrease(Vertex vertex) {
        auto position = positions_[vertex];
        if (position == absent) {
            position = static_cast<Vertex>(heap_.size());
            heap_.push_back(vertex);
            positions_[vertex] = position;
        }
        while (position > 0) {
            const auto parent = (position - 1) / 2;
            if (!less(heap_[position], heap_[parent])) {
                break;
            }
            exchange(position, parent);
            position = parent;
        }
    }

    Vertex pop_min() {
        const auto result = heap_.front();
        exchange(0, static_cast<Vertex>(heap_.size() - 1));
        heap_.pop_back();
        positions_[result] = absent;
        std::size_t position = 0;
        while (2 * position + 1 < heap_.size()) {
            auto child = 2 * position + 1;
            if (child + 1 < heap_.size() && less(heap_[child + 1], heap_[child])) {
                ++child;
            }
            if (!less(heap_[child], heap_[position])) {
                break;
            }
            exchange(static_cast<Vertex>(position), static_cast<Vertex>(child));
            position = child;
        }
        return result;
    }

private:
    bool less(Vertex first, Vertex second) const noexcept {
        return distances_[first] < distances_[second] ||
               (distances_[first] == distances_[second] && first < second);
    }

    void exchange(Vertex first, Vertex second) noexcept {
        std::swap(heap_[first], heap_[second]);
        positions_[heap_[first]] = first;
        positions_[heap_[second]] = second;
    }

    static constexpr auto absent = std::numeric_limits<Vertex>::max();
    const std::vector<std::uint64_t>& distances_;
    std::vector<Vertex> heap_;
    std::vector<Vertex> positions_;
};

} // namespace detail

inline ShortestPaths dijkstra(const CsrGraph& graph, CsrGraph::Vertex source) {
    if (source >= graph.vertex_count()) {
        throw std::out_of_range("Dijkstra source is out of range");
    }
    ShortestPaths result(graph.vertex_count(), source);
    result.distances_[source] = 0;
    result.parents_[source] = source;
    bool exceeded_distance_range = false;
    {
        detail::DijkstraHeap heap(result.distances_);
        heap.push_or_decrease(source);
        while (!heap.empty()) {
            const auto vertex = heap.pop_min();
            const auto neighbors = graph.neighbors(vertex);
            for (std::size_t index = 0; index < neighbors.targets.size(); ++index) {
                const auto next = neighbors.targets[index];
                const auto weight = neighbors.weights[index];
                if (weight >= ShortestPaths::infinity - result.distances_[vertex]) {
                    exceeded_distance_range = true;
                    continue; // Too large to improve any representable distance.
                }
                const auto candidate = result.distances_[vertex] + weight;
                if (candidate < result.distances_[next]) {
                    result.distances_[next] = candidate;
                    result.parents_[next] = vertex;
                    heap.push_or_decrease(next);
                }
            }
        }
    }
    if (exceeded_distance_range) {
        // Distinguish an overflowing shortest distance from a merely long detour.
        // The heap has been freed before allocating this reachability workspace.
        std::vector<unsigned char> reached(graph.vertex_count(), 0);
        std::vector<CsrGraph::Vertex> frontier;
        frontier.reserve(graph.vertex_count());
        reached[source] = 1;
        frontier.push_back(source);
        for (std::size_t index = 0; index < frontier.size(); ++index) {
            for (const auto next : graph.neighbors(frontier[index]).targets) {
                if (result.distances_[next] == ShortestPaths::infinity) {
                    throw std::overflow_error(
                        "A reachable shortest distance exceeds UINT64_MAX - 1");
                }
                if (!reached[next]) {
                    reached[next] = 1;
                    frontier.push_back(next);
                }
            }
        }
    }
    return result;
}

} // namespace algorithm_assets
