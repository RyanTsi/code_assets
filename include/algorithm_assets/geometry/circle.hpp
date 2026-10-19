#pragma once

#include <algorithm_assets/geometry/polygon.hpp>

#include <numbers>
#include <optional>

namespace algorithm_assets::geometry {

class Circle {
public:
    Circle() = default; // A point circle at the origin.
    Circle(Point center, double radius) : center_(center), radius_(radius) {
        detail::validate(center);
        if (!std::isfinite(radius) || radius < 0 || radius > 1e100)
            throw std::invalid_argument("Radius must be finite and within [0,1e100]");
    }
    [[nodiscard]] Point center() const noexcept {
        return center_;
    }
    [[nodiscard]] double radius() const noexcept {
        return radius_;
    }

private:
    Point center_{};
    double radius_ = 0;
};

inline Containment locate(const Circle& circle, Point p, Tolerance tolerance = Tolerance{}) {
    const auto comparison = tolerance.compare(distance(circle.center(), p), circle.radius());
    return comparison < 0    ? Containment::inside
           : comparison == 0 ? Containment::boundary
                             : Containment::outside;
}
inline IntersectionSet<Point, 2> intersection(const Circle& circle, const Line& line,
                                              Tolerance tolerance = Tolerance{}) {
    IntersectionSet<Point, 2> result;
    const auto signed_distance = detail::signed_line_distance(line, circle.center());
    const auto d = std::abs(signed_distance), r = static_cast<long double>(circle.radius());
    const auto comparison = tolerance.compare(d, r);
    if (comparison > 0)
        return result;
    const auto u = line.direction(), normal = perpendicular_ccw(u);
    const auto c = circle.center();
    const auto foot =
        detail::make_point(c.x - normal.x * signed_distance, c.y - normal.y * signed_distance);
    if (comparison == 0 || r == 0) {
        result.add(foot);
        return result;
    }
    const auto offset = std::sqrt(std::max(0.L, (r - d) * (r + d)));
    result.add(detail::make_point(foot.x - offset * u.x, foot.y - offset * u.y));
    result.add(detail::make_point(foot.x + offset * u.x, foot.y + offset * u.y));
    return result;
}
inline IntersectionSet<Point, 2> intersection(const Circle& circle, const Segment& segment,
                                              Tolerance tolerance = Tolerance{}) {
    detail::validate(segment.first);
    detail::validate(segment.second);
    IntersectionSet<Point, 2> result;
    if (segment.first == segment.second) {
        if (locate(circle, segment.first, tolerance) == Containment::boundary)
            result.add(segment.first);
        return result;
    }
    const auto candidates =
        intersection(circle, Line::through(segment.first, segment.second), tolerance);
    for (std::size_t i = 0; i < candidates.count; ++i)
        if (contains(segment, candidates.values[i], tolerance))
            result.add(candidates.values[i]);
    return result;
}
inline IntersectionSet<Point, 2> intersection(const Circle& first, const Circle& second,
                                              Tolerance tolerance = Tolerance{}) {
    IntersectionSet<Point, 2> result;
    const auto a = first.center(), b = second.center();
    const long double r = first.radius(), s = second.radius();
    const long double dx = static_cast<long double>(b.x) - a.x,
                      dy = static_cast<long double>(b.y) - a.y;
    const auto d = std::hypot(dx, dy);
    if (d == 0) {
        if (r == s) {
            if (r == 0)
                result.add(a);
            else
                result.kind = IntersectionKind::infinite;
        }
        return result;
    }
    const int outer = tolerance.compare(d, r + s), inner = tolerance.compare(d, std::abs(r - s));
    if (outer > 0 || inner < 0)
        return result;
    const auto along = (d + (r - s) * (r + s) / d) / 2;
    const auto ux = dx / d, uy = dy / d;
    const auto base = detail::make_point(a.x + along * ux, a.y + along * uy);
    if (outer == 0 || inner == 0 || r == 0 || s == 0) {
        result.add(base);
        return result;
    }
    const auto height = std::sqrt(std::max(0.L, (r - along) * (r + along)));
    result.add(detail::make_point(base.x - uy * height, base.y + ux * height));
    result.add(detail::make_point(base.x + uy * height, base.y - ux * height));
    return result;
}
inline bool intersects(const Circle& circle, const Line& line, Tolerance tolerance = Tolerance{}) {
    return tolerance.compare(distance(line, circle.center()), circle.radius()) <= 0;
}
// These are CIRCUMFERENCE intersections, not disk containment tests.
inline bool intersects(const Circle& circle, const Segment& segment,
                       Tolerance tolerance = Tolerance{}) {
    return intersection(circle, segment, tolerance).count != 0;
}
inline IntersectionSet<Point, 2> tangent_points(const Circle& circle, Point p,
                                                Tolerance tolerance = Tolerance{}) {
    detail::validate(p);
    IntersectionSet<Point, 2> result;
    const auto c = circle.center();
    const long double dx = static_cast<long double>(p.x) - c.x,
                      dy = static_cast<long double>(p.y) - c.y;
    const auto d = std::hypot(dx, dy), r = static_cast<long double>(circle.radius());
    if (r == 0) {
        result.add(c);
        return result;
    }
    const auto relation = tolerance.compare(d, r);
    if (relation < 0 || d == 0)
        return result;
    const auto ux = dx / d, uy = dy / d;
    if (relation == 0) {
        result.add(detail::make_point(c.x + r * ux, c.y + r * uy));
        return result;
    }
    const auto ratio = r / d, along = r * ratio;
    const auto height = r * std::sqrt(std::max(0.L, (1 - ratio) * (1 + ratio)));
    result.add(detail::make_point(c.x + along * ux - height * uy, c.y + along * uy + height * ux));
    result.add(detail::make_point(c.x + along * ux + height * uy, c.y + along * uy - height * ux));
    return result;
}
inline IntersectionSet<Line, 4> common_tangents(const Circle& first, const Circle& second,
                                                Tolerance tolerance = Tolerance{}) {
    IntersectionSet<Line, 4> result;
    const auto a = first.center(), b = second.center();
    const long double r = first.radius(), s = second.radius();
    const long double dx = static_cast<long double>(b.x) - a.x,
                      dy = static_cast<long double>(b.y) - a.y;
    const auto d = std::hypot(dx, dy);
    if (d == 0) {
        if (r == s)
            result.kind = IntersectionKind::infinite;
        return result;
    }
    if (r == 0 && s == 0) {
        result.add(Line::through(a, b));
        return result;
    }
    if (r == 0 || s == 0) {
        const auto point = r == 0 ? a : b;
        const auto circle = r == 0 ? second : first;
        const auto contact = tangent_points(circle, point, tolerance);
        for (std::size_t i = 0; i < contact.count; ++i) {
            if (contact.count == 1)
                result.add(Line::from_direction(
                    contact.values[i],
                    perpendicular_ccw(Line::through(circle.center(), point).direction())));
            else
                result.add(Line::through(point, contact.values[i]));
        }
        return result;
    }
    const auto ux = dx / d, uy = dy / d;
    for (const int side : {1, -1}) {
        const auto ratio = (r - side * s) / d;
        const auto relation = tolerance.compare(std::abs(ratio), 1);
        if (relation > 0)
            continue;
        const auto h = std::clamp(ratio, -1.L, 1.L);
        const auto offset = relation == 0 ? 0.L : std::sqrt(std::max(0.L, (1 - h) * (1 + h)));
        for (const int sign : {-1, 1}) {
            const auto nx = ux * h - sign * uy * offset, ny = uy * h + sign * ux * offset;
            result.add(Line::from_direction(detail::make_point(a.x + r * nx, a.y + r * ny),
                                            {static_cast<double>(-ny), static_cast<double>(nx)}));
            if (offset == 0)
                break;
        }
    }
    return result;
}
// nullopt denotes infinitely many common tangent lines.
inline std::optional<std::size_t> common_tangent_count(const Circle& first, const Circle& second,
                                                       Tolerance tolerance = Tolerance{}) {
    const auto result = common_tangents(first, second, tolerance);
    return result.kind == IntersectionKind::infinite ? std::nullopt
                                                     : std::optional<std::size_t>(result.count);
}
inline std::optional<Circle> circumcircle(Point a, Point b, Point c,
                                          Tolerance tolerance = Tolerance{}) {
    detail::validate(a);
    detail::validate(b);
    detail::validate(c);
    long double bx = static_cast<long double>(b.x) - a.x, by = static_cast<long double>(b.y) - a.y;
    long double cx = static_cast<long double>(c.x) - a.x, cy = static_cast<long double>(c.y) - a.y;
    const auto scale = std::max({std::abs(bx), std::abs(by), std::abs(cx), std::abs(cy)});
    if (scale == 0)
        return std::nullopt;
    bx /= scale;
    by /= scale;
    cx /= scale;
    cy /= scale;
    const auto determinant = bx * cy - by * cx;
    if (tolerance.sign(determinant, std::abs(bx * cy) + std::abs(by * cx)) == 0)
        return std::nullopt;
    const auto nb = bx * bx + by * by, nc = cx * cx + cy * cy;
    const auto center = detail::make_point(a.x + scale * (nb * cy - nc * by) / (2 * determinant),
                                           a.y + scale * (bx * nc - cx * nb) / (2 * determinant));
    return Circle(center, distance(a, center));
}
inline std::optional<Circle> incircle(Point a, Point b, Point c,
                                      Tolerance tolerance = Tolerance{}) {
    if (orientation(a, b, c, tolerance) == Turn::collinear)
        return std::nullopt;
    const long double ab = distance(a, b), bc = distance(b, c), ca = distance(c, a);
    const auto sum = ab + bc + ca;
    const auto center =
        detail::make_point(a.x + (static_cast<long double>(b.x) - a.x) * (ca / sum) +
                               (static_cast<long double>(c.x) - a.x) * (ab / sum),
                           a.y + (static_cast<long double>(b.y) - a.y) * (ca / sum) +
                               (static_cast<long double>(c.y) - a.y) * (ab / sum));
    return Circle(center, distance(Line::through(a, b), center));
}
inline IntersectionSet<Point, 2> centers_through(Point a, Point b, double radius,
                                                 Tolerance tolerance = Tolerance{}) {
    return intersection(Circle(a, radius), Circle(b, radius), tolerance);
}
inline double overlap_area(const Circle& first, const Circle& second) {
    const long double r = first.radius(), s = second.radius(),
                      d = distance(first.center(), second.center());
    if (r == 0 || s == 0 || d >= r + s)
        return 0;
    if (d <= std::abs(r - s))
        return detail::checked_real(std::numbers::pi_v<long double> * std::min(r, s) *
                                    std::min(r, s));
    auto segment_area = [d](long double a, long double b) {
        const auto cosine = std::clamp((d + (a - b) * (a + b) / d) / (2 * a), -1.L, 1.L);
        const auto theta = 2 * std::acos(cosine);
        const auto t2 = theta * theta;
        // theta - sin(theta) loses most significant digits near external tangency.
        const auto angle_term = theta < 1e-3L ? theta * t2 * (1.L / 6 - t2 / 120 + t2 * t2 / 5040)
                                              : theta - std::sin(theta);
        return a * a * angle_term / 2;
    };
    return detail::checked_real(segment_area(r, s) + segment_area(s, r));
}

enum class RegionIntersection { disjoint, touching, overlapping };
// CLOSED disk versus a SIMPLE polygon's filled region. "overlapping" means
// positive area; a zero-radius disk or a 0/1/2-vertex region cannot have it.
inline RegionIntersection disk_polygon_intersection(const Circle& disk,
                                                    std::span<const Point> polygon,
                                                    Tolerance tolerance = Tolerance{}) {
    detail::validate(polygon);
    polygon = detail::open_boundary(polygon);
    if (polygon.empty())
        return RegionIntersection::disjoint;
    const auto position = locate(polygon, disk.center(), tolerance);
    if (disk.radius() == 0)
        return position == Containment::outside ? RegionIntersection::disjoint
                                                : RegionIntersection::touching;
    double boundary_distance = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < polygon.size(); ++i)
        boundary_distance = std::min(
            boundary_distance,
            distance(Segment{polygon[i], polygon[(i + 1) % polygon.size()]}, disk.center()));
    const auto relation = tolerance.compare(boundary_distance, disk.radius());
    if (polygon.size() < 3)
        return relation <= 0 ? RegionIntersection::touching : RegionIntersection::disjoint;
    if (position != Containment::outside || relation < 0)
        return RegionIntersection::overlapping;
    return relation == 0 ? RegionIntersection::touching : RegionIntersection::disjoint;
}

} // namespace algorithm_assets::geometry
