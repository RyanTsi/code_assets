#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace algorithm_assets::geometry {

// Integer predicates are exact for |coordinate| <= 1e9. Floating predicates
// use long double intermediates, not an adaptive exact arithmetic kernel.
template <class T>
concept Coordinate = std::same_as<T, double> || std::same_as<T, std::int64_t>;

template <Coordinate T> struct Vector2 {
    T x{}, y{};
    constexpr bool operator==(const Vector2&) const = default;
    constexpr Vector2 operator-() const {
        return {-x, -y};
    }
    constexpr Vector2 operator+(Vector2 b) const {
        return {x + b.x, y + b.y};
    }
    constexpr Vector2 operator-(Vector2 b) const {
        return {x - b.x, y - b.y};
    }
    constexpr Vector2 operator*(T scale) const {
        return {x * scale, y * scale};
    }
};

template <Coordinate T> struct Point2 {
    T x{}, y{};
    constexpr bool operator==(const Point2&) const = default;
    constexpr Vector2<T> operator-(Point2 b) const {
        return {x - b.x, y - b.y};
    }
    constexpr Point2 operator+(Vector2<T> v) const {
        return {x + v.x, y + v.y};
    }
    constexpr Point2 operator-(Vector2<T> v) const {
        return {x - v.x, y - v.y};
    }
};

using Point = Point2<double>;
using Vector = Vector2<double>;
using IntegerPoint = Point2<std::int64_t>;
static_assert(sizeof(Point) == 16 && sizeof(IntegerPoint) == 16 &&
                  std::numeric_limits<double>::is_iec559 &&
                  std::numeric_limits<double>::digits == 53,
              "Geometry requires binary64 double and 16-byte point storage");
template <Coordinate T>
using SquaredDistance = std::conditional_t<std::same_as<T, double>, long double, std::int64_t>;

enum class Turn { clockwise = -1, collinear = 0, counterclockwise = 1 };
enum class RelativePosition { left, right, before, after, on_segment, degenerate };
enum class Containment { outside, boundary, inside };
enum class IntersectionKind { none, finite, infinite };

// A fixed-capacity result avoids heap allocations for intersections/tangents.
// Only elements [0,count) are meaningful; infinite sets have count == 0.
template <class T, std::size_t Capacity> struct IntersectionSet {
    IntersectionKind kind = IntersectionKind::none;
    std::array<T, Capacity> values{};
    std::size_t count = 0;
    void add(const T& value) {
        if (count == Capacity)
            throw std::length_error("IntersectionSet capacity exceeded");
        values[count++] = value;
        kind = IntersectionKind::finite;
    }
};

class Tolerance {
public:
    explicit Tolerance(double absolute = 1e-12,
                       double relative = 32 * std::numeric_limits<double>::epsilon())
        : absolute_(absolute), relative_(relative) {
        if (!std::isfinite(absolute) || !std::isfinite(relative) || absolute < 0 || relative < 0)
            throw std::invalid_argument("Tolerance must be finite and nonnegative");
    }
    [[nodiscard]] int sign(long double value, long double scale = 1) const {
        const auto error = absolute_ + relative_ * std::abs(scale);
        return value > error ? 1 : value < -error ? -1 : 0;
    }
    [[nodiscard]] int compare(long double a, long double b) const {
        return sign(a - b, std::max(std::abs(a), std::abs(b)));
    }

private:
    double absolute_, relative_;
};

namespace detail {
template <Coordinate T> inline void validate(Point2<T> p) {
    if constexpr (std::same_as<T, double>) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || std::abs(p.x) > 1e100 ||
            std::abs(p.y) > 1e100)
            throw std::invalid_argument("Floating coordinates must be finite and within +/-1e100");
    } else {
        constexpr std::int64_t bound = 1'000'000'000;
        if (p.x < -bound || p.x > bound || p.y < -bound || p.y > bound)
            throw std::invalid_argument("Exact integer coordinates must be within +/-1e9");
    }
}
template <Coordinate T> inline void validate(std::span<const Point2<T>> points) {
    for (const auto p : points)
        validate(p);
}
template <Coordinate T> constexpr bool lex_less(Point2<T> a, Point2<T> b) {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
}
template <Coordinate T> constexpr SquaredDistance<T> squared_unchecked(Point2<T> a, Point2<T> b) {
    // Cast BEFORE subtraction in the floating kernel, preserving guard digits.
    const SquaredDistance<T> dx = static_cast<SquaredDistance<T>>(a.x) - b.x;
    const SquaredDistance<T> dy = static_cast<SquaredDistance<T>>(a.y) - b.y;
    const auto squared = dx * dx + dy * dy;
    if constexpr (std::same_as<T, double>) {
        if (squared == 0 && a != b)
            throw std::underflow_error("Distinct point distance squared underflowed");
    }
    return squared;
}
template <Coordinate T>
constexpr SquaredDistance<T> determinant(Point2<T> a, Point2<T> b, Point2<T> c) {
    const SquaredDistance<T> bx = static_cast<SquaredDistance<T>>(b.x) - a.x;
    const SquaredDistance<T> by = static_cast<SquaredDistance<T>>(b.y) - a.y;
    const SquaredDistance<T> cx = static_cast<SquaredDistance<T>>(c.x) - a.x;
    const SquaredDistance<T> cy = static_cast<SquaredDistance<T>>(c.y) - a.y;
    return bx * cy - by * cx;
}
template <Coordinate T> constexpr int turn_unchecked(Point2<T> a, Point2<T> b, Point2<T> c) {
    const auto value = determinant(a, b, c);
    return (value > 0) - (value < 0);
}
template <Coordinate T> constexpr bool on_segment_unchecked(Point2<T> a, Point2<T> b, Point2<T> p) {
    return turn_unchecked(a, b, p) == 0 && std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x) &&
           std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}
inline Point make_point(long double x, long double y) {
    if (!std::isfinite(x) || !std::isfinite(y) || std::abs(x) > 1e100L || std::abs(y) > 1e100L)
        throw std::overflow_error("Constructed point is outside the floating coordinate domain");
    return {static_cast<double>(x), static_cast<double>(y)};
}
inline double checked_real(long double value) {
    const auto result = static_cast<double>(value);
    if (!std::isfinite(result))
        throw std::overflow_error("Geometry result overflowed");
    return result;
}
} // namespace detail

template <Coordinate T> constexpr SquaredDistance<T> dot(Vector2<T> a, Vector2<T> b) {
    return static_cast<SquaredDistance<T>>(a.x) * b.x + static_cast<SquaredDistance<T>>(a.y) * b.y;
}
template <Coordinate T> constexpr SquaredDistance<T> cross(Vector2<T> a, Vector2<T> b) {
    return static_cast<SquaredDistance<T>>(a.x) * b.y - static_cast<SquaredDistance<T>>(a.y) * b.x;
}
template <Coordinate T>
inline RelativePosition relative_position(Point2<T> a, Point2<T> b, Point2<T> p) {
    detail::validate(a);
    detail::validate(b);
    detail::validate(p);
    if (a == b)
        return a == p ? RelativePosition::on_segment : RelativePosition::degenerate;
    const auto turn = detail::turn_unchecked(a, b, p);
    if (turn > 0)
        return RelativePosition::left;
    if (turn < 0)
        return RelativePosition::right;
    if (dot(b - a, p - a) < 0)
        return RelativePosition::before;
    if (detail::squared_unchecked(a, p) > detail::squared_unchecked(a, b))
        return RelativePosition::after;
    return RelativePosition::on_segment;
}
template <Coordinate T> constexpr Vector2<T> perpendicular_ccw(Vector2<T> v) {
    return {-v.y, v.x};
}

template <Coordinate T> inline Turn orientation(Point2<T> a, Point2<T> b, Point2<T> c) {
    detail::validate(a);
    detail::validate(b);
    detail::validate(c);
    return static_cast<Turn>(detail::turn_unchecked(a, b, c));
}
inline Turn orientation(Point a, Point b, Point c, Tolerance tolerance) {
    detail::validate(a);
    detail::validate(b);
    detail::validate(c);
    const long double bx = static_cast<long double>(b.x) - a.x,
                      by = static_cast<long double>(b.y) - a.y;
    const long double cx = static_cast<long double>(c.x) - a.x,
                      cy = static_cast<long double>(c.y) - a.y;
    return static_cast<Turn>(
        tolerance.sign(bx * cy - by * cx, std::abs(bx * cy) + std::abs(by * cx)));
}
template <Coordinate T> inline SquaredDistance<T> squared_distance(Point2<T> a, Point2<T> b) {
    detail::validate(a);
    detail::validate(b);
    return detail::squared_unchecked(a, b);
}
template <Coordinate T> inline double distance(Point2<T> a, Point2<T> b) {
    detail::validate(a);
    detail::validate(b);
    return detail::checked_real(
        std::hypot(static_cast<long double>(a.x) - b.x, static_cast<long double>(a.y) - b.y));
}
inline bool almost_equal(Point a, Point b, Tolerance tolerance = Tolerance{}) {
    detail::validate(a);
    detail::validate(b);
    return tolerance.compare(a.x, b.x) == 0 && tolerance.compare(a.y, b.y) == 0;
}
inline Vector unit(Vector v) {
    detail::validate(Point{v.x, v.y});
    const auto length = std::hypot(v.x, v.y);
    if (length == 0)
        throw std::invalid_argument("A zero vector has no unit direction");
    return {v.x / length, v.y / length};
}
inline Vector rotate(Vector v, double radians) {
    detail::validate(Point{v.x, v.y});
    if (!std::isfinite(radians))
        throw std::invalid_argument("Angle must be finite");
    const auto sine = std::sin(radians), cosine = std::cos(radians);
    const auto p = detail::make_point(
        static_cast<long double>(v.x) * cosine - static_cast<long double>(v.y) * sine,
        static_cast<long double>(v.x) * sine + static_cast<long double>(v.y) * cosine);
    return {p.x, p.y};
}
inline Point polar(double radius, double radians) {
    if (!std::isfinite(radius) || radius < 0 || radius > 1e100 || !std::isfinite(radians))
        throw std::invalid_argument("Invalid polar radius/angle");
    return {radius * std::cos(radians), radius * std::sin(radians)};
}

template <Coordinate T> struct PointPair {
    Point2<T> first{}, second{};
    SquaredDistance<T> squared_distance{};
    [[nodiscard]] double distance() const {
        return detail::checked_real(std::sqrt(static_cast<long double>(squared_distance)));
    }
};

} // namespace algorithm_assets::geometry
