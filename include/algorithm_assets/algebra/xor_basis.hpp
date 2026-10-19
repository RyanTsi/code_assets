#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace algorithm_assets {

// Echelon basis of 64-bit vectors over GF(2), including the top bit.
// Enumeration refers to DISTINCT span values and always includes empty XOR=0.
class XorBasis64 {
public:
    bool insert(std::uint64_t value) noexcept {
        for (int bit = 63; bit >= 0; --bit) {
            if (!(value & (std::uint64_t{1} << bit)))
                continue;
            if (rows_[bit])
                value ^= rows_[bit];
            else {
                rows_[bit] = value;
                ++rank_;
                return true;
            }
        }
        return false;
    }
    [[nodiscard]] bool contains(std::uint64_t value) const noexcept {
        for (int bit = 63; bit >= 0; --bit)
            if (value & (std::uint64_t{1} << bit))
                value ^= rows_[bit];
        return value == 0;
    }
    [[nodiscard]] std::size_t rank() const noexcept {
        return rank_;
    }
    [[nodiscard]] std::uint64_t maximize(std::uint64_t seed = 0) const noexcept {
        for (int bit = 63; bit >= 0; --bit)
            if ((seed ^ rows_[bit]) > seed)
                seed ^= rows_[bit];
        return seed;
    }
    [[nodiscard]] std::optional<std::uint64_t> minimum_nonzero() const noexcept {
        for (const auto row : rows_)
            if (row)
                return row;
        return std::nullopt;
    }
    void merge(const XorBasis64& other) noexcept {
        for (const auto row : other.rows_)
            insert(row);
    }
    [[nodiscard]] std::optional<std::uint64_t> kth_span_value(std::uint64_t index) const noexcept {
        if (rank_ < 64 && index >= (std::uint64_t{1} << rank_))
            return std::nullopt;
        auto reduced = rows_; // Local RREF: const reads do not mutate shared state.
        for (int bit = 0; bit < 64; ++bit) {
            if (!reduced[bit])
                continue;
            for (int higher = bit + 1; higher < 64; ++higher)
                if (reduced[higher] & (std::uint64_t{1} << bit))
                    reduced[higher] ^= reduced[bit];
        }
        std::uint64_t result = 0;
        std::size_t ordinal = 0;
        for (const auto row : reduced)
            if (row) {
                if (index & (std::uint64_t{1} << ordinal))
                    result ^= row;
                ++ordinal;
            }
        return result;
    }

private:
    std::array<std::uint64_t, 64> rows_{};
    std::size_t rank_ = 0;
};

} // namespace algorithm_assets
