#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace algorithm_assets {

class RollbackDisjointSet {
public:
  // A depth is valid only on the same object's current ancestral history.
  using Checkpoint = std::size_t;

  explicit RollbackDisjointSet(std::size_t size)
      : parents_(checked_size(size), -1), components_(size) {}

  [[nodiscard]] std::size_t size() const noexcept { return parents_.size(); }
  [[nodiscard]] std::size_t component_count() const noexcept { return components_; }
  [[nodiscard]] Checkpoint checkpoint() const noexcept { return history_.size(); }

  [[nodiscard]] std::size_t leader(std::size_t vertex) const {
    check_vertex(vertex);
    return find_root(vertex);
  }

  [[nodiscard]] bool same(std::size_t first, std::size_t second) const {
    check_vertex(first);
    check_vertex(second);
    return find_root(first) == find_root(second);
  }

  [[nodiscard]] std::size_t component_size(std::size_t vertex) const {
    check_vertex(vertex);
    return static_cast<std::size_t>(-parents_[find_root(vertex)]);
  }

  bool merge(std::size_t first, std::size_t second) {
    check_vertex(first);
    check_vertex(second);
    first = find_root(first);
    second = find_root(second);
    if (first == second) {
      return false;  // Failed merges consume no history.
    }
    if (parents_[first] > parents_[second]) {
      std::swap(first, second);
    }
    // Record before mutation, so allocation failure preserves the partition.
    history_.push_back({static_cast<std::int32_t>(first), static_cast<std::int32_t>(second),
                        parents_[first]});
    parents_[first] += parents_[second];
    parents_[second] = static_cast<std::int32_t>(first);
    --components_;
    return true;
  }

  bool undo_last_merge() noexcept {
    if (history_.empty()) {
      return false;
    }
    const auto change = history_.back();
    history_.pop_back();
    // The merged size is the sum of the two original negative sizes.
    parents_[change.child] = parents_[change.parent] - change.parent_size;
    parents_[change.parent] = change.parent_size;
    ++components_;
    return true;
  }

  void rollback(Checkpoint checkpoint) {
    if (checkpoint > history_.size()) {
      throw std::out_of_range("RollbackDisjointSet checkpoint is ahead of current history");
    }
    while (history_.size() > checkpoint) {
      undo_last_merge();
    }
  }

private:
  struct Change {
    std::int32_t parent;
    std::int32_t child;
    std::int32_t parent_size;
  };

  static std::size_t checked_size(std::size_t size) {
    if (size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
      throw std::length_error("RollbackDisjointSet requires at most INT32_MAX vertices");
    }
    return size;
  }

  void check_vertex(std::size_t vertex) const {
    if (vertex >= size()) {
      throw std::out_of_range("RollbackDisjointSet vertex is out of range");
    }
  }

  std::size_t find_root(std::size_t vertex) const noexcept {
    // No path compression: union by size bounds depth and keeps history local.
    while (parents_[vertex] >= 0) {
      vertex = static_cast<std::size_t>(parents_[vertex]);
    }
    return vertex;
  }

  std::vector<std::int32_t> parents_;
  std::vector<Change> history_;
  std::size_t components_;
};

}  // namespace algorithm_assets
