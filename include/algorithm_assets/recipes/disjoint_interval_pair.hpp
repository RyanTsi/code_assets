#pragma once
#include <algorithm_assets/sequence/coordinate_compression.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {
struct PricedInterval { std::int64_t begin, end, cost; }; // [begin, end)
struct DisjointIntervalPair { std::int64_t cost; std::size_t first, second; };

// Earliest-ending sweep + compressed exact durations. Returns original indices.
inline std::optional<DisjointIntervalPair> cheapest_disjoint_interval_pair(
    std::span<const PricedInterval> intervals, std::int64_t total_duration) {
  using Limits = std::numeric_limits<std::int64_t>;
  if (total_duration <= 0) throw std::invalid_argument("Total duration must be positive");
  std::vector<std::int64_t> durations;
  durations.reserve(intervals.size());
  for (const auto& interval : intervals) {
    if (interval.begin >= interval.end || interval.cost < 0)
      throw std::invalid_argument("Intervals must be nonempty with nonnegative costs");
    if (interval.begin < 0 && interval.end > Limits::max() + interval.begin)
      throw std::overflow_error("Interval duration exceeds INT64_MAX");
    durations.push_back(interval.end - interval.begin);
  }
  const CoordinateCompression lengths(std::move(durations));
  std::vector<std::size_t> starts(intervals.size()), ends(intervals.size());
  std::iota(starts.begin(), starts.end(), std::size_t{0});
  ends = starts;
  std::sort(starts.begin(), starts.end(), [&](auto a, auto b) { return intervals[a].begin < intervals[b].begin; });
  std::sort(ends.begin(), ends.end(), [&](auto a, auto b) { return intervals[a].end < intervals[b].end; });
  constexpr auto absent = std::numeric_limits<std::size_t>::max();
  struct Best { std::int64_t cost = 0; std::size_t index = absent; };
  std::vector<Best> best(lengths.size());
  std::optional<DisjointIntervalPair> result;
  bool has_overflowing_pair = false;
  std::size_t cursor = 0;
  for (const auto right : starts) {
    const auto& current = intervals[right];
    while (cursor < ends.size() && intervals[ends[cursor]].end <= current.begin) {
      const auto left = ends[cursor++];
      const auto& interval = intervals[left];
      auto& entry = best[*lengths.rank(interval.end - interval.begin)];
      if (entry.index == absent || interval.cost < entry.cost) entry = {interval.cost, left};
    }
    const auto duration = current.end - current.begin;
    if (duration >= total_duration) continue;
    const auto rank = lengths.rank(total_duration - duration);
    if (!rank || best[*rank].index == absent) continue;
    const auto left = best[*rank];
    if (current.cost > Limits::max() - left.cost) {
      has_overflowing_pair = true;
      continue; // A larger cost cannot improve a representable optimum.
    }
    const auto cost = left.cost + current.cost;
    if (!result || cost < result->cost) result = DisjointIntervalPair{cost, left.index, right};
  }
  if (!result && has_overflowing_pair) throw std::overflow_error("Minimum pair cost exceeds INT64_MAX");
  return result;
}
} // namespace algorithm_assets
