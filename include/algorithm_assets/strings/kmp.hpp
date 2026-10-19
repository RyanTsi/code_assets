#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace algorithm_assets {

// Own the compiled pattern; text is borrowed for each call. Byte semantics,
// including embedded NULs. An empty pattern matches every boundary [0,n].
class KmpPattern {
public:
    explicit KmpPattern(std::string pattern)
        : pattern_(std::move(pattern)), prefix_(pattern_.size()) {
        for (std::size_t i = 1, matched = 0; i < pattern_.size(); ++i) {
            while (matched && pattern_[i] != pattern_[matched])
                matched = prefix_[matched - 1];
            if (pattern_[i] == pattern_[matched])
                ++matched;
            prefix_[i] = matched;
        }
    }
    [[nodiscard]] std::string_view pattern() const noexcept {
        return pattern_;
    }
    [[nodiscard]] std::span<const std::size_t> prefix_function() const noexcept {
        return prefix_;
    }
    [[nodiscard]] std::optional<std::size_t> find_first(std::string_view text) const {
        std::optional<std::size_t> result;
        scan(text, [&](std::size_t offset) {
            result = offset;
            return false;
        });
        return result;
    }
    [[nodiscard]] bool contains(std::string_view text) const {
        return find_first(text).has_value();
    }
    template <class Emit> void for_each_match(std::string_view text, Emit&& emit) const {
        scan(text, [&](std::size_t offset) {
            std::invoke(emit, offset);
            return true;
        });
    }
    [[nodiscard]] std::vector<std::size_t> find_all(std::string_view text) const {
        std::vector<std::size_t> result;
        for_each_match(text, [&](std::size_t offset) {
            result.push_back(offset);
        });
        return result;
    }

private:
    template <class Visit> void scan(std::string_view text, Visit visit) const {
        if (pattern_.empty()) {
            for (std::size_t offset = 0;; ++offset) {
                if (!visit(offset) || offset == text.size())
                    break;
            }
            return;
        }
        for (std::size_t i = 0, matched = 0; i < text.size(); ++i) {
            while (matched && text[i] != pattern_[matched])
                matched = prefix_[matched - 1];
            if (text[i] == pattern_[matched])
                ++matched;
            if (matched == pattern_.size()) {
                if (!visit(i + 1 - matched))
                    return;
                matched = prefix_[matched - 1]; // Preserve overlapping matches.
            }
        }
    }
    std::string pattern_;
    std::vector<std::size_t> prefix_;
};

} // namespace algorithm_assets
