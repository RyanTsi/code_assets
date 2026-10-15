#pragma once

#include <algorithm_assets/numeric/wide_int64_sum.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {

// n int64 nodes, 0-based positions and half-open query ranges.
class FenwickSum {
public:
  explicit FenwickSum(std::size_t size) : tree_(size, 0) {}
  explicit FenwickSum(std::span<const std::int64_t> values) : tree_(values.size()) {
    // Each node aggregates its completed children, using a wide local total.
    // Sum of child counts is sum(v2(i+1)) < n: O(n), no n-sized wide buffer.
    for (std::size_t i = 0; i < values.size(); ++i) {
      WideInt64Sum total;
      total.add(values[i]);
      const auto first = i & (i + 1);
      for (auto previous = i; previous > first; previous &= previous - 1)
        total.add(tree_[previous - 1]);
      tree_[i] = total.value();
    }
  }
  [[nodiscard]] std::size_t size() const noexcept { return tree_.size(); }
  [[nodiscard]] std::size_t storage_bytes() const noexcept { return tree_.capacity() * sizeof(std::int64_t); }

  void add(std::size_t position, std::int64_t delta) {
    if (position >= size()) throw std::out_of_range("Fenwick position out of range");
    // Validate every affected block before any mutation: strong guarantee.
    for (auto i = position; i < size(); i |= i + 1) (void)checked_int64_add(tree_[i], delta);
    for (auto i = position; i < size(); i |= i + 1) tree_[i] += delta;
  }
  [[nodiscard]] std::int64_t prefix_sum(std::size_t last) const {
    if (last > size()) throw std::out_of_range("Fenwick prefix out of range");
    WideInt64Sum total;
    accumulate_prefix(last, total, false);
    return total.value();
  }
  [[nodiscard]] std::int64_t range_sum(std::size_t first, std::size_t last) const {
    if (first > last || last > size()) throw std::out_of_range("Invalid Fenwick range");
    WideInt64Sum total;
    // Neither individual prefix need fit int64 if their difference does.
    accumulate_prefix(last, total, false);
    accumulate_prefix(first, total, true);
    return total.value();
  }
private:
  void accumulate_prefix(std::size_t last, WideInt64Sum& total, bool subtract) const {
    for (; last; last &= last - 1) {
      if (subtract) total.subtract(tree_[last - 1]);
      else total.add(tree_[last - 1]);
    }
  }
  std::vector<std::int64_t> tree_;
};

} // namespace algorithm_assets
