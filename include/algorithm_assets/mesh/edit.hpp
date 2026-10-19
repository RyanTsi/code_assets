#pragma once
#include <algorithm_assets/mesh/document.hpp>
#include <algorithm_assets/mesh/topology.hpp>
#include <set>
#include <optional>

namespace algorithm_assets::mesh {

// Conservative indexed-halfedge editing. Validation/repacking are explicit O(n) or O(n log n),
// not a claim of constant-time local updates. Changes publish only on commit().
class MeshEditSession {
 public:
  explicit MeshEditSession(MeshDocument& document):owner_(&document),base_(document.snapshot()),draft_(*base_.mesh),
      next_vertex_(document.next_vertex_id()),next_face_(document.next_face_id()) {
    require_editable(draft_);
  }
  [[nodiscard]] const SurfaceMesh& preview()const{active();return draft_;}
  Index split_edge(Index edge,double parameter=0.5) {
    active();if(!std::isfinite(parameter)||parameter<=0||parameter>=1)throw std::invalid_argument("Split parameter must be inside (0,1)");
    SurfaceTopology t(draft_);if(edge>=t.edges.size())throw std::out_of_range("Edge");
    auto d=draft_.data();const auto endpoints=t.edges[edge];const auto added=static_cast<Index>(d.positions.size());
    if(next_vertex_==std::numeric_limits<LogicalId>::max())throw std::overflow_error("Vertex ID exhausted");
    auto nf=next_face_;
    d.positions.push_back(geometry3d::interpolate(d.positions[endpoints[0]],d.positions[endpoints[1]],parameter));
    d.vertex_ids.push_back(next_vertex_);
    for(auto corner:t.incident_corners(edge)){
      if(nf==std::numeric_limits<LogicalId>::max())throw std::overflow_error("Face ID exhausted");
      const Index f=t.corner_face[corner],local=corner-static_cast<Index>(draft_.data().face_offsets[f]);
      const auto tri=draft_.triangle(f);const Index a=tri[local],b=tri[(local+1)%3],c=tri[(local+2)%3];
      const double s=a==endpoints[0]?parameter:1-parameter;
      std::array<Uv,3> first_uv{},second_uv{};
      if(!d.corner_uv.empty()){
        const auto offset=draft_.data().face_offsets[f];const auto ua=draft_.data().corner_uv[offset+local];
        const auto ub=draft_.data().corner_uv[offset+(local+1)%3],uc=draft_.data().corner_uv[offset+(local+2)%3];
        const Uv middle{(1-s)*ua[0]+s*ub[0],(1-s)*ua[1]+s*ub[1]};first_uv={ua,middle,uc};second_uv={middle,ub,uc};
      }
      set_triangle(d,f,{a,added,c},first_uv);append_triangle(d,{added,b,c},second_uv,d.face_material[f],nf++);
    }
    d.corner_normals.clear();publish_draft(std::move(d));++next_vertex_;next_face_=nf;return added;
  }
  void flip_edge(Index edge) {
    active();SurfaceTopology t(draft_);const auto pair=t.incident_corners(edge);
    if(pair.size()!=2)throw std::invalid_argument("Flip requires an interior edge");
    const auto c0=pair[0],c1=pair[1];const auto f0=t.corner_face[c0],f1=t.corner_face[c1];
    const auto& old=draft_.data();const Index a=old.corner_vertices[c0],b=old.corner_vertices[t.next[c0]],
      c=old.corner_vertices[t.next[t.next[c0]]],d=old.corner_vertices[t.next[t.next[c1]]];
    if(c==d||old.face_material[f0]!=old.face_material[f1])throw std::invalid_argument("Flip crosses a material boundary or degenerate quad");
    const std::array<Index,2> diagonal{std::min(c,d),std::max(c,d)};
    if(std::find(t.edges.begin(),t.edges.end(),diagonal)!=t.edges.end())throw std::invalid_argument("Flip would create a duplicate edge");
    auto candidate=old;std::array<Uv,3> uv0{},uv1{};
    if(!old.corner_uv.empty()){
      auto uv_at=[&](Index face,Index v){const auto tri=draft_.triangle(face);for(unsigned j=0;j<3;++j)if(tri[j]==v)return old.corner_uv[3*face+j];throw std::logic_error("Missing corner");};
      if(uv_at(f0,a)!=uv_at(f1,a)||uv_at(f0,b)!=uv_at(f1,b))throw std::invalid_argument("Flip crosses a UV seam");
      uv0={uv_at(f0,c),uv_at(f1,d),uv_at(f0,b)};uv1={uv_at(f1,d),uv_at(f0,c),uv_at(f0,a)};
    }
    set_triangle(candidate,f0,{c,d,b},uv0);set_triangle(candidate,f1,{d,c,a},uv1);
    require_not_flipped(old,candidate,f0);require_not_flipped(old,candidate,f1);
    candidate.corner_normals.clear();publish_draft(std::move(candidate));
  }
  void collapse_edge(Index edge,double parameter=0.5) {
    active();if(!std::isfinite(parameter)||parameter<0||parameter>1)throw std::invalid_argument("Collapse parameter");
    SurfaceTopology t(draft_);const auto incident=t.incident_corners(edge);
    if(incident.size()!=2)throw std::invalid_argument("Collapse currently requires interior edge");
    const auto [a,b]=t.edges[edge];const auto& old=draft_.data();
    std::set<Index> na,nb,opposite;
    for(Index e=0;e<t.edges.size();++e){const auto x=t.edges[e];
      if(x[0]==a||x[1]==a){if(t.incident_corners(e).size()!=2)throw std::invalid_argument("Collapse boundary endpoint");na.insert(x[0]==a?x[1]:x[0]);}
      if(x[0]==b||x[1]==b){if(t.incident_corners(e).size()!=2)throw std::invalid_argument("Collapse boundary endpoint");nb.insert(x[0]==b?x[1]:x[0]);}
    }
    for(auto corner:incident)opposite.insert(old.corner_vertices[t.next[t.next[corner]]]);
    std::set<Index> common;std::set_intersection(na.begin(),na.end(),nb.begin(),nb.end(),std::inserter(common,common.end()));
    if(common!=opposite)throw std::invalid_argument("Collapse violates link condition");
    std::optional<Uv> ua,ub;std::optional<std::uint32_t> material;
    for(Index f=0;f<draft_.face_count();++f)for(unsigned j=0;j<3;++j){const auto v=old.corner_vertices[3*f+j];if(v!=a&&v!=b)continue;
      if(material&&*material!=old.face_material[f])throw std::invalid_argument("Collapse crosses a material feature");
      material=old.face_material[f];
      if(!old.corner_uv.empty()){auto& value=v==a?ua:ub;const auto uv=old.corner_uv[3*f+j];if(value&&*value!=uv)throw std::invalid_argument("Collapse crosses a UV seam");value=uv;}
    }
    SurfaceData result;result.length_unit=old.length_unit;result.positions=old.positions;result.vertex_ids=old.vertex_ids;
    result.positions[a]=geometry3d::interpolate(old.positions[a],old.positions[b],parameter);
    result.positions.erase(result.positions.begin()+b);result.vertex_ids.erase(result.vertex_ids.begin()+b);
    Uv merged{};if(ua&&ub)merged={(1-parameter)*(*ua)[0]+parameter*(*ub)[0],(1-parameter)*(*ua)[1]+parameter*(*ub)[1]};
    for(Index f=0;f<draft_.face_count();++f){
      auto tri=draft_.triangle(f);const bool has_a=std::find(tri.begin(),tri.end(),a)!=tri.end(),has_b=std::find(tri.begin(),tri.end(),b)!=tri.end();
      if(has_a&&has_b)continue;
      auto before=normal(old,tri);std::array<Uv,3> uv{};
      for(unsigned j=0;j<3;++j){const auto v=tri[j];if(!old.corner_uv.empty())uv[j]=(v==a||v==b)?merged:old.corner_uv[3*f+j];tri[j]=v==b?a:v>b?v-1:v;}
      if((has_a||has_b)&&!(geometry3d::dot(before,normal(result,tri))>0))throw std::domain_error("Collapse flips a face");
      if(!old.corner_uv.empty()&&result.corner_uv.empty())result.corner_uv.reserve(old.corner_uv.size());
      const bool copy_uv=!old.corner_uv.empty();
      result.corner_vertices.insert(result.corner_vertices.end(),tri.begin(),tri.end());result.face_offsets.push_back(result.corner_vertices.size());
      if(copy_uv)result.corner_uv.insert(result.corner_uv.end(),uv.begin(),uv.end());
      result.face_ids.push_back(old.face_ids[f]);result.face_material.push_back(old.face_material[f]);
    }
    publish_draft(std::move(result));
  }
  void commit(){active();owner_->commit(base_,draft_);closed_=true;}
 private:
  MeshDocument* owner_;MeshSnapshot base_;SurfaceMesh draft_;LogicalId next_vertex_,next_face_;bool closed_=false;
  void active()const{if(closed_)throw std::logic_error("Edit session already committed");}
  static void require_editable(const SurfaceMesh& m){if(!validate_surface(m).editable_triangles())throw std::invalid_argument("Editing requires nondegenerate oriented manifold triangles without duplicate faces");}
  void publish_draft(SurfaceData d){SurfaceMesh candidate(std::move(d));require_editable(candidate);draft_=std::move(candidate);}
  static Vector3 normal(const SurfaceData& d,Triangle t){return geometry3d::cross(d.positions[t[1]]-d.positions[t[0]],d.positions[t[2]]-d.positions[t[0]]);}
  static void require_not_flipped(const SurfaceData& old,const SurfaceData& candidate,Index f){
    const Triangle a{old.corner_vertices[3*f],old.corner_vertices[3*f+1],old.corner_vertices[3*f+2]},b{candidate.corner_vertices[3*f],candidate.corner_vertices[3*f+1],candidate.corner_vertices[3*f+2]};
    if(!(geometry3d::dot(normal(old,a),normal(candidate,b))>0))throw std::domain_error("Flip reverses face orientation");
  }
  static void set_triangle(SurfaceData& d,Index f,Triangle t,const std::array<Uv,3>& uv){
    for(unsigned j=0;j<3;++j){d.corner_vertices[3*f+j]=t[j];if(!d.corner_uv.empty())d.corner_uv[3*f+j]=uv[j];}
  }
  static void append_triangle(SurfaceData& d,Triangle t,const std::array<Uv,3>& uv,std::uint32_t material,LogicalId id){
    const bool has_uv=!d.corner_uv.empty();d.corner_vertices.insert(d.corner_vertices.end(),t.begin(),t.end());d.face_offsets.push_back(d.corner_vertices.size());
    if(has_uv)d.corner_uv.insert(d.corner_uv.end(),uv.begin(),uv.end());
    d.face_material.push_back(material);d.face_ids.push_back(id);
  }
};
} // namespace algorithm_assets::mesh
