#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {

// Immutable directed graph: separate arrays avoid padding in each stored arc.
class CsrGraph {
public:
    using Vertex = std::uint32_t;
    using Weight = std::uint64_t;

    struct Edge {
        Vertex from;
        Vertex to;
        std::int64_t weight;
    };

    struct Neighbors {
        std::span<const Vertex> targets;
        std::span<const Weight> weights;
    };

    CsrGraph(std::size_t vertex_count, std::span<const Edge> edges)
        : offsets_(checked_size(vertex_count) + 1, 0), targets_(edges.size()),
          weights_(edges.size()) {
        for (const auto& edge : edges) {
            if (edge.from >= vertex_count || edge.to >= vertex_count) {
                throw std::out_of_range("CsrGraph edge endpoint is out of range");
            }
            if (edge.weight < 0) {
                throw std::invalid_argument("CsrGraph requires nonnegative edge weights");
            }
            ++offsets_[edge.from + 1];
        }
        for (std::size_t vertex = 1; vertex < offsets_.size(); ++vertex) {
            offsets_[vertex] += offsets_[vertex - 1];
        }
        auto cursor = offsets_;
        for (const auto& edge : edges) {
            const auto index = cursor[edge.from]++;
            targets_[index] = edge.to;
            weights_[index] = static_cast<Weight>(edge.weight);
        }
    }

    [[nodiscard]] std::size_t vertex_count() const noexcept {
        return offsets_.size() - 1;
    }
    [[nodiscard]] std::size_t edge_count() const noexcept {
        return targets_.size();
    }

    [[nodiscard]] Neighbors neighbors(Vertex vertex) const {
        if (vertex >= vertex_count()) {
            throw std::out_of_range("CsrGraph vertex is out of range");
        }
        const auto first = offsets_[vertex];
        const auto count = offsets_[vertex + 1] - first;
        return {std::span<const Vertex>(targets_).subspan(first, count),
                std::span<const Weight>(weights_).subspan(first, count)};
    }

private:
    static std::size_t checked_size(std::size_t size) {
        if (size >= std::numeric_limits<Vertex>::max()) {
            throw std::length_error("CsrGraph requires fewer than UINT32_MAX vertices");
        }
        return size;
    }

    std::vector<std::size_t> offsets_;
    std::vector<Vertex> targets_;
    std::vector<Weight> weights_;
};

} // namespace algorithm_assets
