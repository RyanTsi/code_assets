#pragma once
#include <algorithm_assets/numeric/wide_int64_sum.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
namespace algorithm_assets {
struct KnapsackItem {
    std::size_t cost;
    std::int64_t value;
};
struct BoundedKnapsackItem {
    std::size_t cost;
    std::int64_t value;
    std::size_t count;
};
namespace detail {
inline std::vector<std::int64_t> knapsack_table(std::size_t capacity) {
    std::vector<std::int64_t> table;
    if (capacity >= table.max_size())
        throw std::length_error("Knapsack capacity too large");
    table.resize(capacity + 1);
    return table;
}
inline std::int64_t positive_product(std::int64_t value, std::size_t count) {
    if (value < 0 || (value && count > static_cast<std::uint64_t>(
                                           std::numeric_limits<std::int64_t>::max() / value)))
        throw std::overflow_error("Knapsack optimum exceeds int64");
    return value ? value * static_cast<std::int64_t>(count) : 0;
}
} // namespace detail
inline std::int64_t knapsack_01(std::span<const KnapsackItem> items, std::size_t capacity) {
    auto table = detail::knapsack_table(capacity);
    for (auto item : items)
        if (item.cost <= capacity && item.value > 0)
            for (auto budget = capacity;; --budget) {
                table[budget] = std::max(table[budget],
                                         checked_int64_add(table[budget - item.cost], item.value));
                if (budget == item.cost)
                    break;
            }
    return table.back();
}
inline std::int64_t knapsack_unbounded(std::span<const KnapsackItem> items, std::size_t capacity) {
    for (auto item : items)
        if (!item.cost && item.value > 0)
            throw std::domain_error("Unbounded zero-cost positive item");
    auto table = detail::knapsack_table(capacity);
    for (auto item : items)
        if (item.cost <= capacity && item.cost && item.value > 0)
            for (auto budget = item.cost; budget <= capacity; ++budget)
                table[budget] = std::max(table[budget],
                                         checked_int64_add(table[budget - item.cost], item.value));
    return table.back();
}
inline std::int64_t knapsack_bounded(std::span<const BoundedKnapsackItem> items,
                                     std::size_t capacity) {
    auto table = detail::knapsack_table(capacity);
    struct Candidate {
        std::size_t position;
        std::int64_t value;
    };
    std::vector<Candidate> queue;
    for (auto item : items) {
        if (item.value <= 0 || !item.count || item.cost > capacity)
            continue;
        if (!item.cost) {
            const auto free_value = detail::positive_product(item.value, item.count);
            for (auto& value : table)
                value = checked_int64_add(value, free_value);
            continue;
        }
        const auto maximum = capacity / item.cost;
        // Reject a genuine feasible overflow, not a huge artificial k*value term.
        (void)detail::positive_product(item.value, std::min(item.count, maximum));
        queue.resize(maximum + 1);
        for (std::size_t residue = 0; residue < item.cost; ++residue) {
            std::size_t head = 0, tail = 0, k = 0;
            for (auto budget = residue; budget <= capacity;) {
                const auto old = table[budget];
                auto evaluate = [&](Candidate candidate) {
                    return checked_int64_add(
                        candidate.value,
                        detail::positive_product(item.value, k - candidate.position));
                };
                while (head < tail && k - queue[head].position > item.count)
                    ++head;
                while (head < tail && evaluate(queue[tail - 1]) <= old)
                    --tail;
                queue[tail++] = {k, old};
                table[budget] = evaluate(queue[head]);
                if (capacity - budget < item.cost)
                    break;
                budget += item.cost;
                ++k;
            }
        }
    }
    return table.back();
}
} // namespace algorithm_assets
