#include <algorithm_assets/algebra/xor_basis.hpp>
#include <algorithm_assets/graph/kruskal.hpp>
#include <algorithm_assets/recipes/inversions_fenwick.hpp>
#include <algorithm_assets/strings/kmp.hpp>

bool repository_assets_odr_probe() {
  algorithm_assets::XorBasis64 basis;
  basis.insert(5);
  return basis.contains(5) && algorithm_assets::KmpPattern("ABA").contains("ABABA");
}
