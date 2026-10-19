#include <algorithm_assets/algorithm_assets.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>

int main() {
    using namespace algorithm_assets;

    DisjointSet components(4);
    components.merge(0, 1);
    std::cout << "DSU: " << components.same(0, 1) << '\n';

    RollbackDisjointSet reversible(4);
    const auto checkpoint = reversible.checkpoint();
    reversible.merge(0, 1);
    reversible.rollback(checkpoint);
    std::cout << "After rollback: " << reversible.same(0, 1) << '\n';

    const std::array<std::int64_t, 5> values{1, 2, 3, 4, 5};
    SegmentTree<std::int64_t, std::plus<std::int64_t>> sums(values, 0, {});
    sums.set(2, 10);
    std::cout << "Range sum [1, 4): " << sums.fold(1, 4) << '\n';

    const std::array edges{CsrGraph::Edge{0, 1, 3}, CsrGraph::Edge{1, 2, 4},
                           CsrGraph::Edge{0, 2, 20}};
    const CsrGraph graph(4, edges);
    const auto paths = dijkstra(graph, 0);
    std::cout << "Shortest distance to 2: " << *paths.distance_to(2) << '\n';

    const std::array doubles{1e16, 1.0, -1e16};
    std::cout << "Compensated sum: " << compensated_sum(doubles) << '\n';

    using Kind = ConnectivityEventKind;
    const std::array events{
        ConnectivityEvent{Kind::add, 0, 1}, ConnectivityEvent{Kind::query, 0, 1},
        ConnectivityEvent{Kind::remove, 0, 1}, ConnectivityEvent{Kind::query, 0, 1}};
    std::cout << "Offline connectivity:";
    for (const auto connected : offline_connectivity(2, events)) {
        std::cout << ' ' << connected;
    }
    std::cout << '\n';
}
