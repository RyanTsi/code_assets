#pragma once

#include <algorithm_assets/connectivity/rollback_disjoint_set.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace algorithm_assets {

enum class ConnectivityEventKind { add, remove, query };

struct ConnectivityEvent {
  ConnectivityEventKind kind;
  std::uint32_t first;
  std::uint32_t second;
};

// Empty initial undirected multigraph. Results follow query order, not DFS order.
inline std::vector<bool> offline_connectivity(std::size_t vertex_count,
                                            std::span<const ConnectivityEvent> events) {
  if (vertex_count > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
    throw std::length_error("Offline connectivity requires at most INT32_MAX vertices");
  }
  const auto count = events.size();
  if (count == 0) {
    return {};
  }
  if (count > (std::vector<std::size_t>().max_size() - 1) / 2) {
    throw std::length_error("Offline connectivity timeline is too large");
  }
  using Edge = std::pair<std::uint32_t, std::uint32_t>;
  struct ActiveEdge {
    std::size_t start;
    std::size_t copies;
  };
  struct Interval {
    std::size_t first;
    std::size_t last;
    Edge edge;
  };
  std::vector<Interval> intervals;
  std::vector<std::size_t> query_indices(count);
  std::size_t query_count = 0;
  {
    // An ordered map gives deterministic O(q log(q+1)) preprocessing.
    std::map<Edge, ActiveEdge> active;
    for (std::size_t time = 0; time < count; ++time) {
      const auto& event = events[time];
      if (event.first >= vertex_count || event.second >= vertex_count) {
        throw std::out_of_range("Connectivity event endpoint is out of range");
      }
      const auto edge = Edge{std::min(event.first, event.second),
                             std::max(event.first, event.second)};
      switch (event.kind) {
      case ConnectivityEventKind::add: {
        auto [entry, inserted] = active.try_emplace(edge, ActiveEdge{time, 0});
        (void)inserted;
        ++entry->second.copies;
        break;
      }
      case ConnectivityEventKind::remove: {
        const auto entry = active.find(edge);
        if (entry == active.end()) {
          throw std::invalid_argument("Cannot remove an inactive edge");
        }
        if (--entry->second.copies == 0) {
          intervals.push_back({entry->second.start, time, edge});
          active.erase(entry);
        }
        break;
      }
      case ConnectivityEventKind::query:
        query_indices[time] = query_count++;
        break;
      default:
        throw std::invalid_argument("Unknown connectivity event kind");
      }
    }
    for (const auto& [edge, entry] : active) {
      intervals.push_back({entry.start, count, edge});
    }
  }
  if (query_count == 0) {
    return {};  // All events were still validated; no traversal is needed.
  }

  // CSR-style event storage: avoid a separately allocated vector at each node.
  std::vector<std::size_t> offsets(2 * count + 1, 0);
  const auto visit_nodes = [count](const Interval& interval, const auto& visit) {
    for (auto first = interval.first + count, last = interval.last + count; first < last;
         first /= 2, last /= 2) {
      if (first % 2 != 0) {
        visit(first++);
      }
      if (last % 2 != 0) {
        visit(--last);
      }
    }
  };
  for (const auto& interval : intervals) {
    visit_nodes(interval, [&](std::size_t node) { ++offsets[node + 1]; });
  }
  for (std::size_t node = 1; node < offsets.size(); ++node) {
    if (offsets[node] > std::numeric_limits<std::size_t>::max() - offsets[node - 1]) {
      throw std::length_error("Offline connectivity event count overflowed");
    }
    offsets[node] += offsets[node - 1];
  }
  std::vector<Edge> scheduled_edges(offsets.back());
  {
    auto cursor = offsets;
    for (const auto& interval : intervals) {
      visit_nodes(interval, [&](std::size_t node) {
        scheduled_edges[cursor[node]++] = interval.edge;
      });
    }
  }
  // Interval storage is no longer live during traversal.
  std::vector<Interval>().swap(intervals);

  RollbackDisjointSet components(vertex_count);
  std::vector<bool> answers(query_count);
  const auto traverse = [&](auto&& self, std::size_t node) -> void {
    const auto checkpoint = components.checkpoint();
    for (auto index = offsets[node]; index < offsets[node + 1]; ++index) {
      const auto [first, second] = scheduled_edges[index];
      components.merge(first, second);
    }
    if (node >= count) {
      const auto time = node - count;
      if (events[time].kind == ConnectivityEventKind::query) {
        answers[query_indices[time]] = components.same(events[time].first, events[time].second);
      }
    } else {
      self(self, 2 * node);
      self(self, 2 * node + 1);
    }
    components.rollback(checkpoint);
  };
  traverse(traverse, 1);
  return answers;
}

}  // namespace algorithm_assets
