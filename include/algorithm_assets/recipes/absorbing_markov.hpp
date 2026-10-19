#pragma once
#include <algorithm_assets/numeric/dense_lu.hpp>
#include <cmath>
#include <limits>

namespace algorithm_assets {
struct AbsorbingMarkovResult {
    std::size_t transient_count, absorbing_count;
    std::vector<double> absorption_probabilities; // row-major transient by absorbing
    std::vector<double> expected_steps;
};
inline AbsorbingMarkovResult
analyze_absorbing_markov(std::size_t n, std::size_t transient, std::span<const double> transition,
                         double probability_tolerance = 1e-12,
                         double pivot_tolerance = 64 * std::numeric_limits<double>::epsilon()) {
    if (n >= std::numeric_limits<std::uint32_t>::max() || transient > n ||
        (n && n > std::numeric_limits<std::size_t>::max() / n) || transition.size() != n * n)
        throw std::invalid_argument("Invalid Markov matrix shape");
    if (!std::isfinite(probability_tolerance) || probability_tolerance < 0 ||
        probability_tolerance > 1e-6)
        throw std::invalid_argument("Invalid probability tolerance");
    for (std::size_t i = 0; i < n; ++i) {
        CompensatedSum total;
        for (std::size_t j = 0; j < n; ++j) {
            const auto p = transition[i * n + j];
            if (!std::isfinite(p) || p < 0 || p > 1)
                throw std::invalid_argument("Invalid transition probability");
            if (i >= transient && p != (i == j ? 1.0 : 0.0))
                throw std::invalid_argument("Absorbing rows must be identity rows");
            total.add(p);
        }
        if (std::abs(total.value() - 1) > probability_tolerance)
            throw std::invalid_argument("Markov row sum is not one");
    }
    const auto absorbing = n - transient;
    AbsorbingMarkovResult result{transient, absorbing, {}, {}};
    if (!transient)
        return result;
    // Every transient state must reach an absorbing state in the positive-edge graph.
    std::vector<bool> reaches(n);
    std::vector<std::uint32_t> queue;
    for (std::size_t i = transient; i < n; ++i) {
        reaches[i] = true;
        queue.push_back(static_cast<std::uint32_t>(i));
    }
    for (std::size_t k = 0; k < queue.size(); ++k)
        for (std::size_t i = 0; i < transient; ++i)
            if (!reaches[i] && transition[i * n + queue[k]] > 0) {
                reaches[i] = true;
                queue.push_back(static_cast<std::uint32_t>(i));
            }
    for (std::size_t i = 0; i < transient; ++i)
        if (!reaches[i])
            throw std::domain_error("Transient state cannot reach absorption");
    std::vector<double> coefficients(transient * transient);
    for (std::size_t i = 0; i < transient; ++i)
        for (std::size_t j = 0; j < transient; ++j)
            coefficients[i * transient + j] = (i == j ? 1.0 : 0.0) - transition[i * n + j];
    DenseLu factorization(transient, std::move(coefficients), pivot_tolerance);
    result.absorption_probabilities.resize(transient * absorbing);
    std::vector<double> right(transient, 1);
    result.expected_steps = factorization.solve(right);
    for (std::size_t column = 0; column < absorbing; ++column) {
        for (std::size_t i = 0; i < transient; ++i)
            right[i] = transition[i * n + transient + column];
        auto solution = factorization.solve(right);
        for (std::size_t i = 0; i < transient; ++i)
            result.absorption_probabilities[i * absorbing + column] = solution[i];
    }
    return result;
}
} // namespace algorithm_assets
