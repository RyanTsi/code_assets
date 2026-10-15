#include <algorithm_assets/algebra/xor_basis.hpp>
#include <algorithm_assets/graph/kruskal.hpp>
#include <algorithm_assets/graph/topological_sort.hpp>
#include <algorithm_assets/recipes/inversions_fenwick.hpp>
#include <algorithm_assets/strings/kmp.hpp>

#include <algorithm>
#include <bit>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <source_location>
#include <string>

namespace a = algorithm_assets;
namespace {
void require(bool value, std::source_location where = std::source_location::current()) {
  if (!value) throw std::runtime_error(std::string(where.file_name()) + ":" + std::to_string(where.line()) + ": check failed");
}
template<class Error, class F> void throws(F function) {
  bool caught = false;
  try { function(); } catch (const Error&) { caught = true; }
  require(caught);
}
void test_numeric_and_fenwick() {
  const auto maximum = std::numeric_limits<std::int64_t>::max(), minimum = std::numeric_limits<std::int64_t>::min();
  a::WideInt64Sum sum;
  sum.add(maximum); sum.add(maximum); sum.subtract(maximum);
  require(sum.value() == maximum);
  sum.clear(); sum.subtract(minimum); sum.add(minimum); require(sum.value() == 0);
  sum.add(minimum); sum.add(minimum); sum.subtract(minimum); require(sum.value() == minimum);
  sum.add(-1); throws<std::overflow_error>([&] { (void)sum.value(); });
  a::FenwickSum empty(0);
  require(empty.prefix_sum(0) == 0 && empty.range_sum(0, 0) == 0);
  throws<std::out_of_range>([&] { empty.add(0, 0); });
  // All stored blocks fit although an intermediate build/prefix sum would not.
  const std::vector<std::int64_t> cancellation{-1, maximum, -maximum, maximum};
  a::FenwickSum exact(cancellation);
  require(exact.range_sum(0, 4) == maximum - 1);
  const std::vector<std::int64_t> outside_prefix{maximum, 0, 1};
  a::FenwickSum prefix(outside_prefix);
  throws<std::overflow_error>([&] { (void)prefix.prefix_sum(3); });
  require(prefix.range_sum(2, 3) == 1);
  a::FenwickSum atomic(std::vector<std::int64_t>{0, maximum});
  throws<std::overflow_error>([&] { atomic.add(0, 1); });
  require(atomic.range_sum(0, 1) == 0 && atomic.range_sum(0, 2) == maximum);
  std::mt19937 random(724);
  for (std::size_t n = 0; n < 60; ++n) {
    std::vector<std::int64_t> values(n);
    for (auto& value : values) value = static_cast<int>(random() % 201) - 100;
    a::FenwickSum tree(values);
    for (int operation = 0; operation < 200; ++operation) {
      if (n && random() % 2) {
        const auto position = random() % n;
        const auto delta = static_cast<int>(random() % 201) - 100;
        values[position] += delta; tree.add(position, delta);
      } else {
        auto first = random() % (n + 1), last = random() % (n + 1);
        if (first > last) std::swap(first, last);
        std::int64_t total = 0;
        for (auto i = first; i < last; ++i) total += values[i];
        require(tree.range_sum(first, last) == total);
      }
    }
  }
}
void test_kmp() {
  std::mt19937 random(392);
  for (int iteration = 0; iteration < 500; ++iteration) {
    std::string text(random() % 70, ' '), pattern(random() % 15, ' ');
    for (auto& value : text) value = static_cast<char>(random() % 4); // includes NUL
    for (auto& value : pattern) value = static_cast<char>(random() % 4);
    std::vector<std::size_t> matches;
    for (std::size_t i = 0; i <= text.size(); ++i)
      if (std::string_view(text).substr(i, pattern.size()) == pattern) matches.push_back(i);
    a::KmpPattern compiled(pattern);
    require(compiled.find_all(text) == matches);
    require(compiled.contains(text) == !matches.empty());
    require(compiled.find_first(text) == (matches.empty() ? std::optional<std::size_t>{} : matches.front()));
    const auto borders = compiled.prefix_function();
    for (std::size_t length = 1; length <= pattern.size(); ++length) {
      std::size_t longest = 0;
      for (std::size_t k = 1; k < length; ++k)
        if (pattern.substr(0, k) == pattern.substr(length - k, k)) longest = k;
      require(borders[length - 1] == longest);
    }
  }
  require(a::KmpPattern("AAA").find_all("AAAAA") == std::vector<std::size_t>({0, 1, 2}));
  require(a::KmpPattern("").find_all("") == std::vector<std::size_t>({0}));
}
void test_xor_basis() {
  std::mt19937_64 random(425);
  for (int iteration = 0; iteration < 300; ++iteration) {
    a::XorBasis64 basis, left, right;
    std::set<std::uint64_t> values{0};
    for (int i = 0; i < 12; ++i) {
      const auto value = random() % 256;
      const auto before = values;
      for (const auto previous : before) values.insert(previous ^ value);
      basis.insert(value); (i < 6 ? left : right).insert(value);
    }
    left.merge(right);
    require((std::uint64_t{1} << basis.rank()) == values.size());
    require(basis.maximize() == *values.rbegin());
    require(left.maximize() == basis.maximize());
    std::size_t index = 0;
    for (const auto value : values) {
      require(basis.contains(value));
      require(basis.kth_span_value(index++) == value);
    }
    require(!basis.kth_span_value(values.size()));
    for (std::uint64_t value = 0; value < 256; ++value) require(basis.contains(value) == values.contains(value));
    auto expected = std::next(values.begin());
    require(basis.minimum_nonzero() == (expected == values.end() ? std::optional<std::uint64_t>{} : *expected));
  }
  a::XorBasis64 full;
  for (int i = 0; i < 64; ++i) require(full.insert(std::uint64_t{1} << i));
  require(full.rank() == 64 && full.contains(std::uint64_t{1} << 63));
  require(full.kth_span_value(std::numeric_limits<std::uint64_t>::max()) == std::numeric_limits<std::uint64_t>::max());
  const auto rank = full.rank(); full.merge(full); require(full.rank() == rank);
}
void test_mst_and_topology() {
  std::mt19937 random(714);
  for (std::size_t n = 0; n <= 6; ++n) {
    for (int iteration = 0; iteration < 80; ++iteration) {
      std::vector<a::UndirectedWeightedEdge> edges;
      if (n) for (std::size_t i = 0; i < random() % 10; ++i)
        edges.push_back({static_cast<std::uint32_t>(random() % n), static_cast<std::uint32_t>(random() % n), static_cast<int>(random() % 21) - 10});
      const auto result = a::kruskal_minimum_spanning_forest(n, edges);
      a::DisjointSet connectivity(n);
      for (const auto edge : edges) connectivity.merge(edge.first, edge.second);
      const auto components = connectivity.component_count();
      std::int64_t best = std::numeric_limits<std::int64_t>::max();
      for (std::size_t mask = 0; mask < (std::size_t{1} << edges.size()); ++mask) {
        if (static_cast<std::size_t>(std::popcount(mask)) != n - components) continue;
        a::DisjointSet candidate(n); std::int64_t cost = 0; bool valid = true;
        for (std::size_t i = 0; i < edges.size(); ++i) if (mask & (std::size_t{1} << i)) {
          valid = candidate.merge(edges[i].first, edges[i].second) && valid; cost += edges[i].weight;
        }
        if (valid && candidate.component_count() == components) best = std::min(best, cost);
      }
      require(result.component_count == components && result.total_weight == best);
      require(result.edge_indices.size() == n - components);
      require(result.is_spanning_tree == (n != 0 && components == 1));
    }
  }
  const auto minimum = std::numeric_limits<std::int64_t>::min(), maximum = std::numeric_limits<std::int64_t>::max();
  const std::vector<a::UndirectedWeightedEdge> cancelling{{0, 1, minimum}, {1, 2, minimum}, {2, 3, maximum}};
  throws<std::overflow_error>([&] { (void)a::kruskal_minimum_spanning_forest(4, cancelling); });
  const std::vector<a::UndirectedWeightedEdge> fitting{{0, 1, -maximum}, {1, 2, -maximum}, {2, 3, maximum}};
  require(a::kruskal_minimum_spanning_forest(4, fitting).total_weight == -maximum);
  throws<std::out_of_range>([] { (void)a::kruskal_minimum_spanning_forest(1, std::vector<a::UndirectedWeightedEdge>{{0, 1, 2}}); });
  for (std::size_t n = 0; n < 20; ++n) for (int iteration = 0; iteration < 30; ++iteration) {
    std::vector<a::CsrGraph::Edge> edges;
    std::vector<std::vector<bool>> reachable(n, std::vector<bool>(n));
    for (std::size_t u = 0; u < n; ++u) for (std::size_t v = 0; v < n; ++v)
      if (random() % 20 == 0) { edges.push_back({static_cast<std::uint32_t>(u), static_cast<std::uint32_t>(v), 0}); reachable[u][v] = true; }
    for (std::size_t k = 0; k < n; ++k) for (std::size_t i = 0; i < n; ++i) for (std::size_t j = 0; j < n; ++j)
      reachable[i][j] = reachable[i][j] || (reachable[i][k] && reachable[k][j]);
    bool cycle = false; for (std::size_t i = 0; i < n; ++i) cycle = cycle || reachable[i][i];
    const a::CsrGraph graph(n, edges);
    const auto order = a::topological_sort(graph);
    require(order.acyclic == !cycle);
    require(a::topological_sort(graph).vertices == order.vertices); // no destructive indegrees
    if (order.acyclic) {
      std::vector<std::size_t> position(n);
      for (std::size_t i = 0; i < n; ++i) position[order.vertices[i]] = i;
      for (const auto edge : edges) require(position[edge.from] < position[edge.to]);
    }
  }
}
void test_inversions() {
  std::mt19937 random(279);
  a::InversionCounter workspace;
  for (int iteration = 0; iteration < 500; ++iteration) {
    std::vector<std::int64_t> values(random() % 100);
    for (auto& value : values) value = static_cast<int>(random() % 21) - 10;
    const auto original = values;
    std::uint64_t expected = 0;
    for (std::size_t i = 0; i < values.size(); ++i) for (auto j = i + 1; j < values.size(); ++j) expected += values[i] > values[j];
    require(workspace.count(values) == expected && values == original);
    require(a::count_inversions_fenwick(values) == expected && values == original);
    require(workspace.count_inplace(values) == expected && std::is_sorted(values.begin(), values.end()));
  }
  require(a::count_inversions(std::vector<std::int64_t>{std::numeric_limits<std::int64_t>::max(), std::numeric_limits<std::int64_t>::min()}) == 1);
  workspace.release(); require(workspace.storage_bytes() == 0);
}
} // namespace

bool repository_assets_odr_probe();
int main() {
  try {
    test_numeric_and_fenwick(); test_kmp(); test_xor_basis(); test_mst_and_topology(); test_inversions();
    require(repository_assets_odr_probe());
    std::cout << "Repository assets: randomized oracles, boundaries, numeric safety and ODR passed\n";
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
