#include <algorithm_assets/graph/dinic.hpp>
#include <iostream>
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n, m;
    std::uint32_t source, sink;
    std::cin >> n >> m >> source >> sink;
    algorithm_assets::Dinic network(n);
    while (m--) {
        std::uint32_t u, v;
        std::uint64_t capacity;
        std::cin >> u >> v >> capacity;
        network.add_edge(u - 1, v - 1, capacity);
    }
    std::cout << network.max_flow(source - 1, sink - 1).added << '\n';
}
