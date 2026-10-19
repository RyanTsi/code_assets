#include <algorithm_assets/graph/two_sat.hpp>
#include <algorithm_assets/graph/dinic.hpp>
#include <algorithm_assets/graph/bipartite_matching.hpp>
#include <algorithm_assets/graph/bellman_ford.hpp>
#include <algorithm_assets/graph/lowest_common_ancestor.hpp>
#include <algorithm_assets/sequence/subsequences.hpp>
#include <algorithm_assets/sequence/intervals.hpp>
#include <algorithm_assets/sequence/coordinate_compression.hpp>
#include <algorithm_assets/range/sparse_table.hpp>
#include <algorithm_assets/strings/manacher.hpp>
#include <algorithm_assets/optimization/knapsack.hpp>
#include <algorithm_assets/recipes/absorbing_markov.hpp>
#include <algorithm_assets/numeric/fft.hpp>
#include <algorithm_assets/algebra/modular.hpp>
#include <algorithm_assets/algebra/prime_sieve.hpp>
#include <algorithm_assets/geometry/minkowski.hpp>
#include <algorithm_assets/geometry/minimum_enclosing_circle.hpp>
#include <iostream>
int main() {
    namespace a = algorithm_assets;
    a::TwoSat options(2);
    options.add_clause({0, true}, {1, true});
    options.force({0, false});
    std::cout << "feasible Boolean configuration: " << options.solve().satisfiable << '\n';
    a::Dinic network(3);
    network.add_edge(0, 1, 7);
    network.add_edge(1, 2, 5);
    auto flow = network.max_flow(0, 2);
    std::cout << "flow: " << flow.added << ", complete: " << flow.maximal << '\n';
    std::vector<a::UnweightedCsrGraph::Edge> edges{{0, 0}, {0, 1}, {1, 1}};
    a::UnweightedCsrGraph graph(2, edges);
    std::cout << "matching: " << a::hopcroft_karp(2, 2, graph).size << '\n';
    std::vector<std::int64_t> values{3, 1, 2, 2, 4};
    std::cout << "strict LIS: " << a::increasing_subsequence_length(values) << '\n';
    auto palindrome = a::palindrome_radii("abacaba");
    std::cout << "palindrome length: " << palindrome.longest_last - palindrome.longest_first
              << '\n';
    auto minimum = [](auto x, auto y) {
        return std::min(x, y);
    };
    a::SparseTable<std::int64_t, decltype(minimum)> table(values, minimum);
    std::cout << "range minimum: " << table.fold(1, 4) << '\n';
    std::vector<double> x{1, 2}, y{3, 4};
    auto convolution = a::convolve_real(x, y);
    std::cout << "convolution: ";
    for (auto z : convolution)
        std::cout << z << ' ';
    std::cout << '\n';
    auto chain = a::analyze_absorbing_markov(2, 1, std::vector<double>{0.5, 0.5, 0, 1});
    std::cout << "expected absorption steps: " << chain.expected_steps[0] << '\n';
    namespace g = a::geometry;
    std::vector<g::Point> points{{0, 0}, {4, 0}, {2, 1}};
    std::cout << "enclosing radius: " << g::minimum_enclosing_circle(points)->radius() << '\n';
    auto components = a::strong_components(graph);
    auto condensed = a::condense_graph(graph);
    std::cout << "components: " << components.count
              << ", condensed arcs: " << condensed.dag.edge_count() << '\n';
    std::vector<std::uint32_t> parents{0, 0, 1};
    a::LowestCommonAncestor ancestry(parents);
    std::cout << "common ancestor: " << *ancestry.query(1, 2) << '\n';
    std::vector<a::SignedDirectedEdge> signed_edges{{0, 1, -2}, {1, 2, 3}};
    auto distances = a::bellman_ford(3, signed_edges, 0);
    std::cout << "signed path: " << distances[2].value << ", feasible differences: "
              << bool(a::solve_difference_constraints(3, signed_edges)) << '\n';
    a::CoordinateCompression compressed(values);
    std::cout << "rank of 2: " << *compressed.rank(2) << '\n';
    std::cout << "LCS length: " << a::common_subsequence_length("abc", "ac") << '\n';
    std::vector<a::Interval> intervals{{0, 2}, {1, 3}};
    std::cout << "merged intervals: " << a::merge_intervals(intervals).size()
              << ", minimum cover: " << a::minimum_interval_cover(intervals, {0, 3})->size()
              << '\n';
    a::DenseLu factorization(2, std::vector<double>{2, 1, 1, 2});
    std::cout << "LU solution: " << factorization.solve(std::vector<double>{3, 3})[0] << '\n';
    a::Radix2Fft plan(2);
    std::vector<std::complex<double>> signal{{1, 0}, {2, 0}};
    plan.transform(signal);
    plan.transform(signal, true);
    std::vector<a::KnapsackItem> items{{2, 3}, {3, 4}};
    std::vector<a::BoundedKnapsackItem> bounded{{2, 3, 2}, {3, 4, 1}};
    std::cout << "knapsacks: " << a::knapsack_01(items, 5) << ' ' << a::knapsack_unbounded(items, 5)
              << ' ' << a::knapsack_bounded(bounded, 5) << '\n';
    a::Modulus64 arithmetic(7);
    auto congruence = a::chinese_remainder(std::vector<a::Congruence>{{1, 2}, {2, 3}});
    std::cout << "mod inverse: " << *arithmetic.inverse(3) << ", CRT: " << congruence->residue
              << ", primes: " << a::PrimeSieve(20).primes().size() << '\n';
    std::vector<g::IntegerPoint> vertices{{0, 0}, {1, 0}, {0, 1}};
    auto hull = g::convex_hull<std::int64_t>(vertices);
    auto sum = g::minkowski_sum(hull, hull);
    auto difference = g::minkowski_difference(hull, hull);
    g::TranslatedConvexOverlap collision(hull, hull);
    std::cout << "sum/difference vertices: " << sum.size() << '/' << difference.size()
              << ", origin contact: " << collision.intersects({0, 0}) << '\n';
}
