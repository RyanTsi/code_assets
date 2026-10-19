#include <algorithm_assets/recipes/tree_eccentricities.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int tests;
    std::cin >> tests;
    while (tests--) {
        std::size_t n;
        std::int64_t edge_length, move_cost;
        std::cin >> n >> edge_length >> move_cost;
        std::vector<algorithm_assets::UnweightedCsrGraph::Edge> edges(n - 1);
        for (auto& edge : edges) {
            std::cin >> edge.from >> edge.to;
            --edge.from;
            --edge.to;
        }
        const auto metrics = algorithm_assets::tree_eccentricities(n, edges);
        std::int64_t profit = 0;
        for (std::size_t i = 0; i < n; ++i)
            profit = std::max(profit, edge_length * metrics.eccentricities[i] -
                                          move_cost * metrics.source_distances[i]);
        std::cout << profit << '\n';
    }
}
