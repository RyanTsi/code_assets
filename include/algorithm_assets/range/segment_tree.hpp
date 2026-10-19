#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace algorithm_assets {

// Associativity and identity are semantic requirements, not provable by concepts.
template <class T, class Combine>
    requires requires(const Combine& combine, const T& first, const T& second) {
        { std::invoke(combine, first, second) } -> std::same_as<T>;
    }
class SegmentTree {
public:
    SegmentTree(std::span<const T> values, T identity, Combine combine)
        : size_(values.size()), identity_(std::move(identity)), combine_(std::move(combine)) {
        if (size_ > tree_.max_size() / 2) {
            throw std::length_error("SegmentTree storage size is too large");
        }
        tree_.assign(2 * size_, identity_);
        for (std::size_t index = 0; index < size_; ++index) {
            tree_[size_ + index] = values[index];
        }
        for (auto index = size_; index > 1;) {
            --index;
            tree_[index] = std::invoke(combine_, tree_[2 * index], tree_[2 * index + 1]);
        }
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] const T& get(std::size_t index) const {
        check_index(index);
        return tree_[size_ + index];
    }

    void set(std::size_t index, T value) {
        check_index(index);
        index += size_;
        tree_[index] = std::move(value);
        while (index > 1) {
            index /= 2;
            tree_[index] = std::invoke(combine_, tree_[2 * index], tree_[2 * index + 1]);
        }
    }

    [[nodiscard]] T fold(std::size_t first, std::size_t last) const {
        if (first > last || last > size_) {
            throw std::out_of_range("SegmentTree requires 0 <= first <= last <= size");
        }
        auto left = identity_;
        auto right = identity_;
        for (first += size_, last += size_; first < last; first /= 2, last /= 2) {
            if (first % 2 != 0) {
                left = std::invoke(combine_, left, tree_[first++]);
            }
            if (last % 2 != 0) {
                right = std::invoke(combine_, tree_[--last], right);
            }
        }
        return std::invoke(combine_, left, right);
    }

    // Compact 2n layout can rotate the root's order when n is not a power of two.
    // Query instead of returning tree_[1], preserving noncommutative monoids.
    [[nodiscard]] T all_fold() const {
        return fold(0, size_);
    }

private:
    void check_index(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("SegmentTree index is out of range");
        }
    }

    std::size_t size_;
    T identity_;
    [[no_unique_address]] Combine combine_;
    std::vector<T> tree_;
};

} // namespace algorithm_assets
