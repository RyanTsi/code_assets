#pragma once
#include <algorithm_assets/geometry/circle.hpp>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

namespace algorithm_assets::geometry {
namespace detail {
struct EnclosingPoint {
    long double x, y;
};
struct EnclosingDisk {
    EnclosingPoint center;
    long double radius;
};
inline long double enclosing_distance(EnclosingPoint a, EnclosingPoint b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}
inline bool enclosing_contains(EnclosingDisk d, EnclosingPoint p) {
    return enclosing_distance(d.center, p) <=
           d.radius * (1 + 64 * std::numeric_limits<long double>::epsilon());
}
inline EnclosingDisk enclosing_pair(EnclosingPoint a, EnclosingPoint b) {
    EnclosingPoint center{(a.x + b.x) / 2, (a.y + b.y) / 2};
    return {center, std::max(enclosing_distance(center, a), enclosing_distance(center, b))};
}
inline EnclosingDisk enclosing_triple(EnclosingPoint a, EnclosingPoint b, EnclosingPoint c) {
    const auto x = b.x - a.x, y = b.y - a.y, u = c.x - a.x, v = c.y - a.y;
    const auto determinant = 2 * (x * v - y * u);
    if (determinant == 0) {
        auto ab = enclosing_pair(a, b), ac = enclosing_pair(a, c), bc = enclosing_pair(b, c);
        auto result = ab.radius >= ac.radius ? ab : ac;
        return result.radius >= bc.radius ? result : bc;
    }
    const auto norm_b = x * x + y * y, norm_c = u * u + v * v;
    EnclosingPoint center{a.x + (norm_b * v - norm_c * y) / determinant,
                          a.y + (x * norm_c - u * norm_b) / determinant};
    const auto radius = std::max({enclosing_distance(center, a), enclosing_distance(center, b),
                                  enclosing_distance(center, c)});
    if (!std::isfinite(radius))
        throw std::overflow_error("Ill-conditioned circumcircle computation");
    return {center, radius};
}
} // namespace detail
// Randomized incremental construction, worst O(n^3); exact-arithmetic expected
// O(n) assumes a uniform random permutation. Fixed seed is reproducible, not a
// worst-case time guarantee. Floating constructions remain approximate.
inline std::optional<Circle> minimum_enclosing_circle(std::span<const Point> points,
                                                      std::uint64_t seed = 0x61adbeefU) {
    long double scale = 0;
    for (auto p : points) {
        detail::validate(p);
        if (std::abs(p.x) > 1e50 || std::abs(p.y) > 1e50)
            throw std::invalid_argument("Enclosing-circle coordinates limited to +/-1e50");
        scale = std::max({scale, std::abs(static_cast<long double>(p.x)),
                          std::abs(static_cast<long double>(p.y))});
    }
    if (points.empty())
        return std::nullopt;
    if (scale == 0)
        return Circle(points.front(), 0);
    std::vector<detail::EnclosingPoint> shuffled;
    shuffled.reserve(points.size());
    for (auto p : points)
        shuffled.push_back({p.x / scale, p.y / scale});
    std::mt19937_64 random(seed);
    std::shuffle(shuffled.begin(), shuffled.end(), random);
    detail::EnclosingDisk disk{shuffled[0], 0};
    for (std::size_t i = 0; i < shuffled.size(); ++i)
        if (!detail::enclosing_contains(disk, shuffled[i])) {
            disk = {shuffled[i], 0};
            for (std::size_t j = 0; j < i; ++j)
                if (!detail::enclosing_contains(disk, shuffled[j])) {
                    disk = detail::enclosing_pair(shuffled[i], shuffled[j]);
                    for (std::size_t k = 0; k < j; ++k)
                        if (!detail::enclosing_contains(disk, shuffled[k]))
                            disk = detail::enclosing_triple(shuffled[i], shuffled[j], shuffled[k]);
                }
        }
    const auto center = detail::make_point(disk.center.x * scale, disk.center.y * scale);
    long double radius = 0;
    for (auto p : points)
        radius = std::max(radius, std::hypot(static_cast<long double>(p.x) - center.x,
                                             static_cast<long double>(p.y) - center.y));
    // Compensate center rounding; never round the final enclosing radius inward.
    auto rounded = static_cast<double>(radius);
    if (static_cast<long double>(rounded) < radius)
        rounded = std::nextafter(rounded, std::numeric_limits<double>::infinity());
    return Circle(center, rounded);
}
} // namespace algorithm_assets::geometry
