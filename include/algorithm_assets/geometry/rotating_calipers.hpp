#pragma once

#include <algorithm_assets/geometry/convex_hull.hpp>

#include <optional>

namespace algorithm_assets::geometry {

template<Coordinate T> inline std::optional<PointPair<T>> convex_diameter(const ConvexPolygon2<T>& hull) {
  const auto p = hull.vertices();
  const auto n = p.size();
  if (n == 0) return std::nullopt;
  PointPair<T> result{p[0], p[n == 1 ? 0 : 1], detail::squared_unchecked(p[0], p[n == 1 ? 0 : 1])};
  if (n <= 2) return result;
  auto consider = [&](std::size_t i, std::size_t j) {
    const auto squared = detail::squared_unchecked(p[i], p[j]);
    if (squared > result.squared_distance) result = {p[i], p[j], squared};
  };
  std::size_t opposite = 1, advances = 0;
  for (std::size_t i = 0; i < n; ++i) {
    const auto next = (i + 1) % n;
    auto height = [&](std::size_t j) { return detail::determinant(p[i], p[next], p[j]); };
    while (height((opposite + 1) % n) > height(opposite)) {
      opposite = (opposite + 1) % n;
      if (++advances > 2 * n) throw std::domain_error("Numerically inconsistent convex support sequence");
    }
    consider(i, opposite); consider(next, opposite);
    // A parallel opposite edge yields TWO antipodal vertices.
    if (height((opposite + 1) % n) == height(opposite)) {
      consider(i, (opposite + 1) % n); consider(next, (opposite + 1) % n);
    }
  }
  return result;
}

struct OrientedRectangle {
  std::array<Point, 4> corners{}; // CCW: bottom-left, bottom-right, top-right, top-left.
  double area = 0, width = 0, height = 0;
};

template<Coordinate T> inline std::optional<OrientedRectangle> minimum_area_rectangle(const ConvexPolygon2<T>& hull) {
  const auto p = hull.vertices();
  const auto n = p.size();
  if (n == 0) return std::nullopt;
  auto floating = [](Point2<T> point) { return Point{static_cast<double>(point.x), static_cast<double>(point.y)}; };
  if (n == 1) return OrientedRectangle{{floating(p[0]), floating(p[0]), floating(p[0]), floating(p[0])}, 0, 0, 0};
  if (n == 2) return OrientedRectangle{{floating(p[0]), floating(p[1]), floating(p[1]), floating(p[0])}, 0, distance(p[0], p[1]), 0};
  std::size_t maximum = 0, minimum = 0, opposite = 0;
  const auto initial_edge = p[1] - p[0];
  for (std::size_t j = 1; j < n; ++j) {
    if (dot(initial_edge, p[j] - p[0]) > dot(initial_edge, p[maximum] - p[0])) maximum = j;
    if (dot(initial_edge, p[j] - p[0]) < dot(initial_edge, p[minimum] - p[0])) minimum = j;
    if (detail::determinant(p[0], p[1], p[j]) > detail::determinant(p[0], p[1], p[opposite])) opposite = j;
  }
  std::array<std::size_t, 3> advances{};
  OrientedRectangle best;
  long double best_area = std::numeric_limits<long double>::infinity();
  for (std::size_t i = 0; i < n; ++i) {
    const auto next = (i + 1) % n;
    const auto edge = p[next] - p[i];
    auto along = [&](std::size_t j) { return dot(edge, p[j] - p[i]); };
    auto height = [&](std::size_t j) { return detail::determinant(p[i], p[next], p[j]); };
    auto advance = [&](std::size_t& index, auto value, std::size_t& budget) {
      while (value((index + 1) % n) > value(index)) {
        index = (index + 1) % n;
        if (++budget > 2 * n) throw std::domain_error("Numerically inconsistent convex support sequence");
      }
    };
    advance(maximum, along, advances[0]);
    advance(minimum, [&](std::size_t j) { return -along(j); }, advances[1]);
    advance(opposite, height, advances[2]);
    const long double length = std::hypot(static_cast<long double>(edge.x), static_cast<long double>(edge.y));
    const long double lo = static_cast<long double>(along(minimum)) / length;
    const long double hi = static_cast<long double>(along(maximum)) / length;
    const long double top = static_cast<long double>(height(opposite)) / length;
    const auto current_area = (hi - lo) * top;
    if (current_area >= best_area) continue;
    best_area = current_area;
    const long double ux = edge.x / length, uy = edge.y / length;
    auto corner = [&](long double x, long double y) { return detail::make_point(p[i].x + ux * x - uy * y, p[i].y + uy * x + ux * y); };
    best = {{corner(lo, 0), corner(hi, 0), corner(hi, top), corner(lo, top)},
            detail::checked_real(current_area), detail::checked_real(hi - lo), detail::checked_real(top)};
  }
  return best;
}

} // namespace algorithm_assets::geometry
