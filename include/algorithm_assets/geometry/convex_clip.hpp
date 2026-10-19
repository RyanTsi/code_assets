#pragma once

#include <algorithm_assets/geometry/convex_hull.hpp>
#include <algorithm_assets/geometry/linear.hpp>

#include <vector>

namespace algorithm_assets::geometry {

// Ordered boundary of the intersection with the CLOSED left half-plane.
// Integer input still produces floating constructed coordinates. The result
// may contain collinear vertices and is intentionally not a strict hull type.
template <Coordinate T>
inline std::vector<Point> clip_left(const ConvexPolygon2<T>& hull, const Line& line,
                                    Tolerance tolerance = Tolerance{}) {
    const auto vertices = hull.vertices();
    std::vector<Point> result;
    if (vertices.empty())
        return result;
    result.reserve(vertices.size() + 1);
    auto append = [&](Point p) {
        if (result.empty() || result.back() != p)
            result.push_back(p);
    };
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const auto a =
            Point{static_cast<double>(vertices[i].x), static_cast<double>(vertices[i].y)};
        const auto next = vertices[(i + 1) % vertices.size()];
        const auto b = Point{static_cast<double>(next.x), static_cast<double>(next.y)};
        const auto da = detail::signed_line_distance(line, a),
                   db = detail::signed_line_distance(line, b);
        const auto sa = tolerance.sign(da, detail::line_parameter(line, a)),
                   sb = tolerance.sign(db, detail::line_parameter(line, b));
        if (sa >= 0)
            append(a);
        if (sa * sb < 0) {
            const auto t = da / (da - db);
            append(detail::make_point(a.x + t * (static_cast<long double>(b.x) - a.x),
                                      a.y + t * (static_cast<long double>(b.y) - a.y)));
        }
    }
    if (result.size() > 1 && result.front() == result.back())
        result.pop_back();
    return result;
}

} // namespace algorithm_assets::geometry
