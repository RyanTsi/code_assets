#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {
class UnweightedCsrGraph {
public:
  using Vertex = std::uint32_t;
  struct Edge { Vertex from, to; };
  struct Neighbors { std::span<const Vertex> targets; };
  UnweightedCsrGraph(std::size_t n, std::span<const Edge> edges) {
    if (n >= std::numeric_limits<Vertex>::max()) throw std::length_error("Too many vertices");
    offsets_.assign(n + 1, 0);
    for (auto e : edges) {
      if (e.from >= n || e.to >= n) throw std::out_of_range("Invalid graph endpoint");
      ++offsets_[e.from + 1];
    }
    for (std::size_t i = 1; i <= n; ++i) offsets_[i] += offsets_[i - 1];
    targets_.resize(edges.size());
    auto cursor = offsets_;
    for (auto e : edges) targets_[cursor[e.from]++] = e.to;
  }
  [[nodiscard]] std::size_t vertex_count() const noexcept { return offsets_.size() - 1; }
  [[nodiscard]] std::size_t edge_count() const noexcept { return targets_.size(); }
  [[nodiscard]] Neighbors neighbors(Vertex u) const {
    if (u >= vertex_count()) throw std::out_of_range("Invalid graph vertex");
    return {std::span<const Vertex>(targets_).subspan(offsets_[u], offsets_[u+1]-offsets_[u])};
  }
  [[nodiscard]] std::size_t storage_bytes() const noexcept {
    return offsets_.capacity()*sizeof(std::size_t) + targets_.capacity()*sizeof(Vertex);
  }
private:
  std::vector<std::size_t> offsets_;
  std::vector<Vertex> targets_;
};
} // namespace algorithm_assets
