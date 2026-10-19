#pragma once

#include <algorithm_assets/geometry/primitives.hpp>

namespace algorithm_assets::geometry {

// A line is an anchor and a unit direction, never a zero-length segment.
class Line {
public:
    Line() = default; // The x axis is a valid default value.
    [[nodiscard]] static Line through(Point a, Point b) {
        detail::validate(a);
        detail::validate(b);
        const long double dx = static_cast<long double>(b.x) - a.x,
                          dy = static_cast<long double>(b.y) - a.y;
        const auto length = std::hypot(dx, dy);
        if (length == 0)
            throw std::invalid_argument("Distinct points required for a line");
        return Line(a, {static_cast<double>(dx / length), static_cast<double>(dy / length)});
    }
    [[nodiscard]] static Line from_direction(Point anchor, Vector direction) {
        detail::validate(anchor);
        return Line(anchor, unit(direction));
    }
    [[nodiscard]] static Line from_equation(double a, double b, double c) {
        if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c))
            throw std::invalid_argument("Line coefficients must be finite");
        const auto length = std::hypot(static_cast<long double>(a), static_cast<long double>(b));
        if (length == 0)
            throw std::invalid_argument("Line normal must be nonzero");
        const auto nx = a / length, ny = b / length, offset = c / length;
        return Line(detail::make_point(nx * offset, ny * offset),
                    {static_cast<double>(-ny), static_cast<double>(nx)});
    }
    [[nodiscard]] Point anchor() const noexcept {
        return anchor_;
    }
    [[nodiscard]] Vector direction() const noexcept {
        return direction_;
    }

private:
    Line(Point anchor, Vector direction) : anchor_(anchor), direction_(direction) {
    }
    Point anchor_{};
    Vector direction_{1, 0};
};
template <Coordinate T> struct Segment2 {
    Point2<T> first{}, second{};
};
using Segment = Segment2<double>;
using IntegerSegment = Segment2<std::int64_t>;
enum class SegmentIntersectionKind { none, point, overlap };
struct SegmentIntersection {
    SegmentIntersectionKind kind = SegmentIntersectionKind::none;
    Point first{}, second{}; // For overlap: lexicographically ordered endpoints.
};

namespace detail {
inline long double signed_line_distance(const Line& line, Point p) {
    const auto a = line.anchor();
    const auto u = line.direction();
    return static_cast<long double>(u.x) * (static_cast<long double>(p.y) - a.y) -
           static_cast<long double>(u.y) * (static_cast<long double>(p.x) - a.x);
}
inline long double line_parameter(const Line& line, Point p) {
    const auto a = line.anchor();
    const auto u = line.direction();
    return static_cast<long double>(u.x) * (static_cast<long double>(p.x) - a.x) +
           static_cast<long double>(u.y) * (static_cast<long double>(p.y) - a.y);
}
} // namespace detail

inline Point projection(Point p, const Line& line) {
    detail::validate(p);
    const auto a = line.anchor();
    const auto u = line.direction();
    const auto t = detail::line_parameter(line, p);
    return detail::make_point(a.x + t * u.x, a.y + t * u.y);
}
inline Point reflection(Point p, const Line& line) {
    const auto foot = projection(p, line);
    return detail::make_point(2.L * foot.x - p.x, 2.L * foot.y - p.y);
}
inline Line perpendicular_bisector(Point a, Point b) {
    const auto direction = perpendicular_ccw(Line::through(a, b).direction());
    return Line::from_direction(detail::make_point((static_cast<long double>(a.x) + b.x) / 2,
                                                   (static_cast<long double>(a.y) + b.y) / 2),
                                direction);
}
inline bool parallel(const Line& a, const Line& b, Tolerance tolerance = Tolerance{}) {
    return tolerance.sign(cross(a.direction(), b.direction())) == 0;
}
inline bool perpendicular(const Line& a, const Line& b, Tolerance tolerance = Tolerance{}) {
    return tolerance.sign(dot(a.direction(), b.direction())) == 0;
}
inline double distance(const Line& line, Point p) {
    detail::validate(p);
    return detail::checked_real(std::abs(detail::signed_line_distance(line, p)));
}
inline double distance(Point p, const Line& line) {
    return distance(line, p);
}
inline bool contains(const Line& line, Point p, Tolerance tolerance = Tolerance{}) {
    detail::validate(p);
    return tolerance.sign(detail::signed_line_distance(line, p), detail::line_parameter(line, p)) ==
           0;
}
inline IntersectionSet<Point, 1> intersection(const Line& a, const Line& b,
                                              Tolerance tolerance = Tolerance{}) {
    IntersectionSet<Point, 1> result;
    const auto determinant = cross(a.direction(), b.direction());
    if (tolerance.sign(determinant) == 0) {
        if (contains(a, b.anchor(), tolerance))
            result.kind = IntersectionKind::infinite;
        return result;
    }
    const auto u = a.direction(), v = b.direction();
    const auto aa = a.anchor(), ba = b.anchor();
    const auto t = ((static_cast<long double>(ba.x) - aa.x) * v.y -
                    (static_cast<long double>(ba.y) - aa.y) * v.x) /
                   determinant;
    result.add(detail::make_point(aa.x + t * u.x, aa.y + t * u.y));
    return result;
}
inline double distance(const Line& a, const Line& b, Tolerance tolerance = Tolerance{}) {
    return parallel(a, b, tolerance) ? distance(a, b.anchor()) : 0;
}
inline Point closest_point(const Segment& segment, Point p) {
    detail::validate(segment.first);
    detail::validate(segment.second);
    detail::validate(p);
    const long double dx = static_cast<long double>(segment.second.x) - segment.first.x;
    const long double dy = static_cast<long double>(segment.second.y) - segment.first.y;
    const auto norm = dx * dx + dy * dy;
    if (norm == 0)
        return segment.first;
    const auto t = std::clamp(((static_cast<long double>(p.x) - segment.first.x) * dx +
                               (static_cast<long double>(p.y) - segment.first.y) * dy) /
                                  norm,
                              0.L, 1.L);
    return detail::make_point(segment.first.x + t * dx, segment.first.y + t * dy);
}
inline double distance(const Segment& segment, Point p) {
    return distance(closest_point(segment, p), p);
}
inline double distance(Point p, const Segment& segment) {
    return distance(segment, p);
}
inline bool contains(const Segment& segment, Point p, Tolerance tolerance = Tolerance{}) {
    // Distance tolerance is local to segment length, not the global origin.
    return tolerance.sign(distance(segment, p), distance(segment.first, segment.second)) == 0;
}
inline bool contains(const IntegerSegment& segment, IntegerPoint p) {
    detail::validate(segment.first);
    detail::validate(segment.second);
    detail::validate(p);
    return detail::on_segment_unchecked(segment.first, segment.second, p);
}
inline bool intersects(const IntegerSegment& a, const IntegerSegment& b) {
    detail::validate(a.first);
    detail::validate(a.second);
    detail::validate(b.first);
    detail::validate(b.second);
    const int a1 = detail::turn_unchecked(a.first, a.second, b.first),
              a2 = detail::turn_unchecked(a.first, a.second, b.second);
    const int b1 = detail::turn_unchecked(b.first, b.second, a.first),
              b2 = detail::turn_unchecked(b.first, b.second, a.second);
    if (a1 == 0 && contains(a, b.first))
        return true;
    if (a2 == 0 && contains(a, b.second))
        return true;
    if (b1 == 0 && contains(b, a.first))
        return true;
    if (b2 == 0 && contains(b, a.second))
        return true;
    return a1 * a2 < 0 && b1 * b2 < 0;
}
inline bool intersects(const Line& line, const Segment& segment, Tolerance tolerance);
inline SegmentIntersection intersection(const Segment& a, const Segment& b,
                                        Tolerance tolerance = Tolerance{}) {
    detail::validate(a.first);
    detail::validate(a.second);
    detail::validate(b.first);
    detail::validate(b.second);
    if (a.first == a.second)
        return contains(b, a.first, tolerance)
                   ? SegmentIntersection{SegmentIntersectionKind::point, a.first, a.first}
                   : SegmentIntersection{};
    if (b.first == b.second)
        return contains(a, b.first, tolerance)
                   ? SegmentIntersection{SegmentIntersectionKind::point, b.first, b.first}
                   : SegmentIntersection{};
    const auto first = Line::through(a.first, a.second), second = Line::through(b.first, b.second);
    if (!intersects(first, b, tolerance) || !intersects(second, a, tolerance))
        return {};
    const auto crossing = intersection(first, second, tolerance);
    if (crossing.kind == IntersectionKind::finite) {
        const auto p = crossing.values[0];
        return contains(a, p, tolerance) && contains(b, p, tolerance)
                   ? SegmentIntersection{SegmentIntersectionKind::point, p, p}
                   : SegmentIntersection{};
    }
    if (crossing.kind == IntersectionKind::none)
        return {};
    auto lo1 = a.first, hi1 = a.second, lo2 = b.first, hi2 = b.second;
    if (detail::lex_less(hi1, lo1))
        std::swap(lo1, hi1);
    if (detail::lex_less(hi2, lo2))
        std::swap(lo2, hi2);
    const auto lo = detail::lex_less(lo1, lo2) ? lo2 : lo1;
    const auto hi = detail::lex_less(hi1, hi2) ? hi1 : hi2;
    if (detail::lex_less(hi, lo) && !almost_equal(lo, hi, tolerance))
        return {};
    if (almost_equal(lo, hi, tolerance))
        return {SegmentIntersectionKind::point, lo, lo};
    return {SegmentIntersectionKind::overlap, lo, hi};
}
inline bool intersects(const Segment& a, const Segment& b, Tolerance tolerance = Tolerance{}) {
    return intersection(a, b, tolerance).kind != SegmentIntersectionKind::none;
}
inline bool intersects(const Line& line, const Segment& segment,
                       Tolerance tolerance = Tolerance{}) {
    detail::validate(segment.first);
    detail::validate(segment.second);
    const int first = tolerance.sign(detail::signed_line_distance(line, segment.first),
                                     detail::line_parameter(line, segment.first));
    const int second = tolerance.sign(detail::signed_line_distance(line, segment.second),
                                      detail::line_parameter(line, segment.second));
    return first * second <= 0;
}
inline double distance(const Line& line, const Segment& segment,
                       Tolerance tolerance = Tolerance{}) {
    return intersects(line, segment, tolerance)
               ? 0
               : std::min(distance(line, segment.first), distance(line, segment.second));
}
inline double distance(const Segment& segment, const Line& line,
                       Tolerance tolerance = Tolerance{}) {
    return distance(line, segment, tolerance);
}
inline double distance(const Segment& a, const Segment& b, Tolerance tolerance = Tolerance{}) {
    return intersects(a, b, tolerance) ? 0
                                       : std::min({distance(a, b.first), distance(a, b.second),
                                                   distance(b, a.first), distance(b, a.second)});
}

} // namespace algorithm_assets::geometry
