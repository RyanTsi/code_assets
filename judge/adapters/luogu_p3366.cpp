#include <algorithm_assets/graph/kruskal.hpp>
#include <iostream>
#include <vector>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n, m;
    std::cin >> n >> m;
    std::vector<algorithm_assets::UndirectedWeightedEdge> edges(m);
    for (auto& edge : edges) {
        std::cin >> edge.first >> edge.second >> edge.weight;
        --edge.first; --edge.second;
    }
    auto forest = algorithm_assets::kruskal_minimum_spanning_forest(n, edges);
    if (forest.is_spanning_tree) std::cout << forest.total_weight << '\n';
    else std::cout << "orz\n";
}
