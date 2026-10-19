#include <algorithm_assets/recipes/disjoint_interval_pair.hpp>
#include <algorithm_assets/recipes/bounded_step_subsequence.hpp>
#include <algorithm_assets/recipes/tree_eccentricities.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>
std::size_t combination_odr_check() {
    return algorithm_assets::bounded_step_subsequence_length(std::vector<std::int64_t>{7, 7}, 0);
}
