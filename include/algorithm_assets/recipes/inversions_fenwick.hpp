#pragma once

#include <algorithm_assets/range/fenwick_sum.hpp>
#include <algorithm_assets/sequence/coordinate_compression.hpp>
#include <algorithm_assets/sequence/inversions.hpp>

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

namespace algorithm_assets {

inline std::uint64_t count_inversions_fenwick(std::span<const std::int64_t> values) {
    check_inversion_count_size(values.size());
    CoordinateCompression keys(values);
    FenwickSum frequencies(keys.size());
    std::uint64_t count = 0;
    for (auto i = values.size(); i > 0;) {
        const auto rank = keys.lower_rank(values[--i]);
        count += static_cast<std::uint64_t>(frequencies.prefix_sum(rank));
        frequencies.add(rank, 1);
    }
    return count;
}

} // namespace algorithm_assets
