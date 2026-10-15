#include <algorithm_assets/algorithm_assets.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, std::source_location location = std::source_location::current()) {
  if (!condition) {
    throw std::runtime_error(std::string(location.file_name()) + ":" +
                             std::to_string(location.line()) + ": check failed");
  }
}

template <class Exception, class Function> void expect_throw(Function function) {
  bool thrown = false;
  try {
    function();
  } catch (const Exception&) {
    thrown = true;
  }
  require(thrown);
}

void relabel(std::vector<int>& labels, std::size_t first, std::size_t second) {
  const auto old = labels[second];
  const auto replacement = labels[first];
  for (auto& label : labels) {
    if (label == old) {
      label = replacement;
    }
  }
}

void test_disjoint_sets() {
  using namespace algorithm_assets;
  DisjointSet empty(0);
  require(empty.component_count() == 0);
  expect_throw<std::out_of_range>([&] { empty.leader(0); });
  expect_throw<std::length_error>([] {
    DisjointSet too_large(static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()) + 1);
  });

  constexpr std::size_t size = 32;
  DisjointSet plain(size);
  std::vector<int> labels(size);
  std::iota(labels.begin(), labels.end(), 0);
  std::mt19937 random(741);
  for (int iteration = 0; iteration < 2000; ++iteration) {
    const auto first = random() % size;
    const auto second = random() % size;
    const bool changed = labels[first] != labels[second];
    require(plain.merge(first, second) == changed);
    relabel(labels, first, second);
    for (std::size_t vertex = 0; vertex < size; ++vertex) {
      require(plain.same(first, vertex) == (labels[first] == labels[vertex]));
      require(plain.component_size(vertex) ==
              static_cast<std::size_t>(std::count(labels.begin(), labels.end(), labels[vertex])));
    }
    auto unique = labels;
    std::sort(unique.begin(), unique.end());
    unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
    require(plain.component_count() == unique.size());
  }
  expect_throw<std::out_of_range>([&] { plain.merge(size, 0); });
  require(plain.component_count() == 1);

  RollbackDisjointSet reversible(size);
  std::iota(labels.begin(), labels.end(), 0);
  std::vector<std::vector<int>> states{labels};
  require(!reversible.undo_last_merge());
  for (int iteration = 0; iteration < 3000; ++iteration) {
    const auto first = random() % size;
    const auto second = random() % size;
    if (random() % 3 == 0) {
      const auto checkpoint = random() % states.size();
      reversible.rollback(checkpoint);
      states.resize(checkpoint + 1);
    } else if (random() % 4 == 0) {
      require(reversible.undo_last_merge() == (states.size() > 1));
      if (states.size() > 1) {
        states.pop_back();
      }
    } else {
      const bool changed = states.back()[first] != states.back()[second];
      require(reversible.merge(first, second) == changed);
      if (changed) {
        auto next = states.back();
        relabel(next, first, second);
        states.push_back(std::move(next));
      }
    }
    require(reversible.checkpoint() == states.size() - 1);
    const auto& current = states.back();
    for (std::size_t vertex = 0; vertex < size; ++vertex) {
      require(reversible.same(first, vertex) == (current[first] == current[vertex]));
      require(reversible.component_size(vertex) ==
              static_cast<std::size_t>(std::count(current.begin(), current.end(), current[vertex])));
    }
  }
  expect_throw<std::out_of_range>([&] { reversible.rollback(reversible.checkpoint() + 1); });
  reversible.rollback(0);
  require(reversible.component_count() == size);
}

void test_segment_tree() {
  using namespace algorithm_assets;
  std::mt19937 random(913);
  for (std::size_t size = 0; size <= 33; ++size) {
    std::vector<std::int64_t> values(size);
    SegmentTree<std::int64_t, std::plus<std::int64_t>> tree(values, 0, {});
    for (int iteration = 0; iteration < 200; ++iteration) {
      if (size != 0) {
        const auto index = random() % size;
        values[index] = static_cast<std::int64_t>(random() % 201) - 100;
        tree.set(index, values[index]);
        require(tree.get(index) == values[index]);
      }
      auto first = random() % (size + 1);
      auto last = random() % (size + 1);
      if (first > last) {
        std::swap(first, last);
      }
      require(tree.fold(first, last) ==
              std::accumulate(values.begin() + first, values.begin() + last, std::int64_t{0}));
      require(tree.all_fold() == std::accumulate(values.begin(), values.end(), std::int64_t{0}));
    }
    expect_throw<std::out_of_range>([&] { tree.set(size, 0); });
    expect_throw<std::out_of_range>([&] { (void)tree.fold(1, 0); });
    expect_throw<std::out_of_range>([&] { (void)tree.fold(0, size + 1); });
  }
  // Order matters, and a non-power-of-two 2n root is not the logical whole range.
  const std::array<std::string, 5> values{"a", "b", "c", "d", "e"};
  SegmentTree<std::string, std::plus<std::string>> strings(values, "", {});
  require(strings.all_fold() == "abcde");
  for (std::size_t first = 0; first <= values.size(); ++first) {
    for (auto last = first; last <= values.size(); ++last) {
      std::string expected;
      for (auto index = first; index < last; ++index) {
        expected += values[index];
      }
      require(strings.fold(first, last) == expected);
    }
  }
  strings.set(2, "XYZ");
  require(strings.fold(1, 4) == "bXYZd");
  require(strings.all_fold() == "abXYZde");
}

void test_dijkstra() {
  using namespace algorithm_assets;
  constexpr auto infinity = ShortestPaths::infinity;
  std::mt19937 random(667);
  for (std::uint32_t size = 1; size <= 18; ++size) {
    for (int trial = 0; trial < 8; ++trial) {
      std::vector<CsrGraph::Edge> edges;
      std::vector<std::vector<std::uint64_t>> distances(
          size, std::vector<std::uint64_t>(size, infinity));
      for (std::uint32_t vertex = 0; vertex < size; ++vertex) {
        distances[vertex][vertex] = 0;
      }
      for (std::uint32_t index = 0; index < 3 * size; ++index) {
        const auto first = static_cast<std::uint32_t>(random() % size);
        const auto second = static_cast<std::uint32_t>(random() % size);
        const auto weight = static_cast<std::uint64_t>(random() % 30);
        edges.push_back({first, second, static_cast<std::int64_t>(weight)});
        distances[first][second] = std::min(distances[first][second], weight);
      }
      const auto arc_weights = distances;
      for (std::uint32_t middle = 0; middle < size; ++middle) {
        for (std::uint32_t first = 0; first < size; ++first) {
          for (std::uint32_t second = 0; second < size; ++second) {
            if (distances[first][middle] != infinity && distances[middle][second] != infinity) {
              distances[first][second] =
                  std::min(distances[first][second], distances[first][middle] + distances[middle][second]);
            }
          }
        }
      }
      const CsrGraph graph(size, edges);
      require(graph.edge_count() == edges.size());
      for (std::uint32_t source = 0; source < size; ++source) {
        const auto result = dijkstra(graph, source);
        for (std::uint32_t target = 0; target < size; ++target) {
          const auto distance = result.distance_to(target);
          require(distance.has_value() == (distances[source][target] != infinity));
          const auto path = result.path_to(target);
          if (!distance) {
            require(path.empty());
            continue;
          }
          require(*distance == distances[source][target]);
          require(path.front() == source && path.back() == target);
          require(path.size() <= size);
          std::uint64_t path_length = 0;
          for (std::size_t index = 1; index < path.size(); ++index) {
            require(arc_weights[path[index - 1]][path[index]] != infinity);
            path_length += arc_weights[path[index - 1]][path[index]];
          }
          require(path_length == *distance);
        }
      }
    }
  }
  const CsrGraph empty(0, {});
  expect_throw<std::out_of_range>([&] { dijkstra(empty, 0); });
  const std::array negative{CsrGraph::Edge{0, 1, -1}};
  expect_throw<std::invalid_argument>([&] { CsrGraph graph(2, negative); });
  const std::array invalid{CsrGraph::Edge{0, 2, 1}};
  expect_throw<std::out_of_range>([&] { CsrGraph graph(2, invalid); });
  const auto large = std::numeric_limits<std::int64_t>::max();
  const std::array detour{CsrGraph::Edge{0, 1, large}, CsrGraph::Edge{1, 2, large},
                          CsrGraph::Edge{2, 0, large}};
  const auto result = dijkstra(CsrGraph(4, detour), 0);
  require(result.distance_to(2) == infinity - 1);
  require(!result.distance_to(3));
  const std::array overflow{CsrGraph::Edge{0, 1, large}, CsrGraph::Edge{1, 2, large},
                            CsrGraph::Edge{2, 3, 1}};
  expect_throw<std::overflow_error>([&] { dijkstra(CsrGraph(4, overflow), 0); });
}

void test_compensated_sum() {
  using namespace algorithm_assets;
  const std::array values{1e16, 1.0, -1e16};
  require(compensated_sum(values) == 1.0);
  require(std::accumulate(values.begin(), values.end(), 0.0) == 0.0);
  const std::array negative{-1e16, -1.0, 1e16};
  require(compensated_sum(negative) == -1.0);
  require(compensated_sum({}) == 0.0);
  CompensatedSum sum;
  sum.add(1.0);
  expect_throw<std::invalid_argument>([&] { sum.add(std::numeric_limits<double>::infinity()); });
  expect_throw<std::invalid_argument>([&] { sum.add(std::numeric_limits<double>::quiet_NaN()); });
  require(sum.value() == 1.0);
  sum.reset();
  sum.add(std::numeric_limits<double>::max());
  expect_throw<std::overflow_error>([&] { sum.add(std::numeric_limits<double>::max()); });
  require(sum.value() == std::numeric_limits<double>::max());
  sum.reset();
  std::mt19937 random(805);
  std::int64_t exact = 0;
  for (int index = 0; index < 10000; ++index) {
    const auto value = static_cast<std::int64_t>(random() % 2000001) - 1000000;
    exact += value;
    sum.add(static_cast<double>(value));
  }
  require(sum.value() == static_cast<double>(exact));
}

void test_offline_connectivity() {
  using namespace algorithm_assets;
  using Kind = ConnectivityEventKind;
  using Edge = std::pair<std::uint32_t, std::uint32_t>;
  require(offline_connectivity(0, {}).empty());
  const std::array remove{ConnectivityEvent{Kind::remove, 0, 1}};
  expect_throw<std::invalid_argument>([&] { offline_connectivity(2, remove); });
  const std::array invalid{ConnectivityEvent{Kind::query, 0, 2}};
  expect_throw<std::out_of_range>([&] { offline_connectivity(2, invalid); });
  const std::array duplicates{ConnectivityEvent{Kind::add, 0, 1},
                              ConnectivityEvent{Kind::add, 1, 0},
                              ConnectivityEvent{Kind::remove, 0, 1},
                              ConnectivityEvent{Kind::query, 0, 1},
                              ConnectivityEvent{Kind::remove, 1, 0},
                              ConnectivityEvent{Kind::query, 0, 1}};
  require(offline_connectivity(2, duplicates) == std::vector<bool>({true, false}));
  std::mt19937 random(512);
  for (std::uint32_t size = 1; size <= 10; ++size) {
    for (std::size_t count = 1; count <= 80; ++count) {
      std::map<Edge, int> active;
      std::vector<ConnectivityEvent> events;
      std::vector<bool> expected;
      for (std::size_t index = 0; index < count; ++index) {
        const auto first = static_cast<std::uint32_t>(random() % size);
        const auto second = static_cast<std::uint32_t>(random() % size);
        const Edge edge{std::min(first, second), std::max(first, second)};
        const auto action = random() % 3;
        if (action == 0) {
          events.push_back({Kind::add, first, second});
          ++active[edge];
        } else if (action == 1 && !active.empty()) {
          auto entry = active.begin();
          std::advance(entry, random() % active.size());
          events.push_back({Kind::remove, entry->first.first, entry->first.second});
          if (--entry->second == 0) {
            active.erase(entry);
          }
        } else {
          events.push_back({Kind::query, first, second});
          std::vector<bool> reached(size, false);
          std::vector<std::uint32_t> queue{first};
          reached[first] = true;
          for (std::size_t head = 0; head < queue.size(); ++head) {
            for (const auto& [live_edge, copies] : active) {
              (void)copies;
              const auto next = live_edge.first == queue[head]    ? live_edge.second
                                : live_edge.second == queue[head] ? live_edge.first
                                                                  : size;
              if (next < size && !reached[next]) {
                reached[next] = true;
                queue.push_back(next);
              }
            }
          }
          expected.push_back(reached[second]);
        }
      }
      require(offline_connectivity(size, events) == expected);
    }
  }
}

}  // namespace

int main() {
  try {
    test_disjoint_sets();
    test_segment_tree();
    test_dijkstra();
    test_compensated_sum();
    test_offline_connectivity();
    std::cout << "All algorithm and recipe checks passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
