#pragma once
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <functional>

namespace algorithm_assets::mesh {
struct CadPatchTessellation {
  MeshSnapshot surface;
  std::vector<Uv> vertex_parameters;
  std::vector<LogicalId> source_faces;
  std::string source_body;
  std::uint64_t source_revision{};
  LogicalId source_face{};
  [[nodiscard]] bool matches(const MeshSnapshot& candidate,std::uint64_t cad_revision)const {
    return candidate.mesh==surface.mesh&&candidate.revision==surface.revision&&cad_revision==source_revision;
  }
};

// Evaluator is a caller-owned untrimmed rectangular patch, not a B-rep/STEP reader.
template<class Evaluate>
CadPatchTessellation sample_cad_patch(double u0,double u1,double v0,double v1,
                                    std::size_t u_segments,std::size_t v_segments,Evaluate&& evaluate,
                                    std::string source_body,LogicalId face_id,std::uint64_t cad_revision) {
  for(double x:{u0,u1,v0,v1})if(!std::isfinite(x)||std::abs(x)>1e50)throw std::invalid_argument("Patch parameter domain");
  if(!(u0<u1&&v0<v1)||u_segments==0||v_segments==0||face_id==0||u_segments>=invalid_index||v_segments>=invalid_index||
     (u_segments+1)>static_cast<std::size_t>(invalid_index-1)/(v_segments+1)||
     u_segments>static_cast<std::size_t>(invalid_index-1)/6/v_segments)
    throw std::invalid_argument("Patch sampling size/domain");
  SurfaceData data;CadPatchTessellation result;result.source_body=std::move(source_body);result.source_revision=cad_revision;result.source_face=face_id;
  const auto width=u_segments+1;
  for(std::size_t j=0;j<=v_segments;++j)for(std::size_t i=0;i<=u_segments;++i){
    const double u=(1-static_cast<double>(i)/u_segments)*u0+static_cast<double>(i)/u_segments*u1;
    const double v=(1-static_cast<double>(j)/v_segments)*v0+static_cast<double>(j)/v_segments*v1;
    auto point=std::invoke(evaluate,u,v);geometry3d::validate(point);data.positions.push_back(point);result.vertex_parameters.push_back({u,v});
  }
  for(std::size_t j=0;j<v_segments;++j)for(std::size_t i=0;i<u_segments;++i){
    const auto a=static_cast<Index>(j*width+i),b=a+1,c=static_cast<Index>(a+width),d=c+1;
    for(const Triangle t:std::array<Triangle,2>{{{a,b,d},{a,d,c}}}){
      for(auto vertex:t){data.corner_vertices.push_back(vertex);data.corner_uv.push_back(result.vertex_parameters[vertex]);}
      data.face_offsets.push_back(data.corner_vertices.size());result.source_faces.push_back(face_id);
    }
  }
  result.surface={std::make_shared<const SurfaceMesh>(std::move(data)),1};return result;
}
} // namespace algorithm_assets::mesh
