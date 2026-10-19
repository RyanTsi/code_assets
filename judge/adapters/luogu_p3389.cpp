#include <algorithm_assets/numeric/dense_lu.hpp>
#include <iomanip>
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n;
    std::cin >> n;
    std::vector<double> matrix(n * n), right(n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j)
            std::cin >> matrix[i * n + j];
        std::cin >> right[i];
    }
    try {
        algorithm_assets::DenseLu factor(n, std::move(matrix));
        for (auto x : factor.solve(right))
            std::cout << std::fixed << std::setprecision(2) << x << '\n';
    } catch (const std::domain_error&) {
        std::cout << "No Solution\n";
    }
}
