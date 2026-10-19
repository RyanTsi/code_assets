#pragma once

#include <algorithm_assets/geometry/rotating_calipers.hpp>

namespace algorithm_assets::geometry {

// Verified compositions: unordered points -> strict hull -> linear calipers.
template <Coordinate T>
inline std::optional<PointPair<T>> point_set_diameter(std::span<const Point2<T>> points) {
    return convex_diameter(convex_hull<T>(points));
}
template <Coordinate T>
inline std::optional<OrientedRectangle>
minimum_bounding_rectangle(std::span<const Point2<T>> points) {
    return minimum_area_rectangle(convex_hull<T>(points));
}

} // namespace algorithm_assets::geometry
