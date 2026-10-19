#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {

static_assert(sizeof(std::size_t) <= sizeof(std::uint64_t),
              "Inversion counting requires size_t of at most 64 bits");

inline void check_inversion_count_size(std::size_t size) {
    if (size < 2)
        return;
    auto first = static_cast<std::uint64_t>(size), second = first - 1;
    if (first % 2 == 0)
        first /= 2;
    else
        second /= 2;
    if (first > std::numeric_limits<std::uint64_t>::max() / second)
        throw std::length_error("Potential inversion count exceeds uint64");
}

class InversionCounter {
public:
    [[nodiscard]] std::uint64_t count(std::span<const std::int64_t> values) {
        check_inversion_count_size(values.size());
        values_.assign(values.begin(), values.end());
        return count_inplace(values_);
    }
    // Sorts the caller's array. Allocation/size errors happen before mutation.
    [[nodiscard]] std::uint64_t count_inplace(std::span<std::int64_t> values) {
        check_inversion_count_size(values.size());
        scratch_.assign(values.begin(), values.end());
        return merge_count(scratch_, values, 0, values.size());
    }
    [[nodiscard]] std::size_t storage_bytes() const noexcept {
        return (values_.capacity() + scratch_.capacity()) * sizeof(std::int64_t);
    }
    void release() noexcept {
        std::vector<std::int64_t>{}.swap(values_);
        std::vector<std::int64_t>{}.swap(scratch_);
    }

private:
    static std::uint64_t merge_count(std::span<std::int64_t> source, std::span<std::int64_t> output,
                                     std::size_t first, std::size_t last) {
        if (last - first < 2)
            return 0;
        const auto middle = first + (last - first) / 2;
        auto count = merge_count(output, source, first, middle);
        count += merge_count(output, source, middle, last);
        auto left = first, right = middle, destination = first;
        while (left < middle && right < last) {
            if (source[left] <= source[right])
                output[destination++] = source[left++];
            else {
                output[destination++] = source[right++];
                count += middle - left;
            }
        }
        while (left < middle)
            output[destination++] = source[left++];
        while (right < last)
            output[destination++] = source[right++];
        return count;
    }
    std::vector<std::int64_t> values_, scratch_;
};

inline std::uint64_t count_inversions(std::span<const std::int64_t> values) {
    InversionCounter counter;
    return counter.count(values);
}

} // namespace algorithm_assets
