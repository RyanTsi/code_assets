#include <algorithm_assets/geometry/closest_pair.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>
#include <random>
#include <string>
#include <vector>

namespace g = algorithm_assets::geometry;

namespace allocations {
bool tracking = false;
std::size_t count = 0;
}
void* operator new(std::size_t bytes) {
  if (auto* pointer = std::malloc(bytes == 0 ? 1 : bytes)) {
    if (allocations::tracking) ++allocations::count;
    return pointer;
  }
  throw std::bad_alloc();
}
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void* operator new(std::size_t bytes, const std::nothrow_t&) noexcept {
  try { return ::operator new(bytes); } catch (...) { return nullptr; }
}
void operator delete(void* pointer, const std::nothrow_t&) noexcept { std::free(pointer); }
template<class Function> std::size_t allocation_count(Function function) {
  allocations::count = 0; allocations::tracking = true;
  function();
  allocations::tracking = false;
  return allocations::count;
}

// A template-style comparison baseline: value copy, inplace_merge and hypot
// in the strip. Exact lexicographic sorting replaces the user's invalid epsilon
// comparator. Nondegenerate workloads only; not a repaired public asset.
double template_style(std::vector<g::Point> points) {
  std::sort(points.begin(), points.end(), [](auto a, auto b) { return a.x < b.x || (a.x == b.x && a.y < b.y); });
  std::vector<g::Point> strip(points.size());
  auto solve = [&](auto&& self, std::size_t first, std::size_t last) -> double {
    if (last - first <= 1) return std::numeric_limits<double>::infinity();
    const auto middle = first + (last - first) / 2;
    const auto split_x = points[middle].x;
    auto best = std::min(self(self, first, middle), self(self, middle, last));
    std::inplace_merge(points.begin() + first, points.begin() + middle, points.begin() + last, [](auto a, auto b) { return a.y < b.y; });
    std::size_t count = 0;
    for (auto i = first; i < last; ++i) {
      if (std::abs(points[i].x - split_x) >= best) continue;
      for (auto j = count; j > 0;) {
        --j;
        if (points[i].y - strip[j].y >= best) break;
        best = std::min(best, std::hypot(points[i].x - strip[j].x, points[i].y - strip[j].y));
      }
      strip[count++] = points[i];
    }
    return best;
  };
  return solve(solve, 0, points.size());
}

template<class Function> double median_ms(Function function, double& distance) {
  std::array<double, 5> timings{};
  for (auto& elapsed : timings) {
    const auto start = std::chrono::steady_clock::now();
    distance = function();
    elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  }
  std::sort(timings.begin(), timings.end());
  return timings[2];
}
int main() {
  std::cout << "distribution,n,template_style_ms,cold_workspace_ms,reused_workspace_ms,workspace_capacity_bytes,template_allocations,cold_allocations,reused_allocations,distance\n";
  std::mt19937_64 random(20261014);
  for (const std::string distribution : {"uniform", "collinear", "duplicate"}) {
    for (const std::size_t count : {5000U, 50000U, 100000U}) {
      std::vector<g::Point> points;
      points.reserve(count);
      for (std::size_t i = 0; i < count; ++i) {
        const double x = std::uniform_real_distribution<double>(-1e6, 1e6)(random);
        const double y = std::uniform_real_distribution<double>(-1e6, 1e6)(random);
        points.push_back(distribution == "collinear" ? g::Point{static_cast<double>(i), 0} : g::Point{x, y});
      }
      if (distribution == "duplicate") points.back() = points.front();
      g::ClosestPairWorkspace<double> workspace;
      (void)workspace.solve(points); // Explicit warm-up; cold path also reported.
      double baseline_distance = 0, cold_distance = 0, reused_distance = 0;
      const auto baseline = median_ms([&] { return template_style(points); }, baseline_distance);
      const auto cold = median_ms([&] { return g::closest_pair<double>(points)->distance(); }, cold_distance);
      const auto reused = median_ms([&] { return workspace.solve(points)->distance(); }, reused_distance);
      const auto baseline_allocations = allocation_count([&] { baseline_distance = template_style(points); });
      const auto cold_allocations = allocation_count([&] { cold_distance = g::closest_pair<double>(points)->distance(); });
      const auto reused_allocations = allocation_count([&] { reused_distance = workspace.solve(points)->distance(); });
      if (std::abs(baseline_distance - reused_distance) > 1e-8 * std::max(1., baseline_distance) || cold_distance != reused_distance) return 1;
      if (reused_allocations != 0) return 2;
      std::cout << std::setprecision(9) << distribution << ',' << count << ',' << baseline << ',' << cold << ',' << reused << ',' << workspace.storage_bytes() << ','
                << baseline_allocations << ',' << cold_allocations << ',' << reused_allocations << ',' << reused_distance << '\n';
    }
  }
}
