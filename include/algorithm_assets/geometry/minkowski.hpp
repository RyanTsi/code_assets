#pragma once
#include <algorithm_assets/geometry/convex_hull.hpp>
#include <limits>

namespace algorithm_assets::geometry {
namespace detail {
inline IntegerConvexPolygon minkowski_merge(const IntegerConvexPolygon& first,
                                            const IntegerConvexPolygon& second, bool subtract) {
    const auto a = first.vertices(), b = second.vertices();
    for (auto points : {a, b})
        for (auto p : points)
            if (std::abs(p.x) > 500'000'000 || std::abs(p.y) > 500'000'000)
                throw std::invalid_argument("Minkowski input coordinates must be within +/-5e8");
    if (a.empty() || b.empty())
        return {};
    auto point_b = [&](std::size_t i) {
        auto p = b[i % b.size()];
        return subtract ? IntegerPoint{-p.x, -p.y} : p;
    };
    auto lowest = [](auto get, std::size_t n) {
        std::size_t best = 0;
        for (std::size_t i = 1; i < n; ++i) {
            auto p = get(i), q = get(best);
            if (p.y < q.y || (p.y == q.y && p.x < q.x))
                best = i;
        }
        return best;
    };
    const auto begin_a = lowest(
                   [&](auto i) {
                       return a[i];
                   },
                   a.size()),
               begin_b = lowest(point_b, b.size());
    auto get_a = [&](std::size_t i) {
        return a[(begin_a + i) % a.size()];
    };
    auto get_b = [&](std::size_t i) {
        return point_b(begin_b + i);
    };
    std::vector<IntegerPoint> output;
    if (a.size() > output.max_size() - b.size())
        throw std::length_error("Minkowski output too large");
    output.reserve(a.size() + b.size());
    auto p = get_a(0), q = get_b(0);
    IntegerPoint current{p.x + q.x, p.y + q.y};
    output.push_back(current);
    const auto count_a = a.size() > 1 ? a.size() : 0, count_b = b.size() > 1 ? b.size() : 0;
    std::size_t i = 0, j = 0;
    auto upper = [](Vector2<std::int64_t> v) {
        return v.y > 0 || (v.y == 0 && v.x >= 0);
    };
    while (i < count_a || j < count_b) {
        Vector2<std::int64_t> step{};
        if (i == count_a) {
            step = get_b(j + 1) - get_b(j);
            ++j;
        } else if (j == count_b) {
            step = get_a(i + 1) - get_a(i);
            ++i;
        } else {
            const auto x = get_a(i + 1) - get_a(i), y = get_b(j + 1) - get_b(j);
            const auto cross = x.x * y.y - x.y * y.x;
            if (upper(x) == upper(y) && cross == 0) {
                step = x + y;
                ++i;
                ++j;
            } else if (upper(x) != upper(y) ? upper(x) : cross > 0) {
                step = x;
                ++i;
            } else {
                step = y;
                ++j;
            }
        }
        current = current + step;
        if (i < count_a || j < count_b)
            output.push_back(current);
    }
    std::rotate(output.begin(),
                std::min_element(output.begin(), output.end(), lex_less<std::int64_t>),
                output.end());
    return ConvexPolygonAccess::make<std::int64_t>(std::move(output));
}
} // namespace detail
inline IntegerConvexPolygon minkowski_sum(const IntegerConvexPolygon& a,
                                          const IntegerConvexPolygon& b) {
    return detail::minkowski_merge(a, b, false);
}
inline IntegerConvexPolygon minkowski_difference(const IntegerConvexPolygon& a,
                                                 const IntegerConvexPolygon& b) {
    return detail::minkowski_merge(a, b, true);
}
class TranslatedConvexOverlap {
public:
    TranslatedConvexOverlap(const IntegerConvexPolygon& a, const IntegerConvexPolygon& b)
        : difference_(minkowski_difference(a, b)) {
    }
    [[nodiscard]] bool intersects(IntegerPoint translation) const {
        return locate(difference_, translation) != Containment::outside;
    }
    [[nodiscard]] const IntegerConvexPolygon& difference() const noexcept {
        return difference_;
    }

private:
    IntegerConvexPolygon difference_;
};
} // namespace algorithm_assets::geometry
