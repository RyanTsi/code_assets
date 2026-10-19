#include <algorithm_assets/numeric/fft.hpp>
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n, m;
    std::cin >> n >> m;
    std::vector<double> a(n + 1), b(m + 1);
    for (auto& x : a)
        std::cin >> x;
    for (auto& x : b)
        std::cin >> x;
    // Judge-specific rounding for its bounded nonnegative integer coefficients.
    // The public convolution API itself remains approximate real arithmetic.
    for (auto x : algorithm_assets::convolve_real(a, b))
        std::cout << std::llround(x) << ' ';
    std::cout << '\n';
}
