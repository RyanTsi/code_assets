#pragma once
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <algorithm>
#include <numeric>
#include <queue>
#include <tuple>

namespace algorithm_assets::mesh {

// Owned CSR incidences and an indexed halfedge view. Nonmanifold edges have no twin.
struct SurfaceTopology {
  std::vector<std::array<Index,2>> edges;
  std::vector<Index> corner_face,corner_edge,next,twin,edge_corners,vertex_corners;
  std::vector<std::size_t> edge_offsets{0},vertex_offsets;
  explicit SurfaceTopology(const SurfaceMesh& mesh) {
    const auto& d=mesh.data();const auto count=mesh.corner_count();
    corner_face.resize(count);corner_edge.resize(count);next.resize(count);twin.assign(count,invalid_index);
    struct Record{Index a,b,corner;};std::vector<Record> records;records.reserve(count);
    vertex_offsets.assign(mesh.vertex_count()+1,0);
    for(Index f=0;f<mesh.face_count();++f)for(auto c=d.face_offsets[f];c<d.face_offsets[f+1];++c){
      const auto successor=c+1==d.face_offsets[f+1]?d.face_offsets[f]:c+1;
      corner_face[c]=f;next[c]=static_cast<Index>(successor);
      const Index a=d.corner_vertices[c],b=d.corner_vertices[successor];
      records.push_back({std::min(a,b),std::max(a,b),static_cast<Index>(c)});++vertex_offsets[a+1];
    }
    std::sort(records.begin(),records.end(),[](const Record&a,const Record&b){
      return std::tie(a.a,a.b,a.corner)<std::tie(b.a,b.b,b.corner);
    });
    for(std::size_t first=0;first<records.size();){
      auto last=first+1;while(last<records.size()&&records[last].a==records[first].a&&records[last].b==records[first].b)++last;
      const auto e=static_cast<Index>(edges.size());edges.push_back({records[first].a,records[first].b});
      for(auto i=first;i<last;++i){corner_edge[records[i].corner]=e;edge_corners.push_back(records[i].corner);}
      edge_offsets.push_back(edge_corners.size());
      if(last-first==2){twin[records[first].corner]=records[first+1].corner;twin[records[first+1].corner]=records[first].corner;}
      first=last;
    }
    std::partial_sum(vertex_offsets.begin(),vertex_offsets.end(),vertex_offsets.begin());
    auto cursor=vertex_offsets;vertex_corners.resize(count);
    for(Index c=0;c<count;++c)vertex_corners[cursor[d.corner_vertices[c]]++]=c;
  }
  [[nodiscard]] std::span<const Index> incident_corners(Index e)const {
    if(e>=edges.size())throw std::out_of_range("Edge");
    return std::span<const Index>(edge_corners).subspan(edge_offsets[e],edge_offsets[e+1]-edge_offsets[e]);
  }
};

struct SurfaceValidation {
  std::size_t boundary_edges{},nonmanifold_edges{},orientation_conflicts{},nonmanifold_vertices{},degenerate_faces{},duplicate_faces{},isolated_vertices{};
  bool triangles{};
  // Self intersections have NOT been tested by the combinatorial validation.
  [[nodiscard]] bool oriented_manifold()const{return nonmanifold_edges==0&&orientation_conflicts==0&&nonmanifold_vertices==0;}
  [[nodiscard]] bool editable_triangles()const{return triangles&&oriented_manifold()&&degenerate_faces==0&&duplicate_faces==0;}
};

inline SurfaceValidation validate_surface(const SurfaceMesh& mesh) {
  SurfaceValidation report;report.triangles=mesh.all_triangles();const auto& d=mesh.data();SurfaceTopology t(mesh);
  for(Index e=0;e<t.edges.size();++e){
    const auto corners=t.incident_corners(e);
    if(corners.size()==1)++report.boundary_edges;
    if(corners.size()>2)++report.nonmanifold_edges;
    if(corners.size()==2&&d.corner_vertices[corners[0]]==d.corner_vertices[corners[1]])++report.orientation_conflicts;
  }
  std::vector<Triangle> keys;keys.reserve(mesh.face_count());
  for(Index f=0;f<mesh.face_count();++f){
    const auto face=mesh.face(f);bool degenerate=false;
    if(face.size()==3){Triangle key{face[0],face[1],face[2]};std::sort(key.begin(),key.end());degenerate=key[0]==key[1]||key[1]==key[2];keys.push_back(key);}
    else{std::vector<Index> key(face.begin(),face.end());std::sort(key.begin(),key.end());degenerate=std::adjacent_find(key.begin(),key.end())!=key.end();}
    Vector3 area{};const auto p=d.positions[face[0]];
    for(std::size_t j=1;j+1<face.size();++j)area=area+geometry3d::cross(d.positions[face[j]]-p,d.positions[face[j+1]]-p);
    if(geometry3d::length(area)==0)degenerate=true;
    if(degenerate)++report.degenerate_faces;
  }
  std::sort(keys.begin(),keys.end());
  for(std::size_t i=1;i<keys.size();++i)if(keys[i]==keys[i-1])++report.duplicate_faces;
  std::vector<Index> mark(mesh.face_count(),invalid_index),stack;
  for(Index v=0;v<mesh.vertex_count();++v){
    const auto first=t.vertex_offsets[v],last=t.vertex_offsets[v+1];
    if(first==last){++report.isolated_vertices;continue;}
    std::size_t components=0,boundary=0;bool bad=false;
    for(auto i=first;i<last;++i){
      const auto c=t.vertex_corners[i];const auto f=t.corner_face[c];
      const auto begin=d.face_offsets[f],end=d.face_offsets[f+1];
      const auto previous=static_cast<Index>(c==begin?end-1:c-1);
      for(auto edge_corner:{c,previous}){
        const auto size=t.incident_corners(t.corner_edge[edge_corner]).size();
        if(size==1)++boundary;
        if(size>2)bad=true;
      }
      if(mark[f]==v)continue;
      ++components;mark[f]=v;stack.push_back(f);
      while(!stack.empty()){
        const auto current=stack.back();stack.pop_back();
        for(auto cc=d.face_offsets[current];cc<d.face_offsets[current+1];++cc){
          if(d.corner_vertices[cc]!=v&&d.corner_vertices[t.next[cc]]!=v)continue;
          for(auto other:t.incident_corners(t.corner_edge[cc])){
            const auto neighbor=t.corner_face[other];if(mark[neighbor]!=v){mark[neighbor]=v;stack.push_back(neighbor);}
          }
        }
      }
    }
    // Each boundary edge is visited from its sole incident face once at this vertex.
    if(bad||components!=1||(boundary!=0&&boundary!=2))++report.nonmanifold_vertices;
  }
  return report;
}

inline std::vector<std::vector<Index>> boundary_loops(const SurfaceMesh& mesh) {
  if(!validate_surface(mesh).oriented_manifold())throw std::invalid_argument("Boundary loops require oriented manifold");
  SurfaceTopology t(mesh);std::vector<Index> outgoing(mesh.vertex_count(),invalid_index);
  for(Index e=0;e<t.edges.size();++e){const auto corners=t.incident_corners(e);if(corners.size()==1)outgoing[mesh.data().corner_vertices[corners[0]]]=corners[0];}
  std::vector<bool> seen(mesh.corner_count());std::vector<std::vector<Index>> result;
  for(auto start:outgoing)if(start!=invalid_index&&!seen[start]){
    auto& loop=result.emplace_back();auto c=start;
    do{if(c==invalid_index||seen[c])throw std::domain_error("Broken boundary cycle");seen[c]=true;loop.push_back(mesh.data().corner_vertices[c]);c=outgoing[mesh.data().corner_vertices[t.next[c]]];}while(c!=start);
  }
  return result;
}
} // namespace algorithm_assets::mesh
