#pragma once

#include <algorithm_assets/geometry/linear.hpp>
#include <algorithm_assets/numeric/compensated_sum.hpp>

#include <numbers>
#include <optional>
#include <vector>

namespace algorithm_assets::geometry {

namespace detail {
template <Coordinate T>
inline std::span<const Point2<T>> open_boundary(std::span<const Point2<T>> points) {
    if (points.size() > 1 && points.front() == points.back())
        return points.first(points.size() - 1);
    return points;
}
} // namespace detail

template <Coordinate T> inline double triangle_area(Point2<T> a, Point2<T> b, Point2<T> c) {
    detail::validate(a);
    detail::validate(b);
    detail::validate(c);
    return detail::checked_real(std::abs(static_cast<long double>(detail::determinant(a, b, c))) /
                                2);
}
template <Coordinate T> inline double signed_area(std::span<const Point2<T>> points) {
    detail::validate(points);
    if (points.size() < 3)
        return 0;
    // Translate the shoelace formula to the first vertex to reduce cancellation.
    CompensatedSum sum;
    for (std::size_t i = 1; i + 1 < points.size(); ++i)
        sum.add(detail::checked_real(
            static_cast<long double>(detail::determinant(points[0], points[i], points[i + 1])) /
            2));
    return sum.value();
}
template <Coordinate T> inline double area(std::span<const Point2<T>> points) {
    return std::abs(signed_area(points));
}
template <Coordinate T> inline double perimeter(std::span<const Point2<T>> points) {
    detail::validate(points);
    CompensatedSum sum;
    for (std::size_t i = 0; i < points.size(); ++i)
        sum.add(distance(points[i], points[(i + 1) % points.size()]));
    return sum.value();
}
// Convexity here assumes a SIMPLE boundary; this is not a self-intersection test.
template <Coordinate T>
inline bool is_convex(std::span<const Point2<T>> points, bool allow_collinear = false) {
    detail::validate(points);
    points = detail::open_boundary(points);
    if (points.size() < 3)
        return false;
    int direction = 0;
    for (std::size_t i = 0; i < points.size(); ++i) {
        const auto next = (i + 1) % points.size();
        if (points[i] == points[next])
            return false;
        const auto turn =
            detail::turn_unchecked(points[i], points[next], points[(i + 2) % points.size()]);
        if (!turn && !allow_collinear)
            return false;
        if (turn && direction && turn != direction)
            return false;
        if (turn)
            direction = turn;
    }
    return direction != 0;
}
// Even-odd fill rule; direction-independent, with an explicit boundary result.
template <Coordinate T> inline Containment locate(std::span<const Point2<T>> points, Point2<T> p) {
    detail::validate(points);
    detail::validate(p);
    bool inside = false;
    for (std::size_t i = 0; i < points.size(); ++i) {
        auto a = points[i], b = points[(i + 1) % points.size()];
        if (detail::on_segment_unchecked(a, b, p))
            return Containment::boundary;
        if (a.y > b.y)
            std::swap(a, b);
        if (a.y <= p.y && p.y < b.y && detail::turn_unchecked(a, b, p) > 0)
            inside = !inside;
    }
    return inside ? Containment::inside : Containment::outside;
}
inline Containment locate(std::span<const Point> points, Point p, Tolerance tolerance) {
    detail::validate(points);
    detail::validate(p);
    bool inside = false;
    for (std::size_t i = 0; i < points.size(); ++i) {
        auto a = points[i], b = points[(i + 1) % points.size()];
        if (contains(Segment{a, b}, p, tolerance))
            return Containment::boundary;
        if (a.y > b.y)
            std::swap(a, b);
        // Tolerance is ONLY used for the boundary distance; avoid double-counting
        // horizontal/ray vertices by retaining the exact half-open crossing test.
        if (a.y <= p.y && p.y < b.y && detail::turn_unchecked(a, b, p) > 0)
            inside = !inside;
    }
    return inside ? Containment::inside : Containment::outside;
}
template <Coordinate T>
inline std::optional<Point2<T>> lowest_leftmost(std::span<const Point2<T>> points) {
    detail::validate(points);
    if (points.empty())
        return std::nullopt;
    return *std::min_element(points.begin(), points.end(), [](auto a, auto b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
    });
}
inline void sort_by_angle(std::span<Point> points, Point origin = {}) {
    detail::validate(std::span<const Point>(points));
    detail::validate(origin);
    struct Key {
        Point point;
        long double angle;
    };
    std::vector<Key> keys;
    keys.reserve(points.size());
    for (const auto p : points) {
        auto angle = p == origin ? 0.L
                                 : std::atan2(static_cast<long double>(p.y) - origin.y,
                                              static_cast<long double>(p.x) - origin.x);
        if (angle < 0)
            angle += 2 * std::numbers::pi_v<long double>;
        keys.push_back({p, angle});
    }
    // Precomputed scalar keys are transitive even for almost parallel rays.
    std::sort(keys.begin(), keys.end(), [origin](const auto& a, const auto& b) {
        if (a.angle != b.angle)
            return a.angle < b.angle;
        const auto da = detail::squared_unchecked(a.point, origin),
                   db = detail::squared_unchecked(b.point, origin);
        if (da != db)
            return da < db;
        return detail::lex_less(a.point, b.point);
    });
    for (std::size_t i = 0; i < points.size(); ++i)
        points[i] = keys[i].point;
}

} // namespace algorithm_assets::geometry
