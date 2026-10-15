#include <algorithm_assets/graph/bipartite_matching.hpp>
#include <iostream>
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  std::size_t left,right,m;std::cin>>left>>right>>m;
  std::vector<algorithm_assets::UnweightedCsrGraph::Edge> edges(m);
  for(auto& e:edges){std::cin>>e.from>>e.to;--e.from;--e.to;}
  algorithm_assets::UnweightedCsrGraph graph(std::max(left,right),edges);
  std::cout<<algorithm_assets::hopcroft_karp(left,right,graph).size<<'\n';
}
