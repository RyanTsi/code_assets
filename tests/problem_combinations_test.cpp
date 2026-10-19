#include <algorithm_assets/recipes/disjoint_interval_pair.hpp>
#include <algorithm_assets/recipes/bounded_step_subsequence.hpp>
#include <algorithm_assets/recipes/tree_eccentricities.hpp>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace aa = algorithm_assets;
void require(bool value) {
    if (!value)
        throw std::runtime_error("Combination check failed");
}
template <class Error, class Function> void rejects(Function function) {
    try {
        function();
    } catch (const Error&) {
        return;
    }
    throw std::runtime_error("Missing expected error");
}
std::size_t combination_odr_check();

std::size_t subsequence_oracle(const std::vector<std::int64_t>& values, std::int64_t gap) {
    std::vector<std::size_t> dp(values.size(), 1);
    std::size_t answer = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            const auto distance =
                values[i] >= values[j]
                    ? static_cast<std::uint64_t>(values[i]) - static_cast<std::uint64_t>(values[j])
                    : static_cast<std::uint64_t>(values[j]) - static_cast<std::uint64_t>(values[i]);
            if (distance <= static_cast<std::uint64_t>(gap))
                dp[i] = std::max(dp[i], dp[j] + 1);
        }
        answer = std::max(answer, dp[i]);
    }
    return answer;
}
void interval_check(const std::vector<aa::PricedInterval>& intervals, std::int64_t duration) {
    std::optional<std::uint64_t> best;
    for (std::size_t i = 0; i < intervals.size(); ++i)
        for (std::size_t j = i + 1; j < intervals.size(); ++j) {
            const auto& a = intervals[i];
            const auto& b = intervals[j];
            const auto combined_length = static_cast<std::uint64_t>(a.end - a.begin) +
                                         static_cast<std::uint64_t>(b.end - b.begin);
            if ((a.end <= b.begin || b.end <= a.begin) &&
                combined_length == static_cast<std::uint64_t>(duration)) {
                const auto cost =
                    static_cast<std::uint64_t>(a.cost) + static_cast<std::uint64_t>(b.cost);
                if (!best || cost < *best)
                    best = cost;
            }
        }
    const auto actual = aa::cheapest_disjoint_interval_pair(intervals, duration);
    require(bool(best) == bool(actual));
    if (!actual)
        return;
    require(static_cast<std::uint64_t>(actual->cost) == *best);
    require(actual->first != actual->second);
    const auto& a = intervals[actual->first];
    const auto& b = intervals[actual->second];
    require(a.end <= b.begin && a.end - a.begin + b.end - b.begin == duration);
    require(a.cost + b.cost == actual->cost);
}
void tree_check(std::size_t n, const std::vector<aa::UnweightedCsrGraph::Edge>& edges,
                std::uint32_t root) {
    std::vector<std::vector<std::uint32_t>> d(n, std::vector<std::uint32_t>(n, 100000));
    for (std::size_t i = 0; i < n; ++i)
        d[i][i] = 0;
    for (auto edge : edges)
        d[edge.from][edge.to] = d[edge.to][edge.from] = 1;
    for (std::size_t k = 0; k < n; ++k)
        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
    const auto result = aa::tree_eccentricities(n, edges, root);
    require(result.source_distances == d[root]);
    std::uint32_t diameter = 0;
    for (std::size_t i = 0; i < n; ++i) {
        const auto eccentricity = *std::max_element(d[i].begin(), d[i].end());
        require(result.eccentricities[i] == eccentricity);
        diameter = std::max(diameter, eccentricity);
    }
    require(result.diameter_length == diameter &&
            d[result.diameter_first][result.diameter_second] == diameter);
}
int main() {
    using Limits = std::numeric_limits<std::int64_t>;
    const auto lo = Limits::min(), hi = Limits::max();
    require(combination_odr_check() == 2);
    require(aa::bounded_step_subsequence_length({}, 0) == 0);
    rejects<std::invalid_argument>([] {
        aa::bounded_step_subsequence_length({}, -1);
    });
    for (const auto gap : {std::int64_t{0}, std::int64_t{1}, hi}) {
        const std::vector<std::int64_t> extremes{lo, lo + 1, -1, 0, hi - 1, hi, lo, hi};
        require(aa::bounded_step_subsequence_length(extremes, gap) ==
                subsequence_oracle(extremes, gap));
    }
    require(aa::bounded_step_subsequence_length(std::vector<std::int64_t>{0, 100, 101}, 1) == 2);
    interval_check({}, 2);
    interval_check({{0, 1, 0}, {1, 2, 0}}, 2); // Touching half-open intervals are disjoint.
    interval_check({{0, 2, 3}, {1, 3, 1}}, 4);
    interval_check({{0, 1, hi}, {1, 2, 0}}, 2);
    rejects<std::overflow_error>([&] {
        aa::cheapest_disjoint_interval_pair(std::vector<aa::PricedInterval>{{0, 1, hi}, {1, 2, 1}},
                                            2);
    });
    interval_check({{0, 1, hi}, {1, 2, hi}, {3, 4, 1}, {4, 5, 2}},
                   2); // Dominated overflow must not reject optimum.
    rejects<std::overflow_error>([&] {
        aa::cheapest_disjoint_interval_pair(std::vector<aa::PricedInterval>{{lo, 0, 0}}, 1);
    });
    interval_check({{lo, -1, 0}, {0, 1, 0}},
                   hi); // Full duration boundary, but no representable target pair.
    rejects<std::invalid_argument>([] {
        aa::cheapest_disjoint_interval_pair({}, 0);
    });
    rejects<std::invalid_argument>([] {
        aa::cheapest_disjoint_interval_pair(std::vector<aa::PricedInterval>{{1, 1, 0}}, 2);
    });
    rejects<std::invalid_argument>([] {
        aa::cheapest_disjoint_interval_pair(std::vector<aa::PricedInterval>{{0, 1, -1}}, 2);
    });
    tree_check(1, {}, 0);
    rejects<std::invalid_argument>([] {
        aa::tree_eccentricities(0, {});
    });
    rejects<std::out_of_range>([] {
        aa::tree_eccentricities(1, {}, 1);
    });
    rejects<std::invalid_argument>([] {
        aa::tree_eccentricities(3, std::vector<aa::UnweightedCsrGraph::Edge>{{0, 1}});
    });
    rejects<std::invalid_argument>([] {
        aa::tree_eccentricities(3, std::vector<aa::UnweightedCsrGraph::Edge>{{0, 1}, {0, 1}});
    });
    rejects<std::invalid_argument>([] {
        aa::tree_eccentricities(2, std::vector<aa::UnweightedCsrGraph::Edge>{{0, 0}});
    });
    rejects<std::out_of_range>([] {
        aa::tree_eccentricities(2, std::vector<aa::UnweightedCsrGraph::Edge>{{0, 2}});
    });
    std::mt19937_64 rng(1015);
    for (int trial = 0; trial < 400; ++trial) {
        std::vector<std::int64_t> values(rng() % 45);
        for (auto& value : values)
            value = static_cast<std::int64_t>(rng() % 81) - 40;
        const auto gap = static_cast<std::int64_t>(rng() % 50);
        const auto original = values;
        require(aa::bounded_step_subsequence_length(values, gap) ==
                subsequence_oracle(values, gap));
        require(values == original);
        std::vector<aa::PricedInterval> intervals(rng() % 30);
        for (auto& interval : intervals) {
            interval.begin = static_cast<std::int64_t>(rng() % 51) - 25;
            interval.end = interval.begin + 1 + static_cast<std::int64_t>(rng() % 12);
            interval.cost = static_cast<std::int64_t>(rng() % 100);
        }
        interval_check(intervals, 1 + static_cast<std::int64_t>(rng() % 26));
        const auto n = static_cast<std::uint32_t>(1 + rng() % 25);
        std::vector<aa::UnweightedCsrGraph::Edge> edges;
        for (std::uint32_t v = 1; v < n; ++v)
            edges.push_back({v, static_cast<std::uint32_t>(rng() % v)});
        std::shuffle(edges.begin(), edges.end(), rng);
        tree_check(n, edges, static_cast<std::uint32_t>(rng() % n));
    }
    std::vector<aa::UnweightedCsrGraph::Edge> chain;
    for (std::uint32_t i = 1; i < 30000; ++i)
        chain.push_back({i - 1, i});
    const auto long_tree = aa::tree_eccentricities(30000, chain);
    require(long_tree.diameter_length == 29999 && long_tree.eccentricities[15000] == 15000);
    std::cout
        << "Three callable recipes: 1200 randomized oracle checks, boundaries, ODR, 30000-node chain passed\n";
}
