#pragma once

#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace algorithm_assets {

inline std::int64_t checked_int64_add(std::int64_t a, std::int64_t b) {
    if ((b > 0 && a > std::numeric_limits<std::int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<std::int64_t>::min() - b))
        throw std::overflow_error("int64 addition overflow");
    return a + b;
}

// Two-word signed accumulation, implemented without nonstandard __int128.
// Intermediate totals may exceed int64; value() narrows only the FINAL sum.
class WideInt64Sum {
public:
    void add(std::int64_t value) {
        const auto next = low_ + static_cast<std::uint64_t>(value);
        const auto carry = next < low_ ? 1 : 0;
        const auto high = checked_int64_add(high_, (value < 0 ? -1 : 0) + carry);
        low_ = next;
        high_ = high;
    }
    void subtract(std::int64_t value) {
        const auto word = static_cast<std::uint64_t>(value);
        const auto borrow = low_ < word ? 1 : 0;
        const auto high = checked_int64_add(high_, (value < 0 ? 1 : 0) - borrow);
        low_ -= word;
        high_ = high;
    }
    [[nodiscard]] std::int64_t value() const {
        const auto sign_bit = std::uint64_t{1} << 63;
        if ((high_ == 0 && low_ < sign_bit) || (high_ == -1 && low_ >= sign_bit))
            return std::bit_cast<std::int64_t>(low_);
        throw std::overflow_error("Final sum does not fit int64");
    }
    void clear() noexcept {
        low_ = 0;
        high_ = 0;
    }

private:
    std::uint64_t low_ = 0;
    std::int64_t high_ = 0;
};
static_assert(sizeof(WideInt64Sum) == 16);

} // namespace algorithm_assets
