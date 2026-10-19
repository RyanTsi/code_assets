#include <algorithm_assets/geometry/minimum_enclosing_circle.hpp>
#include <iomanip>
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n;
    std::cin >> n;
    std::vector<algorithm_assets::geometry::Point> points(n);
    for (auto& p : points)
        std::cin >> p.x >> p.y;
    const auto circle = algorithm_assets::geometry::minimum_enclosing_circle(points);
    std::cout << std::fixed << std::setprecision(12) << circle->radius() << '\n'
              << circle->center().x << ' ' << circle->center().y << '\n';
}
