#include <algorithm_assets/range/fenwick_sum.hpp>
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n, q;
    std::cin >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto& value : values) std::cin >> value;
    algorithm_assets::FenwickSum sums(values);
    while (q--) {
        int operation;
        std::size_t first;
        std::int64_t last_or_delta;
        std::cin >> operation >> first >> last_or_delta;
        if (operation == 1) sums.add(first - 1, last_or_delta);
        else std::cout << sums.range_sum(first - 1, static_cast<std::size_t>(last_or_delta)) << '\n';
    }
}
