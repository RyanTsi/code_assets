#pragma once
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <algorithm>
#include <atomic>
#include <deque>

namespace algorithm_assets::mesh {
enum class EntityDomain {vertex,face};
struct EntityHandle {
  std::uint64_t document{},revision{};
  LogicalId logical_id{};
  EntityDomain domain{};
  Index dense_index{};
};

// Immutable owned snapshots; explicit bounded, full-snapshot undo/redo.
class MeshDocument {
 public:
  explicit MeshDocument(SurfaceMesh mesh,std::size_t history_limit=32)
      :MeshDocument(MeshSnapshot{std::make_shared<const SurfaceMesh>(std::move(mesh)),1},history_limit) {}
  explicit MeshDocument(MeshSnapshot initial,std::size_t history_limit=32)
      :current_(std::move(initial.mesh)),history_limit_(history_limit),identity_(next_identity()),revision_(initial.revision) {
    if(!current_||revision_==0)throw std::invalid_argument("Initial mesh snapshot");
    for(auto id:current_->data().vertex_ids)next_vertex_=std::max(next_vertex_,successor(id));
    for(auto id:current_->data().face_ids)next_face_=std::max(next_face_,successor(id));
  }
  MeshDocument(const MeshDocument&)=delete;
  MeshDocument& operator=(const MeshDocument&)=delete;
  [[nodiscard]] MeshSnapshot snapshot()const{return {current_,revision_};}
  [[nodiscard]] LogicalId next_vertex_id()const{return next_vertex_;}
  [[nodiscard]] LogicalId next_face_id()const{return next_face_;}
  [[nodiscard]] EntityHandle vertex_handle(Index v)const {
    if(v>=current_->vertex_count())throw std::out_of_range("Vertex handle");
    return {identity_,revision_,current_->data().vertex_ids[v],EntityDomain::vertex,v};
  }
  [[nodiscard]] EntityHandle face_handle(Index f)const {
    if(f>=current_->face_count())throw std::out_of_range("Face handle");
    return {identity_,revision_,current_->data().face_ids[f],EntityDomain::face,f};
  }
  [[nodiscard]] Index resolve(EntityHandle handle)const {
    if(handle.document!=identity_||handle.revision!=revision_)throw std::invalid_argument("Stale or foreign handle");
    if(handle.domain!=EntityDomain::vertex&&handle.domain!=EntityDomain::face)throw std::invalid_argument("Invalid entity domain");
    const auto& ids=handle.domain==EntityDomain::vertex?current_->data().vertex_ids:current_->data().face_ids;
    if(handle.dense_index>=ids.size()||ids[handle.dense_index]!=handle.logical_id)throw std::invalid_argument("Missing entity");
    return handle.dense_index;
  }
  void commit(const MeshSnapshot& base,SurfaceMesh replacement) {
    if(base.revision!=revision_||base.mesh!=current_)throw std::invalid_argument("Conflicting mesh revision");
    ensure_revision();auto value=std::make_shared<const SurfaceMesh>(std::move(replacement));
    const std::unordered_set<LogicalId> old_vertices(current_->data().vertex_ids.begin(),current_->data().vertex_ids.end());
    const std::unordered_set<LogicalId> old_faces(current_->data().face_ids.begin(),current_->data().face_ids.end());
    for(auto id:value->data().vertex_ids)if(!old_vertices.contains(id)&&id<next_vertex_)throw std::invalid_argument("Cannot reuse a retired vertex ID");
    for(auto id:value->data().face_ids)if(!old_faces.contains(id)&&id<next_face_)throw std::invalid_argument("Cannot reuse a retired face ID");
    auto nv=next_vertex_,nf=next_face_;
    for(auto id:value->data().vertex_ids)nv=std::max(nv,successor(id));
    for(auto id:value->data().face_ids)nf=std::max(nf,successor(id));
    // Potentially throwing history allocation happens before publishing a new state.
    if(history_limit_)undo_.push_back(current_);
    if(undo_.size()>history_limit_)undo_.pop_front();
    redo_.clear();current_=std::move(value);next_vertex_=nv;next_face_=nf;++revision_;
  }
  void set_position(EntityHandle handle,Point3 p) {
    if(handle.domain!=EntityDomain::vertex)throw std::invalid_argument("Not a vertex handle");
    const auto v=resolve(handle);geometry3d::validate(p);auto data=current_->data();data.positions[v]=p;data.corner_normals.clear();
    commit(snapshot(),SurfaceMesh(std::move(data)));
  }
  bool undo(){if(undo_.empty())return false;ensure_revision();redo_.push_back(current_);current_=undo_.back();undo_.pop_back();++revision_;return true;}
  bool redo(){if(redo_.empty())return false;ensure_revision();undo_.push_back(current_);current_=redo_.back();redo_.pop_back();++revision_;return true;}
 private:
  std::shared_ptr<const SurfaceMesh> current_;
  std::deque<std::shared_ptr<const SurfaceMesh>> undo_,redo_;
  std::size_t history_limit_;
  std::uint64_t identity_,revision_=1;
  LogicalId next_vertex_=1,next_face_=1;
  static LogicalId successor(LogicalId id){if(id==std::numeric_limits<LogicalId>::max())throw std::overflow_error("Logical ID exhausted");return id+1;}
  void ensure_revision()const{if(revision_==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Revision exhausted");}
  static std::uint64_t next_identity(){
    static std::atomic<std::uint64_t> next{1};auto value=next.load();
    do{if(value==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Document identity exhausted");}
    while(!next.compare_exchange_weak(value,value+1));
    return value;
  }
};
} // namespace algorithm_assets::mesh
