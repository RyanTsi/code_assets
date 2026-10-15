#include <algorithm_assets/strings/kmp.hpp>
#include <iostream>
#include <string>
#include <utility>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string text, pattern;
    std::cin >> text >> pattern;
    algorithm_assets::KmpPattern matcher(std::move(pattern));
    matcher.for_each_match(text, [](std::size_t offset) { std::cout << offset + 1 << '\n'; });
    for (auto border : matcher.prefix_function()) std::cout << border << ' ';
    std::cout << '\n';
}
