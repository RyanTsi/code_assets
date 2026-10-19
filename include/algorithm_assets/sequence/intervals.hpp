#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
namespace algorithm_assets {
struct Interval {
    std::int64_t first, last;
    friend bool operator==(Interval, Interval) = default;
};
inline void validate_interval(Interval x) {
    if (x.first > x.last)
        throw std::invalid_argument("Reversed interval");
}
inline std::vector<Interval> merge_intervals(std::vector<Interval> intervals,
                                             bool merge_touching = true) {
    for (auto x : intervals)
        validate_interval(x);
    std::sort(intervals.begin(), intervals.end(), [](auto a, auto b) {
        return a.first < b.first || (a.first == b.first && a.last < b.last);
    });
    std::size_t count = 0;
    for (auto x : intervals) {
        if (x.first == x.last)
            continue;
        if (count && (x.first < intervals[count - 1].last ||
                      (merge_touching && x.first == intervals[count - 1].last)))
            intervals[count - 1].last = std::max(intervals[count - 1].last, x.last);
        else
            intervals[count++] = x;
    }
    intervals.resize(count);
    if (intervals.capacity() > 2 * intervals.size())
        std::vector<Interval>(intervals).swap(intervals);
    return intervals;
}
inline std::vector<Interval> merge_intervals(std::span<const Interval> intervals,
                                             bool merge_touching = true) {
    return merge_intervals(std::vector<Interval>(intervals.begin(), intervals.end()),
                           merge_touching);
}
inline std::optional<std::vector<std::size_t>>
minimum_interval_cover(std::span<const Interval> intervals, Interval target) {
    validate_interval(target);
    for (auto x : intervals)
        validate_interval(x);
    std::vector<std::size_t> result;
    if (target.first == target.last)
        return result;
    std::vector<std::size_t> order(intervals.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](auto a, auto b) {
        return intervals[a].first < intervals[b].first ||
               (intervals[a].first == intervals[b].first && a < b);
    });
    std::size_t cursor = 0;
    auto reached = target.first;
    while (reached < target.last) {
        auto best = reached;
        auto chosen = intervals.size();
        while (cursor < order.size() && intervals[order[cursor]].first <= reached) {
            const auto index = order[cursor++];
            if (intervals[index].last > best) {
                best = intervals[index].last;
                chosen = index;
            }
        }
        if (chosen == intervals.size())
            return std::nullopt;
        result.push_back(chosen);
        reached = best;
    }
    return result;
}
} // namespace algorithm_assets
