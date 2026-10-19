#pragma once
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <map>

namespace algorithm_assets::mesh {
using Tetrahedron=std::array<Index,4>;
class TetrahedralMesh {
 public:
  TetrahedralMesh(std::vector<Point3> vertices,std::vector<Tetrahedron> cells,std::vector<std::uint32_t> regions={})
      :positions_(std::move(vertices)),cells_(std::move(cells)),regions_(std::move(regions)) {
    if(positions_.size()>=invalid_index||cells_.size()>=invalid_index)throw std::length_error("Tetrahedral index range");
    for(auto p:positions_)geometry3d::validate(p);
    if(regions_.empty())regions_.assign(cells_.size(),0);
    if(regions_.size()!=cells_.size())throw std::invalid_argument("Cell region shape");
    for(auto t:cells_){for(auto v:t)if(v>=positions_.size())throw std::out_of_range("Tetrahedron vertex");
      const double volume=geometry3d::oriented_volume6(positions_[t[0]],positions_[t[1]],positions_[t[2]],positions_[t[3]])/6;
      if(!(volume>0)||!std::isfinite(volume))throw std::domain_error("Tetrahedra must have finite positive approximate volume");
      volumes_.push_back(volume);
    }
  }
  [[nodiscard]] std::span<const Point3> positions()const{return positions_;}
  [[nodiscard]] std::span<const Tetrahedron> cells()const{return cells_;}
  [[nodiscard]] std::span<const std::uint32_t> regions()const{return regions_;}
  [[nodiscard]] std::span<const double> volumes()const{return volumes_;}
 private:
  std::vector<Point3> positions_;std::vector<Tetrahedron> cells_;std::vector<std::uint32_t> regions_;std::vector<double> volumes_;
};
struct TetrahedralBoundary {
  SurfaceMesh surface;
  std::vector<Index> source_cells;
  std::vector<unsigned> source_local_faces;
};
inline TetrahedralBoundary extract_tetrahedral_boundary(const TetrahedralMesh& mesh) {
  struct Use {Triangle oriented;Index cell;unsigned local;};std::map<Triangle,std::vector<Use>> faces;
  constexpr std::array<std::array<unsigned,3>,4> local{{{1,2,3},{0,3,2},{0,1,3},{0,2,1}}};
  for(Index cell=0;cell<mesh.cells().size();++cell)for(unsigned j=0;j<4;++j){const auto t=mesh.cells()[cell];Triangle f{t[local[j][0]],t[local[j][1]],t[local[j][2]]},key=f;std::sort(key.begin(),key.end());faces[key].push_back({f,cell,j});}
  SurfaceData d;d.positions.assign(mesh.positions().begin(),mesh.positions().end());TetrahedralBoundary out;
  for(const auto& [key,uses]:faces){
    (void)key;if(uses.size()>2)throw std::invalid_argument("Nonmanifold tetrahedral face");
    if(uses.size()==2){
      const auto a=uses[0].oriented,b=uses[1].oriented;bool opposite=false;
      for(unsigned j=0;j<3;++j)if(a[0]==b[j]&&a[1]==b[(j+2)%3]&&a[2]==b[(j+1)%3])opposite=true;
      if(!opposite)throw std::invalid_argument("Inconsistent internal face orientation");
      continue;
    }
    const auto use=uses[0];d.corner_vertices.insert(d.corner_vertices.end(),use.oriented.begin(),use.oriented.end());d.face_offsets.push_back(d.corner_vertices.size());
    d.face_material.push_back(mesh.regions()[use.cell]);d.face_ids.push_back(4*static_cast<LogicalId>(use.cell)+use.local+1);
    out.source_cells.push_back(use.cell);out.source_local_faces.push_back(use.local);
  }
  out.surface=SurfaceMesh(std::move(d));return out;
}
} // namespace algorithm_assets::mesh
