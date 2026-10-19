#include <algorithm_assets/graph/two_sat.hpp>
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n, q;
    std::cin >> n >> q;
    algorithm_assets::TwoSat solver(n);
    while (q--) {
        std::uint32_t i, j;
        int x, y;
        std::cin >> i >> x >> j >> y;
        solver.add_clause({i - 1, bool(x)}, {j - 1, bool(y)});
    }
    auto solution = solver.solve();
    if (!solution.satisfiable) {
        std::cout << "IMPOSSIBLE\n";
        return 0;
    }
    std::cout << "POSSIBLE\n";
    for (auto x : solution.assignment)
        std::cout << unsigned(x) << ' ';
    std::cout << '\n';
}
