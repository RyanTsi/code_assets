#pragma once
#include <algorithm_assets/range/segment_tree.hpp>
#include <algorithm_assets/sequence/coordinate_compression.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {
// Longest subsequence whose adjacent original values differ by at most max_difference.
inline std::size_t bounded_step_subsequence_length(
    std::span<const std::int64_t> values, std::int64_t max_difference) {
  if (max_difference < 0) throw std::invalid_argument("Difference must be nonnegative");
  const CoordinateCompression coordinates(values);
  const auto maximum = [](std::size_t a, std::size_t b) { return std::max(a, b); };
  auto make_tree = [&] {
    const std::vector<std::size_t> zeros(coordinates.size(), 0);
    return SegmentTree<std::size_t, decltype(maximum)>(zeros, 0, maximum);
  };
  auto best = make_tree(); // Initialization copy is freed before the DP scan.
  using Limits = std::numeric_limits<std::int64_t>;
  for (const auto value : values) {
    const auto low = value < Limits::min() + max_difference ? Limits::min() : value - max_difference;
    const auto high = value > Limits::max() - max_difference ? Limits::max() : value + max_difference;
    // Compression preserves order, not numeric distance: bounds use original values.
    const auto first = coordinates.lower_rank(low);
    const auto last = high == Limits::max() ? coordinates.size() : coordinates.lower_rank(high + 1);
    const auto length = best.fold(first, last) + 1;
    const auto rank = *coordinates.rank(value);
    best.set(rank, std::max(best.get(rank), length));
  }
  return best.all_fold();
}
} // namespace algorithm_assets
