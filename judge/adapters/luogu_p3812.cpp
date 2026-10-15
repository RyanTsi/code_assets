#include <algorithm_assets/algebra/xor_basis.hpp>
#include <cstdint>
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n;
    std::cin >> n;
    algorithm_assets::XorBasis64 basis;
    while (n--) {
        std::uint64_t value;
        std::cin >> value;
        basis.insert(value);
    }
    std::cout << basis.maximize() << '\n';
}
