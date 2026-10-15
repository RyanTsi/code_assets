#include <algorithm_assets/geometry/minkowski.hpp>
#include <iostream>
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  namespace g=algorithm_assets::geometry;
  std::size_t n,m,q;std::cin>>n>>m>>q;std::vector<g::IntegerPoint> a(n),b(m);
  for(auto& p:a)std::cin>>p.x>>p.y;
  for(auto& p:b)std::cin>>p.x>>p.y;
  auto first=g::convex_hull<std::int64_t>(a),second=g::convex_hull<std::int64_t>(b);
  g::TranslatedConvexOverlap collision(first,second);
  while(q--){g::IntegerPoint translation;std::cin>>translation.x>>translation.y;std::cout<<collision.intersects(translation)<<'\n';}
}
