#include <algorithm_assets/recipes/bounded_step_subsequence.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n;
    std::int64_t difference;
    if (!(std::cin >> n >> difference))
        return 0;
    std::vector<std::int64_t> values(n);
    for (auto& value : values)
        std::cin >> value;
    std::cout << algorithm_assets::bounded_step_subsequence_length(values, difference) << '\n';
}
