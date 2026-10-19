#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace algorithm_assets::geometry3d {

struct Vector3 {
    double x{}, y{}, z{};
    friend bool operator==(Vector3, Vector3) = default;
};
struct Point3 {
    double x{}, y{}, z{};
    friend bool operator==(Point3, Point3) = default;
};
inline Vector3 operator+(Vector3 a, Vector3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
inline Vector3 operator-(Vector3 a, Vector3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline Vector3 operator*(Vector3 a, double s) {
    return {a.x * s, a.y * s, a.z * s};
}
inline Vector3 operator/(Vector3 a, double s) {
    return {a.x / s, a.y / s, a.z / s};
}
inline Vector3 operator-(Point3 a, Point3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline Point3 operator+(Point3 p, Vector3 v) {
    return {p.x + v.x, p.y + v.y, p.z + v.z};
}
inline double dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline Vector3 cross(Vector3 a, Vector3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline double length(Vector3 a) {
    return std::hypot(a.x, a.y, a.z);
}
inline double squared_distance(Point3 a, Point3 b) {
    return dot(a - b, a - b);
}
inline double coordinate(Point3 p, unsigned axis) {
    return axis == 0 ? p.x : axis == 1 ? p.y : p.z;
}
inline void validate(Point3 p) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
        std::max({std::abs(p.x), std::abs(p.y), std::abs(p.z)}) > 1e50)
        throw std::invalid_argument("3D coordinates must be finite and within +/-1e50");
}
inline Vector3 normalized(Vector3 v) {
    const double n = length(v);
    if (!(n > 0) || !std::isfinite(n))
        throw std::invalid_argument("Invalid direction");
    return v / n;
}
inline Point3 interpolate(Point3 a, Point3 b, double t) {
    return a + (b - a) * t;
}
// Low-level arithmetic is approximate; this is not an exact orientation predicate.
inline double oriented_volume6(Point3 a, Point3 b, Point3 c, Point3 d) {
    return dot(b - a, cross(c - a, d - a));
}
struct Aabb3 {
    Point3 minimum{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
                   std::numeric_limits<double>::infinity()};
    Point3 maximum{-std::numeric_limits<double>::infinity(),
                   -std::numeric_limits<double>::infinity(),
                   -std::numeric_limits<double>::infinity()};
    void include(Point3 p) {
        minimum = {std::min(minimum.x, p.x), std::min(minimum.y, p.y), std::min(minimum.z, p.z)};
        maximum = {std::max(maximum.x, p.x), std::max(maximum.y, p.y), std::max(maximum.z, p.z)};
    }
    void include(const Aabb3& b) {
        include(b.minimum);
        include(b.maximum);
    }
    [[nodiscard]] bool empty() const {
        return minimum.x > maximum.x;
    }
    [[nodiscard]] double distance_squared(Point3 p) const {
        if (empty())
            return std::numeric_limits<double>::infinity();
        const Vector3 d{std::max({minimum.x - p.x, 0.0, p.x - maximum.x}),
                        std::max({minimum.y - p.y, 0.0, p.y - maximum.y}),
                        std::max({minimum.z - p.z, 0.0, p.z - maximum.z})};
        return dot(d, d);
    }
};
struct TriangleClosestPoint {
    Point3 point;
    std::array<double, 3> barycentric{};
    double distance_squared{};
};
inline TriangleClosestPoint closest_point_on_triangle(Point3 p, Point3 a, Point3 b, Point3 c) {
    validate(p);
    validate(a);
    validate(b);
    validate(c);
    const Vector3 ab = b - a, ac = c - a, ap = p - a;
    auto result = [&](double u, double v, double w) {
        const Point3 q = a + ab * v + ac * w;
        if (!std::isfinite(u) || !std::isfinite(v) || !std::isfinite(w) || !std::isfinite(q.x) ||
            !std::isfinite(q.y) || !std::isfinite(q.z))
            throw std::domain_error("Nonfinite triangle projection");
        return TriangleClosestPoint{q, {u, v, w}, squared_distance(p, q)};
    };
    // Degenerate triangles reduce to three segments, including point segments.
    if (length(cross(ab, ac)) == 0) {
        auto best = result(1, 0, 0);
        const std::array<Point3, 3> points{a, b, c};
        for (unsigned i = 0; i < 3; ++i) {
            const unsigned j = (i + 1) % 3;
            const Vector3 d = points[j] - points[i];
            const double denominator = dot(d, d);
            const double t =
                denominator == 0 ? 0 : std::clamp(dot(p - points[i], d) / denominator, 0.0, 1.0);
            std::array<double, 3> weights{};
            weights[i] = 1 - t;
            weights[j] = t;
            auto candidate = result(weights[0], weights[1], weights[2]);
            if (candidate.distance_squared < best.distance_squared)
                best = candidate;
        }
        return best;
    }
    const double d1 = dot(ab, ap), d2 = dot(ac, ap);
    if (d1 <= 0 && d2 <= 0)
        return result(1, 0, 0);
    const Vector3 bp = p - b;
    const double d3 = dot(ab, bp), d4 = dot(ac, bp);
    if (d3 >= 0 && d4 <= d3)
        return result(0, 1, 0);
    const double vc = d1 * d4 - d3 * d2;
    if (vc <= 0 && d1 >= 0 && d3 <= 0) {
        const double v = d1 / (d1 - d3);
        return result(1 - v, v, 0);
    }
    const Vector3 cp = p - c;
    const double d5 = dot(ab, cp), d6 = dot(ac, cp);
    if (d6 >= 0 && d5 <= d6)
        return result(0, 0, 1);
    const double vb = d5 * d2 - d1 * d6;
    if (vb <= 0 && d2 >= 0 && d6 <= 0) {
        const double w = d2 / (d2 - d6);
        return result(1 - w, 0, w);
    }
    const double va = d3 * d6 - d5 * d4;
    if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0) {
        const double w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return result(0, 1 - w, w);
    }
    const double denominator = va + vb + vc;
    if (denominator == 0 || !std::isfinite(denominator))
        throw std::domain_error("Unresolved triangle conditioning");
    return result(va / denominator, vb / denominator, vc / denominator);
}

} // namespace algorithm_assets::geometry3d
