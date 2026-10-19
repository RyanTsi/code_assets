#pragma once
#include <bit>
#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
namespace algorithm_assets {
template <class T, class Combine> class SparseTable {
public:
    SparseTable(std::span<const T> values, Combine combine)
        : n_(values.size()), combine_(std::move(combine)) {
        std::size_t total = 0;
        for (std::size_t width = 1; width <= n_;) {
            const auto count = n_ - width + 1;
            if (count > data_.max_size() - total)
                throw std::length_error("Sparse table too large");
            offsets_.push_back(total);
            total += count;
            if (width > n_ / 2)
                break;
            width *= 2;
        }
        data_.reserve(total);
        data_.insert(data_.end(), values.begin(), values.end());
        for (std::size_t level = 1; level < offsets_.size(); ++level) {
            const auto half = std::size_t{1} << (level - 1), width = 2 * half;
            for (std::size_t i = 0; i <= n_ - width; ++i)
                data_.push_back(std::invoke(combine_, data_[offsets_[level - 1] + i],
                                            data_[offsets_[level - 1] + i + half]));
        }
    }
    [[nodiscard]] T fold(std::size_t first, std::size_t last) const {
        if (first >= last || last > n_)
            throw std::out_of_range("SparseTable requires a nonempty valid range");
        const auto level = std::bit_width(last - first) - 1;
        const auto width = std::size_t{1} << level;
        return std::invoke(combine_, data_[offsets_[level] + first],
                           data_[offsets_[level] + last - width]);
    }
    [[nodiscard]] std::size_t size() const noexcept {
        return n_;
    }
    [[nodiscard]] std::size_t stored_values() const noexcept {
        return data_.size();
    }

private:
    std::size_t n_;
    Combine combine_;
    std::vector<std::size_t> offsets_;
    std::vector<T> data_;
};
} // namespace algorithm_assets
