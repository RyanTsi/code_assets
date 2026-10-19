#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
namespace algorithm_assets {
class CoordinateCompression {
public:
    explicit CoordinateCompression(std::vector<std::int64_t> values) : keys_(std::move(values)) {
        std::sort(keys_.begin(), keys_.end());
        keys_.erase(std::unique(keys_.begin(), keys_.end()), keys_.end());
        if (keys_.capacity() > 2 * keys_.size())
            std::vector<std::int64_t>(keys_).swap(keys_);
    }
    explicit CoordinateCompression(std::span<const std::int64_t> values)
        : CoordinateCompression(std::vector<std::int64_t>(values.begin(), values.end())) {
    }
    [[nodiscard]] std::size_t lower_rank(std::int64_t value) const {
        return static_cast<std::size_t>(std::lower_bound(keys_.begin(), keys_.end(), value) -
                                        keys_.begin());
    }
    [[nodiscard]] std::optional<std::size_t> rank(std::int64_t value) const {
        auto i = lower_rank(value);
        return i < keys_.size() && keys_[i] == value ? std::optional<std::size_t>{i} : std::nullopt;
    }
    [[nodiscard]] std::int64_t value_at(std::size_t rank) const {
        return keys_.at(rank);
    }
    [[nodiscard]] std::span<const std::int64_t> keys() const noexcept {
        return keys_;
    }
    [[nodiscard]] std::size_t size() const noexcept {
        return keys_.size();
    }

private:
    std::vector<std::int64_t> keys_;
};
} // namespace algorithm_assets
