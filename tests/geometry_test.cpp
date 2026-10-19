#include <algorithm_assets/geometry/geometry2d.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <source_location>
#include <string>
#include <vector>

namespace g = algorithm_assets::geometry;
namespace {
void require(bool condition, std::source_location where = std::source_location::current()) {
    if (!condition)
        throw std::runtime_error(std::string(where.file_name()) + ":" +
                                 std::to_string(where.line()) + ": check failed");
}
bool near(long double a, long double b, long double tolerance = 1e-9L) {
    return std::abs(a - b) <= tolerance * std::max({1.L, std::abs(a), std::abs(b)});
}
template <class Error, class Function> void throws(Function function) {
    bool caught = false;
    try {
        function();
    } catch (const Error&) {
        caught = true;
    }
    require(caught);
}

void linear_tests() {
    const g::Point a{}, b{4, 0}, p{2, 3};
    require(a == g::Point{0, 0});
    require(g::perpendicular_ccw(g::Vector{1, 0}) == g::Vector{0, 1});
    require(g::relative_position(g::IntegerPoint{0, 0}, {1, 0}, {-1, 0}) ==
            g::RelativePosition::before);
    require(g::relative_position(g::IntegerPoint{0, 0}, {1, 0}, {2, 0}) ==
            g::RelativePosition::after);
    require(g::relative_position(g::Point{0, 0}, {0, 0}, {1, 0}) ==
            g::RelativePosition::degenerate);
    const auto rotated = g::rotate({3, 4}, .723);
    require(near(g::dot(rotated, rotated), 25));
    require(g::almost_equal(g::polar(2, 0), {2, 0}));
    const auto line = g::Line::through(a, b);
    require(g::almost_equal(g::projection(p, line), {2, 0}));
    require(g::almost_equal(g::reflection(p, line), {2, -3}));
    require(near(g::distance(line, p), 3));
    require(near(g::distance(g::Segment{a, b}, p), 3));
    require(
        near(g::distance(p, g::Segment{a, b}), 3)); // Regression: old overload recursed forever.
    require(near(g::distance(g::Segment{a, a}, p), std::sqrt(13.)));
    require(g::closest_point(g::Segment{a, b}, {8, 2}) == b);
    require(g::contains(g::Line::from_equation(1, 1, 0), {2, -2}));
    require(g::perpendicular(line, g::perpendicular_bisector(a, b)));
    require(g::intersection(line, g::Line::through({2, -1}, {2, 1})).values[0] == g::Point{2, 0});
    require(g::intersection(line, g::Line::through({1, 0}, {9, 0})).kind ==
            g::IntersectionKind::infinite);
    require(g::intersection(line, g::Line::through({0, 1}, {4, 1})).kind ==
            g::IntersectionKind::none);
    const auto overlap = g::intersection(g::Segment{{0, 0}, {4, 0}}, g::Segment{{3, 0}, {1, 0}});
    require(overlap.kind == g::SegmentIntersectionKind::overlap &&
            overlap.first == g::Point{1, 0} && overlap.second == g::Point{3, 0});
    require(g::intersection(g::Segment{{0, 0}, {0, 0}}, g::Segment{{0, 0}, {0, 0}}).kind ==
            g::SegmentIntersectionKind::point);
    require(!g::intersects(g::Segment{{0, 0}, {1, 0}}, g::Segment{{2, 0}, {3, 0}}));
    require(
        near(g::distance(g::Segment{{0, 0}, {1, 0}}, g::Segment{{2, 1}, {2, 2}}), std::sqrt(2.)));
    throws<std::invalid_argument>([&] {
        (void)g::Line::through(a, a);
    });
    throws<std::invalid_argument>([] {
        (void)g::Line::from_equation(0, 0, 1);
    });
    throws<std::invalid_argument>([] {
        g::unit({0, 0});
    });
    throws<std::invalid_argument>([] {
        g::Tolerance(-1);
    });
    throws<std::invalid_argument>([] {
        g::orientation(g::Point{NAN, 0}, {}, {});
    });
    throws<std::invalid_argument>([] {
        g::orientation(g::IntegerPoint{1'000'000'001, 0}, {}, {});
    });
    require(g::squared_distance(g::IntegerPoint{-1'000'000'000, -1'000'000'000},
                                g::IntegerPoint{1'000'000'000, 1'000'000'000}) ==
            8'000'000'000'000'000'000LL);

    std::mt19937 random(735);
    for (int iteration = 0; iteration < 1000; ++iteration) {
        auto point = [&] {
            return g::IntegerPoint{static_cast<int>(random() % 41) - 20,
                                   static_cast<int>(random() % 41) - 20};
        };
        const g::IntegerSegment s{point(), point()}, t{point(), point()};
        auto floating = [](g::IntegerPoint x) {
            return g::Point{static_cast<double>(x.x), static_cast<double>(x.y)};
        };
        require(g::intersects(s, t) ==
                g::intersects(g::Segment{floating(s.first), floating(s.second)},
                              g::Segment{floating(t.first), floating(t.second)}));
    }
}

long double rectangle_oracle(std::span<const g::IntegerPoint> points) {
    if (points.size() < 3)
        return 0;
    auto best = std::numeric_limits<long double>::infinity();
    for (std::size_t i = 0; i < points.size(); ++i) {
        const auto a = points[i], b = points[(i + 1) % points.size()];
        const long double dx = b.x - a.x, dy = b.y - a.y;
        const auto length = std::hypot(dx, dy);
        auto minimum = std::numeric_limits<long double>::infinity(), maximum = -minimum;
        long double height = 0;
        for (const auto p : points) {
            const auto projection = ((p.x - a.x) * dx + (p.y - a.y) * dy) / length;
            minimum = std::min(minimum, projection);
            maximum = std::max(maximum, projection);
            height = std::max(height, ((p.y - a.y) * dx - (p.x - a.x) * dy) / length);
        }
        best = std::min(best, (maximum - minimum) * height);
    }
    return best;
}

void polygon_tests() {
    const std::vector<g::Point> square{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
    require(near(g::area<double>(square), 16) && near(g::perimeter<double>(square), 16));
    require(g::locate<double>(square, {2, 2}) == g::Containment::inside);
    require(g::locate<double>(square, {0, 2}) == g::Containment::boundary);
    require(g::locate<double>(square, {5, 0}) == g::Containment::outside);
    auto clockwise = square;
    std::reverse(clockwise.begin(), clockwise.end());
    require(g::is_convex<double>(clockwise) && g::signed_area<double>(clockwise) < 0);
    require(!g::is_convex<double>(std::vector<g::Point>{{0, 0}, {1, 0}, {2, 0}}));
    require(
        g::is_convex<double>(std::vector<g::Point>{{0, 0}, {2, 0}, {4, 0}, {4, 4}, {0, 4}}, true));
    const auto hull = g::ConvexPolygon::from_boundary(clockwise);
    require(hull.vertices()[0] == g::Point{0, 0} && hull.size() == 4);
    const auto rectangle = g::minimum_area_rectangle(hull);
    require(rectangle && near(rectangle->area, 16) &&
            near(g::signed_area<double>(rectangle->corners), 16));
    const auto clipped = g::clip_left(hull, g::Line::through({2, 0}, {2, 1}));
    require(near(g::area<double>(clipped), 8));
    require(g::clip_left(hull, g::Line::through({-1, 0}, {-1, 1})).empty());
    require(near(g::area<double>(g::clip_left(hull, g::Line::through({0, 1}, {0, 0}))), 16));
    auto translated = square;
    for (auto& p : translated) {
        p.x += 1e12;
        p.y += 1e12;
    }
    require(g::area<double>(translated) == 16); // Translation avoids huge shoelace cancellation.
    require(!g::lowest_leftmost<double>({}));
    require(!g::convex_diameter(g::ConvexPolygon{}));
    require(!g::minimum_area_rectangle(g::ConvexPolygon{}));
    const auto point = g::ConvexPolygon::from_points(std::vector<g::Point>{{3, 4}, {3, 4}});
    require(point.size() == 1 && g::convex_diameter(point)->squared_distance == 0);
    require(g::minimum_area_rectangle(point)->corners[0] == g::Point{3, 4});
    const auto collinear =
        g::ConvexPolygon::from_points(std::vector<g::Point>{{1, 0}, {0, 0}, {2, 0}, {1, 0}});
    require(collinear.size() == 2 && near(g::convex_diameter(collinear)->distance(), 2));
    require(g::minimum_area_rectangle(collinear)->area == 0);
    require(g::clip_left(collinear, g::Line::through({1, 1}, {1, 0})).size() == 2);
    throws<std::invalid_argument>([&] {
        (void)g::ConvexPolygon::from_boundary(
            std::vector<g::Point>{{0, 0}, {4, 4}, {4, 0}, {0, 4}});
    });
    throws<std::invalid_argument>([&] {
        (void)g::ConvexPolygon::from_boundary(
            std::vector<g::Point>{{0, 0}, {1, 0}, {4, 0}, {4, 4}, {0, 4}});
    });
    std::vector<g::Point> angles{{0, -1}, {-1, 0}, {0, 1}, {2, 0}, {1, 0}, {0, 0}};
    g::sort_by_angle(angles);
    require(angles[0] == g::Point{0, 0} && angles[1] == g::Point{1, 0} &&
            angles.back() == g::Point{0, -1});
    // Old epsilon comparisons could cycle among these three points.
    auto tiny = g::ConvexPolygon::from_points(
        std::vector<g::Point>{{0, 2}, {7.5e-9, 1}, {1.5e-8, 0}, {1, 1}});
    require(tiny.size() >= 3);
    std::vector<g::Point> signed_zero{{-0., -0.}, {1, 0}};
    g::sort_by_angle(signed_zero);
    require(signed_zero.front() == g::Point{});
    for (const double scale : {1e-30, 1e30}) {
        const std::vector<g::Point> scaled{
            {0, 0}, {4 * scale, 0}, {4 * scale, 3 * scale}, {0, 3 * scale}};
        const auto scaled_hull = g::ConvexPolygon::from_points(scaled);
        require(near(g::convex_diameter(scaled_hull)->squared_distance / (scale * scale), 25));
        require(near(g::minimum_area_rectangle(scaled_hull)->area / (scale * scale), 12));
        require(near(g::closest_pair<double>(scaled)->squared_distance / (scale * scale), 9));
    }
}

void randomized_point_sets() {
    std::mt19937 random(9623);
    g::ClosestPairWorkspace<std::int64_t> workspace;
    g::ClosestPairWorkspace<double> floating_workspace;
    for (int iteration = 0; iteration < 700; ++iteration) {
        std::vector<g::IntegerPoint> points;
        const auto count = random() % 55;
        for (std::size_t i = 0; i < count; ++i)
            points.push_back(
                {static_cast<int>(random() % 101) - 50, static_cast<int>(random() % 101) - 50});
        const auto original = points;
        const auto hull = g::convex_hull<std::int64_t>(points);
        const auto closest = workspace.solve(points), farthest = g::convex_diameter(hull);
        const auto recipe = g::point_set_diameter<std::int64_t>(points);
        std::int64_t minimum = std::numeric_limits<std::int64_t>::max(), maximum = 0;
        for (std::size_t i = 0; i < count; ++i)
            for (std::size_t j = i + 1; j < count; ++j) {
                // Independent small-coordinate arithmetic oracle.
                const auto dx = points[i].x - points[j].x, dy = points[i].y - points[j].y;
                const auto squared = dx * dx + dy * dy;
                minimum = std::min(minimum, squared);
                maximum = std::max(maximum, squared);
            }
        require(bool(closest) == (count >= 2));
        require(bool(farthest) == (count > 0) && bool(recipe) == (count > 0));
        if (closest)
            require(closest->squared_distance == minimum);
        if (farthest)
            require(farthest->squared_distance == maximum && recipe->squared_distance == maximum);
        require(points == original);
        for (const auto p : points)
            require(g::locate(hull, p) != g::Containment::outside);
        for (int q = 0; q < 20; ++q) {
            const g::IntegerPoint p{static_cast<int>(random() % 121) - 60,
                                    static_cast<int>(random() % 121) - 60};
            require(g::locate(hull, p) == g::locate<std::int64_t>(hull.vertices(), p));
        }
        std::vector<g::Point> floating;
        for (const auto p : points)
            floating.push_back({static_cast<double>(p.x), static_cast<double>(p.y)});
        const auto closest_float = floating_workspace.solve(floating);
        if (closest_float)
            require(near(closest_float->squared_distance, minimum));
        const auto float_hull = g::ConvexPolygon::from_points(floating);
        const auto float_diameter = g::convex_diameter(float_hull);
        if (float_diameter)
            require(near(float_diameter->squared_distance, maximum));
        const auto rectangle = g::minimum_area_rectangle(hull),
                   float_rectangle = g::minimum_area_rectangle(float_hull);
        if (rectangle) {
            require(near(rectangle->area, rectangle_oracle(hull.vertices())));
            require(near(rectangle->area, float_rectangle->area));
            const auto recipe_rectangle = g::minimum_bounding_rectangle<std::int64_t>(points);
            require(near(recipe_rectangle->area, rectangle->area));
            if (hull.size() >= 3) {
                const auto box_hull = g::ConvexPolygon::from_points(
                    std::vector<g::Point>(rectangle->corners.begin(), rectangle->corners.end()));
                for (const auto p : floating)
                    require(g::locate(box_hull.vertices(), p, g::Tolerance(1e-7)) !=
                            g::Containment::outside);
            }
        }
    }
    std::vector<g::IntegerPoint> extremes{{-1'000'000'000, -1'000'000'000},
                                          {1'000'000'000, 1'000'000'000}};
    require(workspace.solve(extremes)->squared_distance == 8'000'000'000'000'000'000LL);
    std::vector<g::IntegerPoint> repeated(1000, {1, 2});
    require(workspace.solve(repeated)->squared_distance == 0);
    const auto repeated_hull = g::IntegerConvexPolygon::from_points(repeated);
    require(repeated_hull.size() == 1 && repeated_hull.storage_bytes() <= 32);
    std::vector<g::IntegerPoint> vertical;
    for (int i = 0; i < 257; ++i)
        vertical.push_back({0, i * 3});
    require(workspace.solve(vertical)->squared_distance == 9);
    const auto vertical_hull = g::IntegerConvexPolygon::from_points(vertical);
    require(vertical_hull.size() == 2 && vertical_hull.storage_bytes() <= 64);
    const std::vector<g::IntegerPoint> boundary{{-1'000'000'000, -1'000'000'000},
                                                {1'000'000'000, -1'000'000'000},
                                                {1'000'000'000, 1'000'000'000},
                                                {-1'000'000'000, 1'000'000'000}};
    const auto wide_hull = g::IntegerConvexPolygon::from_points(boundary);
    require(g::convex_diameter(wide_hull)->squared_distance == 8'000'000'000'000'000'000LL);
    require(g::locate(wide_hull, g::IntegerPoint{}) == g::Containment::inside);
    require(near(g::minimum_area_rectangle(wide_hull)->area, 4e18));
    workspace.release();
    require(workspace.storage_bytes() == 0);
    require(workspace.solve(vertical)->squared_distance == 9);
}

void circle_tests() {
    const g::Circle circle({0, 0}, 1);
    require(g::intersection(circle, g::Line::through({-2, 1}, {2, 1})).count == 1);
    require(g::intersection(circle, g::Line::through({-2, 2}, {2, 2})).count == 0);
    require(g::intersection(circle, g::Line::through({-2, 0}, {2, 0})).count == 2);
    require(g::intersection(circle, g::Segment{{0, 0}, {1, 0}}).count == 1);
    require(g::intersection(circle, g::Segment{{-1, 0}, {1, 0}}).count == 2);
    require(g::intersection(circle, g::Segment{{-2, 1}, {2, 1}}).count == 1);
    require(g::intersection(circle, g::Segment{{1, 0}, {1, 0}}).count == 1);
    require(g::intersection(circle, g::Segment{{0, 0}, {0, 0}}).count == 0);
    require(g::intersection(circle, circle).kind == g::IntersectionKind::infinite);
    require(g::intersection(circle, g::Circle({}, 2)).kind == g::IntersectionKind::none);
    require(g::intersection(g::Circle({}, 0), g::Circle({}, 0)).count == 1);
    require(g::centers_through({0, 0}, {0, 0}, 1).kind == g::IntersectionKind::infinite);
    require(g::centers_through({-1, 0}, {1, 0}, 1).count == 1);
    require(!g::circumcircle({0, 0}, {1, 0}, {2, 0}));
    require(!g::incircle({0, 0}, {1, 0}, {2, 0}));
    const auto outer = g::circumcircle({0, 0}, {4, 0}, {0, 3});
    require(outer && g::almost_equal(outer->center(), {2, 1.5}) && near(outer->radius(), 2.5));
    const auto inner = g::incircle({0, 0}, {4, 0}, {0, 3});
    require(inner && g::almost_equal(inner->center(), {1, 1}) && near(inner->radius(), 1));
    require(g::tangent_points(circle, {0, 0}).count == 0);
    require(g::tangent_points(circle, {1, 0}).count == 1);
    const auto contact = g::tangent_points(circle, {2, 0});
    require(contact.count == 2);
    for (std::size_t i = 0; i < contact.count; ++i) {
        require(near(g::distance(contact.values[i], circle.center()), 1));
        require(near(
            g::dot(contact.values[i] - circle.center(), contact.values[i] - g::Point{2, 0}), 0));
    }
    require(g::common_tangent_count(circle, circle) == std::nullopt);
    require(g::common_tangent_count(circle, g::Circle({4, 0}, 1)) == 4);
    require(g::common_tangent_count(circle, g::Circle({2, 0}, 1)) == 3);
    require(g::common_tangent_count(circle, g::Circle({1, 0}, 1)) == 2);
    require(g::common_tangent_count(g::Circle({}, 2), g::Circle({1, 0}, 1)) == 1);
    require(g::common_tangent_count(g::Circle({}, 3), circle) == 0);
    require(g::common_tangent_count(circle, g::Circle({2, 0}, 0)) == 2);
    require(g::common_tangent_count(circle, g::Circle({1, 0}, 0)) == 1);
    require(g::common_tangent_count(g::Circle({}, 0), g::Circle({2, 0}, 0)) == 1);
    require(near(g::overlap_area(circle, circle), std::numbers::pi));
    require(near(g::overlap_area(circle, g::Circle({1, 0}, 1)),
                 2 * std::numbers::pi / 3 - std::sqrt(3.) / 2));
    require(g::overlap_area(circle, g::Circle({2, 0}, 1)) == 0);
    require(g::overlap_area(circle, g::Circle({2 - 1e-8, 0}, 1)) > 0);
    const std::vector<g::Point> square{{-4, -4}, {4, -4}, {4, 4}, {-4, 4}};
    require(g::disk_polygon_intersection(circle, square) == g::RegionIntersection::overlapping);
    require(g::disk_polygon_intersection(g::Circle({0, 0}, 10), square) ==
            g::RegionIntersection::overlapping);
    require(g::disk_polygon_intersection(g::Circle({5, 0}, 1), square) ==
            g::RegionIntersection::touching);
    require(g::disk_polygon_intersection(g::Circle({6, 0}, 1), square) ==
            g::RegionIntersection::disjoint);
    require(g::disk_polygon_intersection(g::Circle({0, 0}, 0), square) ==
            g::RegionIntersection::touching);
    require(g::disk_polygon_intersection(circle, {}) == g::RegionIntersection::disjoint);
    throws<std::invalid_argument>([] {
        g::Circle({}, -1);
    });
    throws<std::invalid_argument>([] {
        g::Circle({}, std::numeric_limits<double>::infinity());
    });

    std::mt19937 random(2563);
    for (int i = 0; i < 400; ++i) {
        auto make_circle = [&] {
            return g::Circle(
                {static_cast<int>(random() % 41) - 20., static_cast<int>(random() % 41) - 20.},
                (random() % 15) * .5);
        };
        const auto a = make_circle(), b = make_circle();
        const auto intersections = g::intersection(a, b);
        for (std::size_t k = 0; k < intersections.count; ++k) {
            require(near(g::distance(intersections.values[k], a.center()), a.radius()));
            require(near(g::distance(intersections.values[k], b.center()), b.radius()));
        }
        const auto tangents = g::common_tangents(a, b);
        for (std::size_t k = 0; k < tangents.count; ++k) {
            require(near(g::distance(tangents.values[k], a.center()), a.radius()));
            require(near(g::distance(tangents.values[k], b.center()), b.radius()));
        }
        require(near(g::overlap_area(a, b), g::overlap_area(b, a)));
        require(g::overlap_area(a, b) >= 0 &&
                g::overlap_area(a, b) <= std::numbers::pi * std::min(a.radius(), b.radius()) *
                                                 std::min(a.radius(), b.radius()) +
                                             1e-8);
    }
}
} // namespace

bool geometry_odr_probe();
int main() {
    try {
        linear_tests();
        polygon_tests();
        randomized_point_sets();
        circle_tests();
        require(geometry_odr_probe());
        std::cout
            << "Geometry regressions, exact integer predicates, randomized oracles and ODR passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
