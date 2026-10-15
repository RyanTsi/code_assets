#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
namespace algorithm_assets {
inline std::size_t increasing_subsequence_length(std::span<const std::int64_t> values,bool nondecreasing=false) {
  std::vector<std::int64_t> tails;
  for (auto x:values) {
    auto pos=nondecreasing?std::upper_bound(tails.begin(),tails.end(),x):std::lower_bound(tails.begin(),tails.end(),x);
    if (pos==tails.end()) tails.push_back(x); else *pos=x;
  }
  return tails.size();
}
inline std::vector<std::size_t> increasing_subsequence_indices(std::span<const std::int64_t> values,bool nondecreasing=false) {
  std::vector<std::size_t> tails,parent(values.size(),values.size());
  for (std::size_t i=0;i<values.size();++i) {
    auto pos=nondecreasing?
      std::upper_bound(tails.begin(),tails.end(),values[i],[&](auto value,auto index){return value<values[index];}):
      std::lower_bound(tails.begin(),tails.end(),values[i],[&](auto index,auto value){return values[index]<value;});
    const auto rank=static_cast<std::size_t>(pos-tails.begin());
    if (rank) parent[i]=tails[rank-1];
    if (pos==tails.end()) tails.push_back(i); else *pos=i;
  }
  std::vector<std::size_t> result(tails.size());
  if (!tails.empty()) {
    auto i=tails.back();
    for (std::size_t k=result.size();k-- >0;) { result[k]=i; i=parent[i]; }
  }
  return result;
}
inline std::size_t common_subsequence_length(std::string_view a,std::string_view b) {
  if (a.size()<b.size()) std::swap(a,b);
  if (b.empty()) return 0;
  std::vector<std::size_t> row(b.size()+1);
  for (auto x:a) {
    std::size_t previous=0;
    for (std::size_t j=1;j<=b.size();++j) {
      const auto old=row[j]; row[j]=x==b[j-1]?previous+1:std::max(row[j],row[j-1]); previous=old;
    }
  }
  return row.back();
}
} // namespace algorithm_assets
