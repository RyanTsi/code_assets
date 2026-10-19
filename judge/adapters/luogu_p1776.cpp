#include <algorithm_assets/optimization/knapsack.hpp>
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n, capacity;
    std::cin >> n >> capacity;
    std::vector<algorithm_assets::BoundedKnapsackItem> items(n);
    for (auto& item : items)
        std::cin >> item.value >> item.cost >> item.count;
    std::cout << algorithm_assets::knapsack_bounded(items, capacity) << '\n';
}
