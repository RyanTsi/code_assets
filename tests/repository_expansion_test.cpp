#include <algorithm_assets/graph/two_sat.hpp>
#include <algorithm_assets/graph/dinic.hpp>
#include <algorithm_assets/graph/bipartite_matching.hpp>
#include <algorithm_assets/graph/bellman_ford.hpp>
#include <algorithm_assets/graph/lowest_common_ancestor.hpp>
#include <algorithm_assets/graph/topological_sort.hpp>
#include <algorithm_assets/sequence/intervals.hpp>
#include <algorithm_assets/sequence/subsequences.hpp>
#include <algorithm_assets/sequence/coordinate_compression.hpp>
#include <algorithm_assets/strings/manacher.hpp>
#include <algorithm_assets/range/sparse_table.hpp>
#include <algorithm_assets/recipes/absorbing_markov.hpp>
#include <algorithm_assets/numeric/fft.hpp>
#include <algorithm_assets/geometry/minkowski.hpp>
#include <algorithm_assets/geometry/minimum_enclosing_circle.hpp>
#include <algorithm_assets/optimization/knapsack.hpp>
#include <algorithm_assets/algebra/modular.hpp>
#include <algorithm_assets/algebra/prime_sieve.hpp>
#include <functional>
#include <iostream>
#include <random>
#include <source_location>
#include <string>

namespace a = algorithm_assets;
namespace g = a::geometry;
namespace {
void require(bool value, std::source_location where = std::source_location::current()) {
    if (!value)
        throw std::runtime_error(std::string(where.file_name()) + ":" +
                                 std::to_string(where.line()));
}
template <class E, class F> void throws(F f) {
    bool caught = false;
    try {
        f();
    } catch (const E&) {
        caught = true;
    }
    require(caught);
}
bool close(double a, double b, double tolerance = 1e-9) {
    return std::abs(a - b) <= tolerance * std::max({1.0, std::abs(a), std::abs(b)});
}
std::mt19937_64 random(0x20261014);
void graph_components_sat() {
    for (std::size_t n = 0; n < 12; ++n)
        for (int trial = 0; trial < 80; ++trial) {
            std::vector<a::UnweightedCsrGraph::Edge> edges;
            std::vector<std::vector<bool>> reach(n, std::vector<bool>(n));
            for (std::size_t i = 0; i < n; ++i) {
                reach[i][i] = true;
                for (std::size_t j = 0; j < n; ++j)
                    if (random() % 4 == 0) {
                        edges.push_back(
                            {static_cast<std::uint32_t>(i), static_cast<std::uint32_t>(j)});
                        reach[i][j] = true;
                    }
            }
            for (std::size_t k = 0; k < n; ++k)
                for (std::size_t i = 0; i < n; ++i)
                    for (std::size_t j = 0; j < n; ++j)
                        reach[i][j] = reach[i][j] || (reach[i][k] && reach[k][j]);
            a::UnweightedCsrGraph graph(n, edges);
            auto components = a::strong_components(graph);
            for (std::size_t i = 0; i < n; ++i)
                for (std::size_t j = 0; j < n; ++j)
                    require((components.component[i] == components.component[j]) ==
                            (reach[i][j] && reach[j][i]));
            for (auto e : edges)
                require(components.component[e.from] >= components.component[e.to]);
            auto dag = a::condense_graph(graph);
            require(a::topological_sort(dag.dag).acyclic);
        }
    for (std::uint32_t n = 0; n < 8; ++n)
        for (int trial = 0; trial < 100; ++trial) {
            a::TwoSat solver(n);
            std::vector<std::pair<a::TwoSat::Literal, a::TwoSat::Literal>> clauses;
            if (n)
                for (int i = 0; i < 20; ++i) {
                    a::TwoSat::Literal x{static_cast<std::uint32_t>(random() % n),
                                         bool(random() % 2)},
                        y{static_cast<std::uint32_t>(random() % n), bool(random() % 2)};
                    clauses.push_back({x, y});
                    solver.add_clause(x, y);
                }
            auto satisfies = [&](auto bit) {
                for (auto [x, y] : clauses)
                    if (!(bit(x.variable) == x.value || bit(y.variable) == y.value))
                        return false;
                return true;
            };
            bool feasible = false;
            for (std::uint64_t mask = 0; mask < (std::uint64_t{1} << n); ++mask)
                feasible |= satisfies([&](auto i) {
                    return bool((mask >> i) & 1U);
                });
            const auto solution = solver.solve();
            require(solution.satisfiable == feasible);
            if (feasible)
                require(satisfies([&](auto i) {
                    return bool(solution.assignment[i]);
                }));
            require(solver.solve().satisfiable == feasible);
        }
    a::TwoSat invalid(1);
    throws<std::out_of_range>([&] {
        invalid.force({1, true});
    });
}
void flow_and_matching() {
    for (std::uint32_t n = 2; n <= 7; ++n)
        for (int trial = 0; trial < 100; ++trial) {
            a::Dinic network(n);
            std::vector<a::Dinic::EdgeState> edges;
            for (int i = 0; i < 20; ++i) {
                const auto from = static_cast<std::uint32_t>(random() % n),
                           to = static_cast<std::uint32_t>(random() % n);
                const auto capacity = random() % 15;
                network.add_edge(from, to, capacity);
                edges.push_back({from, to, capacity, 0});
            }
            std::uint64_t minimum = std::numeric_limits<std::uint64_t>::max();
            for (std::uint64_t mask = 0; mask < (std::uint64_t{1} << n); ++mask)
                if ((mask & 1U) && !(mask & (std::uint64_t{1} << (n - 1)))) {
                    std::uint64_t cut = 0;
                    for (auto e : edges)
                        if ((mask >> e.from & 1U) && !(mask >> e.to & 1U))
                            cut += e.capacity;
                    minimum = std::min(minimum, cut);
                }
            const auto first = network.max_flow(0, n - 1, 3), rest = network.max_flow(0, n - 1);
            require(first.added + rest.added == minimum && rest.maximal);
            auto seen = network.residual_reachable(0);
            std::uint64_t cut = 0;
            std::vector<std::int64_t> balance(n);
            for (std::size_t i = 0; i < edges.size(); ++i) {
                auto e = network.edge(i);
                require(e.capacity == edges[i].capacity && e.flow <= e.capacity);
                balance[e.from] -= static_cast<std::int64_t>(e.flow);
                balance[e.to] += static_cast<std::int64_t>(e.flow);
                if (seen[e.from] && !seen[e.to])
                    cut += e.capacity;
            }
            require(cut == minimum);
            for (std::size_t i = 1; i + 1 < n; ++i)
                require(balance[i] == 0);
            throws<std::invalid_argument>([&] {
                (void)network.max_flow(n - 1, 0);
            });
            network.reset_flow();
            require(network.max_flow(0, n - 1).added == minimum);
        }
    a::Dinic big(2);
    big.add_edge(0, 1, std::numeric_limits<std::uint64_t>::max());
    big.add_edge(0, 1, 2);
    auto limited = big.max_flow(0, 1);
    require(limited.added == std::numeric_limits<std::uint64_t>::max() && !limited.maximal);
    require(big.max_flow(0, 1).added == 2);
    for (std::size_t left = 0; left < 7; ++left)
        for (std::size_t right = 0; right < 7; ++right)
            for (int trial = 0; trial < 20; ++trial) {
                std::vector<a::UnweightedCsrGraph::Edge> edges;
                for (std::uint32_t u = 0; u < left; ++u)
                    for (std::uint32_t v = 0; v < right; ++v)
                        if (random() % 2)
                            edges.push_back({u, v});
                a::UnweightedCsrGraph graph(std::max(left, right), edges);
                auto matching = a::hopcroft_karp(left, right, graph);
                std::function<std::size_t(std::size_t, std::uint32_t)> brute =
                    [&](auto u, auto used) -> std::size_t {
                    if (u == left)
                        return 0;
                    auto best = brute(u + 1, used);
                    for (auto v : graph.neighbors(static_cast<std::uint32_t>(u)).targets)
                        if (!(used & (1U << v)))
                            best = std::max(best, 1 + brute(u + 1, used | (1U << v)));
                    return best;
                };
                require(matching.size == brute(0, 0));
                auto cover =
                    std::count(matching.cover_left.begin(), matching.cover_left.end(), true) +
                    std::count(matching.cover_right.begin(), matching.cover_right.end(), true);
                require(static_cast<std::size_t>(cover) == matching.size);
                for (auto e : edges)
                    require(matching.cover_left[e.from] || matching.cover_right[e.to]);
                for (std::size_t u = 0; u < left; ++u)
                    if (matching.left_to_right[u] != a::BipartiteMatching::unmatched)
                        require(matching.right_to_left[matching.left_to_right[u]] == u);
            }
    constexpr std::uint32_t n = 30000;
    std::vector<a::UnweightedCsrGraph::Edge> chain;
    a::Dinic network(n);
    for (std::uint32_t i = 1; i < n; ++i) {
        chain.push_back({i - 1, i});
        network.add_edge(i - 1, i, 1);
    }
    require(a::strong_components(a::UnweightedCsrGraph(n, chain)).count == n);
    require(network.max_flow(0, n - 1).added == 1);
}
void signed_paths_and_forest() {
    for (std::uint32_t n = 1; n < 9; ++n)
        for (int trial = 0; trial < 80; ++trial) {
            std::vector<a::SignedDirectedEdge> edges;
            constexpr std::int64_t inf = 1000000;
            std::vector<std::vector<std::int64_t>> d(n, std::vector<std::int64_t>(n, inf));
            for (std::uint32_t u = 0; u < n; ++u) {
                d[u][u] = 0;
                for (std::uint32_t v = 0; v < n; ++v)
                    if (random() % 5 == 0) {
                        const auto w = static_cast<std::int64_t>(random() % 15) - 5;
                        edges.push_back({u, v, w});
                        d[u][v] = std::min(d[u][v], w);
                    }
            }
            for (std::uint32_t k = 0; k < n; ++k)
                for (std::uint32_t u = 0; u < n; ++u)
                    for (std::uint32_t v = 0; v < n; ++v)
                        if (d[u][k] < inf && d[k][v] < inf)
                            d[u][v] = std::min(d[u][v], d[u][k] + d[k][v]);
            for (std::uint32_t source = 0; source < n; ++source) {
                auto actual = a::bellman_ford(n, edges, source);
                for (std::uint32_t v = 0; v < n; ++v) {
                    bool negative = false;
                    for (std::uint32_t k = 0; k < n; ++k)
                        negative |= d[source][k] < inf && d[k][k] < 0 && d[k][v] < inf;
                    if (negative)
                        require(actual[v].status == a::DistanceStatus::negative_infinity);
                    else if (d[source][v] == inf)
                        require(actual[v].status == a::DistanceStatus::unreachable);
                    else
                        require(actual[v].status == a::DistanceStatus::finite &&
                                actual[v].value == d[source][v]);
                }
            }
            auto solution = a::solve_difference_constraints(n, edges);
            bool negative = false;
            for (std::uint32_t k = 0; k < n; ++k)
                negative |= d[k][k] < 0;
            require(bool(solution) == !negative);
            if (solution)
                for (auto e : edges)
                    require((*solution)[e.to] <= (*solution)[e.from] + e.weight);
        }
    for (std::uint32_t n = 1; n < 100; ++n) {
        std::vector<std::uint32_t> parents(n);
        for (std::uint32_t i = 0; i < n; ++i)
            parents[i] = random() % (i + 1);
        a::LowestCommonAncestor index(parents);
        for (int q = 0; q < 100; ++q) {
            auto x = static_cast<std::uint32_t>(random() % n),
                 y = static_cast<std::uint32_t>(random() % n);
            std::vector<std::uint32_t> px{x}, py{y};
            while (parents[px.back()] != px.back())
                px.push_back(parents[px.back()]);
            while (parents[py.back()] != py.back())
                py.push_back(parents[py.back()]);
            std::optional<std::uint32_t> common;
            for (auto u : px)
                if (std::find(py.begin(), py.end(), u) != py.end()) {
                    common = u;
                    break;
                }
            require(index.query(x, y) == common);
            for (std::size_t k = 0; k <= px.size(); ++k)
                require(index.ancestor(x, static_cast<std::uint32_t>(k)) ==
                        (k < px.size() ? std::optional<std::uint32_t>{px[k]} : std::nullopt));
        }
    }
    std::vector<std::uint32_t> cycle{1, 0};
    throws<std::invalid_argument>([&] {
        a::LowestCommonAncestor bad(cycle);
    });
}
void sequences() {
    auto minimum = [](std::int64_t x, std::int64_t y) {
        return std::min(x, y);
    };
    for (int trial = 0; trial < 500; ++trial) {
        std::vector<std::int64_t> values(random() % 40);
        for (auto& x : values)
            x = static_cast<std::int64_t>(random() % 21) - 10;
        a::CoordinateCompression keys(values);
        require(std::is_sorted(keys.keys().begin(), keys.keys().end()));
        for (auto x : values)
            require(keys.value_at(*keys.rank(x)) == x);
        a::SparseTable<std::int64_t, decltype(minimum)> table(values, minimum);
        for (std::size_t l = 0; l < values.size(); ++l)
            for (auto r = l + 1; r <= values.size(); ++r)
                require(table.fold(l, r) ==
                        *std::min_element(values.begin() + l, values.begin() + r));
        for (bool weak : {false, true}) {
            std::vector<std::size_t> dp(values.size(), 1);
            std::size_t optimum = 0;
            for (std::size_t i = 0; i < values.size(); ++i) {
                for (std::size_t j = 0; j < i; ++j)
                    if (weak ? values[j] <= values[i] : values[j] < values[i])
                        dp[i] = std::max(dp[i], dp[j] + 1);
                optimum = std::max(optimum, dp[i]);
            }
            auto chosen = a::increasing_subsequence_indices(values, weak);
            require(chosen.size() == optimum &&
                    a::increasing_subsequence_length(values, weak) == optimum);
            for (std::size_t i = 1; i < chosen.size(); ++i)
                require(chosen[i - 1] < chosen[i] &&
                        (weak ? values[chosen[i - 1]] <= values[chosen[i]]
                              : values[chosen[i - 1]] < values[chosen[i]]));
        }
        std::string text(random() % 50, ' '), other(random() % 40, ' ');
        for (auto& c : text)
            c = static_cast<char>(random() % 5);
        for (auto& c : other)
            c = static_cast<char>(random() % 5);
        auto pal = a::palindrome_radii(text);
        std::size_t longest = 0, first = 0;
        for (std::size_t i = 0; i < text.size(); ++i) {
            std::size_t odd = 1, even = 0;
            while (odd <= i && odd < text.size() - i && text[i - odd] == text[i + odd])
                ++odd;
            while (even < i && even < text.size() - i && text[i - even - 1] == text[i + even])
                ++even;
            require(pal.odd[i] == odd && pal.even[i] == even);
        }
        for (std::size_t l = 0; l < text.size(); ++l)
            for (auto r = l + 1; r <= text.size(); ++r) {
                bool palindrome = true;
                for (auto i = l; i < r; ++i)
                    palindrome &= text[i] == text[r - 1 - (i - l)];
                if (palindrome && r - l > longest) {
                    longest = r - l;
                    first = l;
                }
            }
        require(pal.longest_last - pal.longest_first == longest && pal.longest_first == first);
        std::vector<std::vector<std::size_t>> full(text.size() + 1,
                                                   std::vector<std::size_t>(other.size() + 1));
        for (std::size_t i = 1; i <= text.size(); ++i)
            for (std::size_t j = 1; j <= other.size(); ++j)
                full[i][j] = text[i - 1] == other[j - 1] ? full[i - 1][j - 1] + 1
                                                         : std::max(full[i - 1][j], full[i][j - 1]);
        require(a::common_subsequence_length(text, other) == full.back().back());
    }
    for (int trial = 0; trial < 200; ++trial) {
        std::vector<a::Interval> intervals;
        for (int i = 0; i < 8; ++i) {
            auto l = static_cast<std::int64_t>(random() % 10),
                 r = static_cast<std::int64_t>(random() % 10);
            if (l > r)
                std::swap(l, r);
            intervals.push_back({l, r});
        }
        auto merged = a::merge_intervals(intervals);
        for (int point = 0; point < 10; ++point) {
            bool before = false, after = false;
            for (auto x : intervals)
                before |= x.first <= point && point < x.last;
            for (auto x : merged)
                after |= x.first <= point && point < x.last;
            require(before == after);
        }
        auto chosen = a::minimum_interval_cover(intervals, {0, 9});
        std::size_t best = 9;
        for (unsigned mask = 0; mask < 256; ++mask) {
            std::vector<a::Interval> subset;
            for (unsigned i = 0; i < 8; ++i)
                if (mask >> i & 1U)
                    subset.push_back(intervals[i]);
            auto united = a::merge_intervals(subset);
            if (united.size() == 1 && united[0].first <= 0 && united[0].last >= 9)
                best = std::min(best, subset.size());
        }
        require(bool(chosen) == (best < 9));
        if (chosen)
            require(chosen->size() == best);
    }
}
void numeric() {
    throws<std::invalid_argument>([] {
        a::Radix2Fft bad(3);
    });
    throws<std::invalid_argument>([] {
        a::Radix2Fft plan(2);
        std::vector<std::complex<double>> input{{1, 0},
                                                {std::numeric_limits<double>::infinity(), 0}};
        plan.transform(input);
    });
    throws<std::invalid_argument>([] {
        a::DenseLu bad(2, std::vector<double>{1, 2, 3});
    });
    throws<std::invalid_argument>([] {
        (void)a::analyze_absorbing_markov(2, 1, std::vector<double>{0.5, 0.4, 0, 1});
    });
    for (std::size_t n = 1; n <= 64; n *= 2) {
        a::Radix2Fft plan(n);
        std::vector<std::complex<double>> values(n);
        for (auto& x : values)
            x = {static_cast<double>(random() % 10), static_cast<double>(random() % 10)};
        auto original = values;
        plan.transform(values);
        for (std::size_t k = 0; k < n; ++k) {
            std::complex<long double> reference = 0;
            for (std::size_t j = 0; j < n; ++j)
                reference += std::complex<long double>(original[j]) *
                             std::polar(1.L, -2 * std::numbers::pi_v<long double> *
                                                 static_cast<long double>(j) * k / n);
            require(std::abs(values[k] - std::complex<double>(reference)) < 1e-9 * n);
        }
        plan.transform(values, true);
        for (std::size_t i = 0; i < n; ++i)
            require(std::abs(values[i] - original[i]) < 1e-10);
    }
    for (int trial = 0; trial < 200; ++trial) {
        std::vector<double> x(random() % 30), y(random() % 30);
        for (auto& z : x)
            z = (static_cast<double>(random() % 100) - 50) / 7;
        for (auto& z : y)
            z = (static_cast<double>(random() % 100) - 50) / 7;
        auto actual = a::convolve_real(x, y);
        if (x.empty() || y.empty()) {
            require(actual.empty());
            continue;
        }
        std::vector<long double> expected(x.size() + y.size() - 1);
        for (std::size_t i = 0; i < x.size(); ++i)
            for (std::size_t j = 0; j < y.size(); ++j)
                expected[i + j] += static_cast<long double>(x[i]) * y[j];
        for (std::size_t i = 0; i < actual.size(); ++i)
            require(close(actual[i], static_cast<double>(expected[i]), 1e-8));
    }
    for (std::size_t n = 1; n < 15; ++n)
        for (int trial = 0; trial < 20; ++trial) {
            std::vector<double> matrix(n * n), known(n), right(n);
            for (auto& x : matrix)
                x = (static_cast<double>(random() % 20) - 10) / 20;
            for (auto& x : known)
                x = (static_cast<double>(random() % 20) - 10) / 3;
            for (std::size_t i = 0; i < n; ++i) {
                matrix[i * n + i] += n;
                for (std::size_t j = 0; j < n; ++j)
                    right[i] += matrix[i * n + j] * known[j];
            }
            a::DenseLu factorization(n, matrix);
            auto solved = factorization.solve(right);
            for (std::size_t i = 0; i < n; ++i)
                require(close(solved[i], known[i]));
            require(std::isfinite(factorization.log_determinant().log_absolute));
        }
    a::DenseLu pivot(2, std::vector<double>{0, 2, 1, 3});
    auto solved = pivot.solve(std::vector<double>{4, 7});
    require(close(solved[0], 1) && close(solved[1], 2));
    require(pivot.log_determinant().sign == -1);
    throws<std::domain_error>([] {
        a::DenseLu singular(2, std::vector<double>{1, 2, 2, 4});
    });
    auto chain =
        a::analyze_absorbing_markov(3, 2, std::vector<double>{0.5, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1});
    require(close(chain.expected_steps[0], 4) && close(chain.expected_steps[1], 2));
    for (auto p : chain.absorption_probabilities)
        require(close(p, 1));
    throws<std::domain_error>([] {
        (void)a::analyze_absorbing_markov(2, 1, std::vector<double>{1, 0, 0, 1});
    });
    for (int trial = 0; trial < 500; ++trial) {
        std::vector<a::BoundedKnapsackItem> items;
        std::vector<a::KnapsackItem> plain;
        for (int i = 0; i < 5; ++i) {
            items.push_back(
                {random() % 8, static_cast<std::int64_t>(random() % 20) - 5, random() % 5});
            plain.push_back({items.back().cost, items.back().value});
        }
        std::function<std::int64_t(std::size_t, std::size_t)> brute =
            [&](auto i, auto budget) -> std::int64_t {
            if (i == items.size())
                return 0;
            auto best = brute(i + 1, budget);
            for (std::size_t count = 1; count <= items[i].count && count * items[i].cost <= budget;
                 ++count)
                best = std::max(best, static_cast<std::int64_t>(count) * items[i].value +
                                          brute(i + 1, budget - count * items[i].cost));
            return best;
        };
        require(a::knapsack_bounded(items, 20) == brute(0, 20));
        for (auto& item : items)
            item.count = 1;
        require(a::knapsack_01(plain, 20) == brute(0, 20));
        for (std::size_t i = 0; i < items.size(); ++i) {
            items[i].cost = plain[i].cost = std::max(std::size_t{1}, items[i].cost);
            items[i].count = 20 / items[i].cost;
        }
        require(a::knapsack_unbounded(plain, 20) == brute(0, 20));
    }
    const auto maximum = std::numeric_limits<std::int64_t>::max();
    require(a::knapsack_bounded(std::vector<a::BoundedKnapsackItem>{{1, maximum, 1}}, 10) ==
            maximum);
    throws<std::overflow_error>([&] {
        (void)a::knapsack_01(std::vector<a::KnapsackItem>{{0, maximum}, {0, 1}}, 0);
    });
}
void algebra() {
    throws<std::invalid_argument>([] {
        a::Modulus64 bad(1);
    });
    throws<std::invalid_argument>([] {
        (void)a::chinese_remainder(std::vector<a::Congruence>{{0, 0}});
    });
    throws<std::overflow_error>([] {
        (void)a::chinese_remainder(
            std::vector<a::Congruence>{{0, std::numeric_limits<std::uint64_t>::max()}, {0, 2}});
    });
    for (std::uint64_t modulus = 2; modulus < 100; ++modulus) {
        a::Modulus64 m(modulus);
        for (std::uint64_t x = 0; x < 150; ++x) {
            auto inverse = m.inverse(x);
            require(bool(inverse) == (std::gcd(x, modulus) == 1));
            if (inverse)
                require(m.multiply(x, *inverse) == 1);
            for (std::uint64_t y = 0; y < 100; ++y)
                require(m.multiply(x, y) == x * y % modulus);
        }
    }
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    a::Modulus64 full(maximum);
    require(full.multiply(maximum - 1, maximum - 1) == 1 &&
            full.add(maximum - 1, maximum - 1) == maximum - 2);
    require(full.inverse(maximum - 1) == maximum - 1);
    for (std::uint64_t m = 1; m < 20; ++m)
        for (std::uint64_t n = 1; n < 20; ++n) {
            std::vector<a::Congruence> equations{{random() % m, m}, {random() % n, n}};
            auto result = a::chinese_remainder(equations);
            std::optional<std::uint64_t> smallest;
            for (std::uint64_t x = 0; x < std::lcm(m, n); ++x)
                if (x % m == equations[0].residue && x % n == equations[1].residue) {
                    smallest = x;
                    break;
                }
            require(bool(result) == bool(smallest));
            if (result)
                require(result->residue == *smallest && result->modulus == std::lcm(m, n));
        }
    for (std::uint32_t n : {0U, 1U, 2U, 3U, 1000U}) {
        a::PrimeSieve sieve(n);
        auto primes = sieve.primes();
        std::size_t count = 0;
        for (std::uint32_t x = 0; x <= n; ++x) {
            bool prime = x >= 2;
            for (std::uint32_t d = 2; d <= x / d; ++d)
                if (x % d == 0)
                    prime = false;
            require(sieve.is_prime(x) == prime);
            count += prime;
        }
        require(count == primes.size());
    }
}
void geometry() {
    for (int trial = 0; trial < 500; ++trial) {
        std::vector<g::IntegerPoint> p(random() % 10), q(random() % 10);
        for (auto& x : p)
            x = {static_cast<std::int64_t>(random() % 21) - 10,
                 static_cast<std::int64_t>(random() % 21) - 10};
        for (auto& x : q)
            x = {static_cast<std::int64_t>(random() % 21) - 10,
                 static_cast<std::int64_t>(random() % 21) - 10};
        auto hp = g::convex_hull<std::int64_t>(p), hq = g::convex_hull<std::int64_t>(q);
        for (bool difference : {false, true}) {
            std::vector<g::IntegerPoint> sums;
            for (auto x : p)
                for (auto y : q)
                    sums.push_back(
                        {x.x + (difference ? -y.x : y.x), x.y + (difference ? -y.y : y.y)});
            auto reference = g::convex_hull<std::int64_t>(sums),
                 actual = difference ? g::minkowski_difference(hp, hq) : g::minkowski_sum(hp, hq);
            require(std::ranges::equal(reference.vertices(), actual.vertices()));
            if (difference) {
                g::TranslatedConvexOverlap overlap(hp, hq);
                for (int i = 0; i < 10; ++i) {
                    g::IntegerPoint t{static_cast<std::int64_t>(random() % 41) - 20, 0};
                    require(overlap.intersects(t) ==
                            (g::locate(reference, t) != g::Containment::outside));
                }
            }
        }
    }
    for (int trial = 0; trial < 300; ++trial) {
        std::vector<g::Point> points(1 + random() % 9);
        for (auto& p : points)
            p = {static_cast<double>(random() % 21) - 10, static_cast<double>(random() % 21) - 10};
        double best = std::numeric_limits<double>::infinity();
        auto consider = [&](g::Circle c) {
            for (auto p : points)
                if (g::distance(c.center(), p) > c.radius() + 1e-8)
                    return;
            best = std::min(best, c.radius());
        };
        for (auto p : points)
            consider(g::Circle(p, 0));
        for (std::size_t i = 0; i < points.size(); ++i)
            for (std::size_t j = i + 1; j < points.size(); ++j) {
                auto a = points[i], b = points[j];
                consider(g::Circle({(a.x + b.x) / 2, (a.y + b.y) / 2}, g::distance(a, b) / 2));
                for (std::size_t k = j + 1; k < points.size(); ++k)
                    if (auto c = g::circumcircle(a, b, points[k], g::Tolerance(0, 0)))
                        consider(*c);
            }
        for (std::uint64_t seed = 0; seed < 5; ++seed) {
            auto actual = g::minimum_enclosing_circle(points, seed);
            require(bool(actual) && close(actual->radius(), best));
            for (auto p : points)
                require(g::distance(actual->center(), p) <= actual->radius() * (1 + 1e-14) + 1e-10);
        }
    }
    require(!g::minimum_enclosing_circle({}));
    for (double scale : {1e-100, 1.0, 1e40}) {
        std::vector<g::Point> points{{0, 0}, {2 * scale, 0}, {scale, 1e-12 * scale}};
        auto result = g::minimum_enclosing_circle(points);
        require(std::abs(result->radius() / scale - 1) < 1e-9);
    }
}
} // namespace
int expansion_odr_probe();
int main() {
    graph_components_sat();
    flow_and_matching();
    signed_paths_and_forest();
    sequences();
    numeric();
    algebra();
    geometry();
    require(expansion_odr_probe() == 1);
    std::cout
        << "Repository expansion: graph, numeric, optimization, sequence and geometry oracles passed\n";
}
