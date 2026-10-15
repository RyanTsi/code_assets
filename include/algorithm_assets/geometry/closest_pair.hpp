#pragma once

#include <algorithm_assets/geometry/primitives.hpp>

#include <optional>
#include <vector>

namespace algorithm_assets::geometry {

// Each call copies the input; both owned point buffers keep capacity for reuse.
// The result owns coordinate values, not references into this workspace.
template<Coordinate T> class ClosestPairWorkspace {
public:
  [[nodiscard]] std::optional<PointPair<T>> solve(std::span<const Point2<T>> input) {
    detail::validate(input);
    if (input.size() < 2) return std::nullopt;
    points_.assign(input.begin(), input.end());
    std::sort(points_.begin(), points_.end(), detail::lex_less<T>);
    for (std::size_t i = 1; i < points_.size(); ++i)
      if (points_[i - 1] == points_[i]) return PointPair<T>{points_[i - 1], points_[i], 0};
    scratch_.resize(points_.size());
    std::copy(points_.begin(), points_.end(), scratch_.begin());
    return solve_range(points_, scratch_, 0, points_.size());
  }
  [[nodiscard]] std::size_t storage_bytes() const noexcept {
    return points_.capacity() * sizeof(Point2<T>) + scratch_.capacity() * sizeof(Point2<T>);
  }
  void release() noexcept {
    std::vector<Point2<T>>{}.swap(points_);
    std::vector<Point2<T>>{}.swap(scratch_);
  }
private:
  static bool y_less(Point2<T> a, Point2<T> b) { return a.y < b.y || (a.y == b.y && a.x < b.x); }
  PointPair<T> solve_range(std::vector<Point2<T>>& source, std::vector<Point2<T>>& output,
                           std::size_t first, std::size_t last) {
    if (last - first <= 3) {
      PointPair<T> result{output[first], output[first + 1], detail::squared_unchecked(output[first], output[first + 1])};
      for (auto i = first; i < last; ++i) for (auto j = i + 1; j < last; ++j) {
        const auto squared = detail::squared_unchecked(output[i], output[j]);
        if (squared < result.squared_distance) result = {output[i], output[j], squared};
      }
      std::sort(output.begin() + first, output.begin() + last, y_less);
      return result;
    }
    const auto middle = first + (last - first) / 2;
    const auto split_x = source[middle].x; // Save before children reorder by y.
    // Children write sorted halves into our source. Each parent then merges
    // once into its output, avoiding a full copy-back at every recursion level.
    auto result = solve_range(output, source, first, middle);
    const auto right_result = solve_range(output, source, middle, last);
    if (right_result.squared_distance < result.squared_distance) result = right_result;
    std::merge(source.begin() + first, source.begin() + middle,
               source.begin() + middle, source.begin() + last, output.begin() + first, y_less);
    // After merging, this source slice is no longer needed and holds the strip.
    std::size_t count = 0;
    for (auto i = first; i < last; ++i) {
      const SquaredDistance<T> dx = static_cast<SquaredDistance<T>>(output[i].x) - split_x;
      if (dx * dx >= result.squared_distance) continue;
      for (auto j = count; j > 0;) {
        --j;
        const auto candidate = source[first + j];
        const SquaredDistance<T> dy = static_cast<SquaredDistance<T>>(output[i].y) - candidate.y;
        if (dy * dy >= result.squared_distance) break;
        const auto squared = detail::squared_unchecked(output[i], candidate);
        if (squared < result.squared_distance) result = {output[i], candidate, squared};
      }
      source[first + count++] = output[i];
    }
    return result;
  }
  std::vector<Point2<T>> points_, scratch_;
};

template<Coordinate T> inline std::optional<PointPair<T>> closest_pair(std::span<const Point2<T>> points, ClosestPairWorkspace<T>& workspace) { return workspace.solve(points); }
template<Coordinate T> inline std::optional<PointPair<T>> closest_pair(std::span<const Point2<T>> points) {
  ClosestPairWorkspace<T> workspace;
  return workspace.solve(points);
}

} // namespace algorithm_assets::geometry
