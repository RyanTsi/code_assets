#include <algorithm_assets/geometry/geometry2d.hpp>

#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    namespace g = algorithm_assets::geometry;
    const std::vector<g::IntegerPoint> points{{0, 0}, {4, 0}, {4, 3}, {0, 3}, {2, 1}, {0, 0}};
    const auto hull = g::convex_hull<std::int64_t>(points);
    const auto diameter = g::convex_diameter(hull);
    const auto rectangle = g::minimum_area_rectangle(hull);
    g::ClosestPairWorkspace<std::int64_t> workspace;
    const auto closest = workspace.solve(points);
    std::cout << "hull vertices: " << hull.size()
              << "\ndiameter squared: " << diameter->squared_distance
              << "\nrectangle area: " << rectangle->area
              << "\nclosest squared: " << closest->squared_distance << '\n';
    std::cout << "point inside: "
              << (g::locate(hull, g::IntegerPoint{2, 1}) == g::Containment::inside) << '\n';

    const auto clipped = g::clip_left(hull, g::Line::through({2, 0}, {2, 1}));
    std::cout << "clipped area: " << g::area<double>(clipped) << '\n';
    const auto contact = g::tangent_points(g::Circle({0, 0}, 1), {2, 0});
    std::cout << "tangent points: " << contact.count << '\n';
    const auto intersection = g::intersection(g::Circle({0, 0}, 1), g::Circle({1, 0}, 1));
    for (std::size_t i = 0; i < intersection.count; ++i)
        std::cout << intersection.values[i].x << ' ' << intersection.values[i].y << '\n';
}
