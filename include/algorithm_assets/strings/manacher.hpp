#pragma once
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>
namespace algorithm_assets {
struct PalindromeRadii {
    std::vector<std::size_t> odd, even;
    std::size_t longest_first = 0, longest_last = 0;
};
inline PalindromeRadii palindrome_radii(std::string_view text) {
    const auto n = text.size();
    if (n > std::numeric_limits<std::size_t>::max() / 2)
        throw std::length_error("Palindrome input too large");
    PalindromeRadii result{std::vector<std::size_t>(n), std::vector<std::size_t>(n), 0, 0};
    auto consider = [&](std::size_t first, std::size_t last) {
        if (last - first > result.longest_last - result.longest_first ||
            (last - first == result.longest_last - result.longest_first &&
             first < result.longest_first)) {
            result.longest_first = first;
            result.longest_last = last;
        }
    };
    std::size_t left = 0, right = 0;
    for (std::size_t i = 0; i < n; ++i) {
        auto radius = i < right ? std::min(result.odd[left + (right - 1 - i)], right - i) : 1;
        while (radius <= i && radius < n - i && text[i - radius] == text[i + radius])
            ++radius;
        result.odd[i] = radius;
        consider(i - (radius - 1), i + radius);
        if (i + radius > right) {
            left = i - (radius - 1);
            right = i + radius;
        }
    }
    left = right = 0;
    for (std::size_t i = 0; i < n; ++i) {
        auto radius = i < right ? std::min(result.even[left + (right - i)], right - i) : 0;
        while (radius < i && radius < n - i && text[i - radius - 1] == text[i + radius])
            ++radius;
        result.even[i] = radius;
        consider(i - radius, i + radius);
        if (i + radius > right) {
            left = i - radius;
            right = i + radius;
        }
    }
    return result;
}
} // namespace algorithm_assets
