#include <algorithm_assets/geometry/geometry2d.hpp>

bool geometry_odr_probe() {
  using namespace algorithm_assets::geometry;
  const auto circle = circumcircle({0, 0}, {2, 0}, {0, 2});
  return circle && almost_equal(circle->center(), {1, 1});
}
