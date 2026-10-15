#pragma once

#include <algorithm_assets/geometry/primitives.hpp>

#include <utility>
#include <vector>

namespace algorithm_assets::geometry {

namespace detail { struct ConvexPolygonAccess; }

template<Coordinate T> class ConvexPolygon2 {
public:
  ConvexPolygon2() = default;
  // A value parameter lets callers choose copying or transferring ownership.
  [[nodiscard]] static ConvexPolygon2 from_points(std::vector<Point2<T>> points) {
    detail::validate(std::span<const Point2<T>>(points));
    std::sort(points.begin(), points.end(), detail::lex_less<T>);
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if (points.size() <= 2) {
      // Do not retain an n-point buffer for an all-duplicate/collinear result.
      if (points.capacity() > points.size())
        return ConvexPolygon2(std::vector<Point2<T>>(points.begin(), points.end()));
      return ConvexPolygon2(std::move(points));
    }
    std::vector<Point2<T>> hull;
    if (points.size() == hull.max_size()) throw std::length_error("Hull capacity exceeds vector max_size");
    hull.reserve(points.size() + 1);
    for (const auto p : points) {
      while (hull.size() >= 2 && detail::turn_unchecked(hull[hull.size() - 2], hull.back(), p) <= 0) hull.pop_back();
      hull.push_back(p);
    }
    const auto lower_size = hull.size();
    for (std::size_t i = points.size() - 1; i-- > 0;) {
      while (hull.size() > lower_size && detail::turn_unchecked(hull[hull.size() - 2], hull.back(), points[i]) <= 0) hull.pop_back();
      hull.push_back(points[i]);
    }
    hull.pop_back(); // Remove the repeated first vertex.
    // Free the sorted input BEFORE compacting a sparse output, so compaction
    // cannot increase the build's O(n) peak payload. Avoid an extra h-point
    // copy for dense hulls; their capacity is already at most twice h.
    std::vector<Point2<T>>{}.swap(points);
    if (hull.capacity() > 2 * hull.size())
      return ConvexPolygon2(std::vector<Point2<T>>(hull.begin(), hull.end()));
    return ConvexPolygon2(std::move(hull));
  }
  [[nodiscard]] static ConvexPolygon2 from_points(std::span<const Point2<T>> points) {
    return from_points(std::vector<Point2<T>>(points.begin(), points.end()));
  }
  // Validate an existing strict convex boundary, rather than trusting a flag.
  // Accept either winding and one repeated closing vertex. O(n log n).
  [[nodiscard]] static ConvexPolygon2 from_boundary(std::span<const Point2<T>> boundary) {
    if (boundary.size() > 1 && boundary.front() == boundary.back()) boundary = boundary.first(boundary.size() - 1);
    auto hull = from_points(boundary);
    if (boundary.size() != hull.size()) throw std::invalid_argument("Boundary must be strict convex without duplicate/collinear vertices");
    if (boundary.empty()) return hull;
    const auto first = std::min_element(boundary.begin(), boundary.end(), detail::lex_less<T>);
    const auto start = static_cast<std::size_t>(first - boundary.begin()), n = boundary.size();
    bool forward = true, backward = true;
    for (std::size_t i = 0; i < n; ++i) {
      forward = forward && boundary[(start + i) % n] == hull.points_[i];
      backward = backward && boundary[(start + n - i) % n] == hull.points_[i];
    }
    if (!forward && !backward) throw std::invalid_argument("Boundary order is not convex");
    return hull;
  }
  [[nodiscard]] std::span<const Point2<T>> vertices() const noexcept { return points_; }
  [[nodiscard]] std::size_t size() const noexcept { return points_.size(); }
  [[nodiscard]] std::size_t storage_bytes() const noexcept { return points_.capacity() * sizeof(Point2<T>); }
private:
  friend struct detail::ConvexPolygonAccess;
  explicit ConvexPolygon2(std::vector<Point2<T>> points) : points_(std::move(points)) {}
  std::vector<Point2<T>> points_;
};
using ConvexPolygon = ConvexPolygon2<double>;
using IntegerConvexPolygon = ConvexPolygon2<std::int64_t>;

// Internal construction for algorithms that establish canonical strict hull
// invariants themselves; never exposed as an unchecked public factory.
namespace detail {
struct ConvexPolygonAccess {
  template<Coordinate T> static ConvexPolygon2<T> make(std::vector<Point2<T>> points) {
    return ConvexPolygon2<T>(std::move(points));
  }
};
}

template<Coordinate T> inline ConvexPolygon2<T> convex_hull(std::span<const Point2<T>> points) { return ConvexPolygon2<T>::from_points(points); }

// Fan binary search: O(log h), no allocation. Exact for the integer kernel.
template<Coordinate T> inline Containment locate(const ConvexPolygon2<T>& hull, Point2<T> p) {
  detail::validate(p);
  const auto v = hull.vertices();
  if (v.empty()) return Containment::outside;
  if (v.size() == 1) return p == v[0] ? Containment::boundary : Containment::outside;
  if (v.size() == 2) return detail::on_segment_unchecked(v[0], v[1], p) ? Containment::boundary : Containment::outside;
  const auto first = detail::turn_unchecked(v[0], v[1], p), last = detail::turn_unchecked(v[0], v.back(), p);
  if (first < 0 || last > 0) return Containment::outside;
  if (first == 0) return detail::on_segment_unchecked(v[0], v[1], p) ? Containment::boundary : Containment::outside;
  if (last == 0) return detail::on_segment_unchecked(v[0], v.back(), p) ? Containment::boundary : Containment::outside;
  std::size_t left = 1, right = v.size() - 1;
  while (right - left > 1) {
    const auto middle = left + (right - left) / 2;
    if (detail::turn_unchecked(v[0], v[middle], p) >= 0) left = middle;
    else right = middle;
  }
  const auto side = detail::turn_unchecked(v[left], v[right], p);
  return side < 0 ? Containment::outside : side == 0 ? Containment::boundary : Containment::inside;
}

} // namespace algorithm_assets::geometry
