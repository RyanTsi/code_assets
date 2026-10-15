#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {

// A negative entry stores a root's component size; other entries store its parent.
class DisjointSet {
public:
  explicit DisjointSet(std::size_t size) : parents_(checked_size(size), -1), components_(size) {}

  [[nodiscard]] std::size_t size() const noexcept { return parents_.size(); }
  [[nodiscard]] std::size_t component_count() const noexcept { return components_; }

  std::size_t leader(std::size_t vertex) {
    check_vertex(vertex);
    return find_root(vertex);
  }

  bool same(std::size_t first, std::size_t second) {
    check_vertex(first);
    check_vertex(second);
    return find_root(first) == find_root(second);
  }

  std::size_t component_size(std::size_t vertex) {
    check_vertex(vertex);
    return static_cast<std::size_t>(-parents_[find_root(vertex)]);
  }

  // Returns false for a redundant merge. Queries can mutate paths as well.
  bool merge(std::size_t first, std::size_t second) {
    check_vertex(first);
    check_vertex(second);
    first = find_root(first);
    second = find_root(second);
    if (first == second) {
      return false;
    }
    if (parents_[first] > parents_[second]) {
      const auto temporary = first;
      first = second;
      second = temporary;
    }
    parents_[first] += parents_[second];
    parents_[second] = static_cast<std::int32_t>(first);
    --components_;
    return true;
  }

private:
  static std::size_t checked_size(std::size_t size) {
    if (size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
      throw std::length_error("DisjointSet requires at most INT32_MAX vertices");
    }
    return size;
  }

  void check_vertex(std::size_t vertex) const {
    if (vertex >= size()) {
      throw std::out_of_range("DisjointSet vertex is out of range");
    }
  }

  std::size_t find_root(std::size_t vertex) {
    auto root = vertex;
    while (parents_[root] >= 0) {
      root = static_cast<std::size_t>(parents_[root]);
    }
    while (vertex != root) {
      const auto next = static_cast<std::size_t>(parents_[vertex]);
      parents_[vertex] = static_cast<std::int32_t>(root);
      vertex = next;
    }
    return root;
  }

  std::vector<std::int32_t> parents_;
  std::size_t components_;
};

}  // namespace algorithm_assets
