#pragma once

#include <algorithm_assets/geometry3d/primitives.hpp>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace algorithm_assets::mesh {
using geometry3d::Point3;
using geometry3d::Vector3;
using Index=std::uint32_t;
using LogicalId=std::uint64_t;
inline constexpr Index invalid_index=std::numeric_limits<Index>::max();
using Triangle=std::array<Index,3>;
using Uv=std::array<double,2>;

struct SurfaceData {
  std::vector<Point3> positions;
  std::vector<std::uint64_t> face_offsets{0};
  std::vector<Index> corner_vertices;
  std::vector<Uv> corner_uv;
  std::vector<Vector3> corner_normals;
  std::vector<std::uint32_t> face_material;
  std::vector<LogicalId> vertex_ids,face_ids;
  std::string length_unit="unspecified";
};

class SurfaceMesh {
 public:
  SurfaceMesh()=default;
  explicit SurfaceMesh(SurfaceData data):data_(std::move(data)) {
    const auto n=data_.positions.size();
    if(data_.length_unit.empty())throw std::invalid_argument("Empty length unit");
    for(unsigned char ch:data_.length_unit)if(ch<32||ch==127)throw std::invalid_argument("Invalid length unit");
    if(data_.face_offsets.empty()||data_.face_offsets.front()!=0||
       data_.face_offsets.back()!=data_.corner_vertices.size())
      throw std::invalid_argument("Invalid face offsets");
    const auto f=face_count(),c=data_.corner_vertices.size();
    if(n>=invalid_index||f>=invalid_index||c>=invalid_index)throw std::length_error("32-bit mesh domain exceeded");
    for(const auto p:data_.positions)geometry3d::validate(p);
    for(std::size_t i=0;i<f;++i)
      if(data_.face_offsets[i+1]<data_.face_offsets[i]||data_.face_offsets[i+1]-data_.face_offsets[i]<3)
        throw std::invalid_argument("Faces require at least three corners");
    for(const auto v:data_.corner_vertices)if(v>=n)throw std::out_of_range("Corner vertex");
    if(!data_.corner_uv.empty()&&data_.corner_uv.size()!=c)throw std::invalid_argument("UV domain mismatch");
    for(const auto uv:data_.corner_uv)for(double x:uv)if(!std::isfinite(x))throw std::invalid_argument("Nonfinite UV");
    if(!data_.corner_normals.empty()&&data_.corner_normals.size()!=c)throw std::invalid_argument("Normal domain mismatch");
    for(auto normal:data_.corner_normals)if(!std::isfinite(geometry3d::length(normal)))throw std::invalid_argument("Nonfinite normal");
    if(data_.face_material.empty())data_.face_material.assign(f,0);
    if(data_.face_material.size()!=f)throw std::invalid_argument("Material domain mismatch");
    prepare_ids(data_.vertex_ids,n);prepare_ids(data_.face_ids,f);
  }
  static SurfaceMesh from_triangles(std::vector<Point3> positions,std::span<const Triangle> faces) {
    SurfaceData d;d.positions=std::move(positions);
    for(auto t:faces){d.corner_vertices.insert(d.corner_vertices.end(),t.begin(),t.end());d.face_offsets.push_back(d.corner_vertices.size());}
    return SurfaceMesh(std::move(d));
  }
  [[nodiscard]] std::size_t vertex_count()const{return data_.positions.size();}
  [[nodiscard]] std::size_t face_count()const{return data_.face_offsets.size()-1;}
  [[nodiscard]] std::size_t corner_count()const{return data_.corner_vertices.size();}
  [[nodiscard]] const SurfaceData& data()const noexcept{return data_;}
  [[nodiscard]] std::span<const Point3> positions()const noexcept{return data_.positions;}
  [[nodiscard]] std::span<const Index> face(std::size_t i)const {
    if(i>=face_count())throw std::out_of_range("Face");
    return std::span<const Index>(data_.corner_vertices).subspan(data_.face_offsets[i],data_.face_offsets[i+1]-data_.face_offsets[i]);
  }
  [[nodiscard]] Triangle triangle(std::size_t i)const {
    const auto vertices=face(i);if(vertices.size()!=3)throw std::invalid_argument("Operation requires triangle faces");
    return {vertices[0],vertices[1],vertices[2]};
  }
  [[nodiscard]] bool all_triangles()const {
    for(std::size_t f=0;f<face_count();++f)if(data_.face_offsets[f+1]-data_.face_offsets[f]!=3)return false;
    return true;
  }
 private:
  SurfaceData data_;
  static void prepare_ids(std::vector<LogicalId>& ids,std::size_t count) {
    if(ids.empty()){ids.resize(count);std::iota(ids.begin(),ids.end(),LogicalId{1});}
    if(ids.size()!=count)throw std::invalid_argument("ID domain mismatch");
    std::unordered_set<LogicalId> unique;unique.reserve(count);
    for(auto id:ids)if(id==0||!unique.insert(id).second)throw std::invalid_argument("IDs must be unique and nonzero");
  }
};

struct MeshSnapshot {
  std::shared_ptr<const SurfaceMesh> mesh;
  std::uint64_t revision{};
};

} // namespace algorithm_assets::mesh
