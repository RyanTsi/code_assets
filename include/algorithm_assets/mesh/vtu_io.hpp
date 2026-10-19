#pragma once
#include <algorithm_assets/mesh/tetrahedral.hpp>
#include <algorithm_assets/mesh/detail/stream_format.hpp>
#include <ostream>
#include <string_view>

namespace algorithm_assets::mesh {
inline void write_vtu(std::ostream& out,const TetrahedralMesh& mesh,std::span<const double> vertex_scalar={},std::string_view field_name="value") {
  if(!vertex_scalar.empty()&&vertex_scalar.size()!=mesh.positions().size())throw std::invalid_argument("Vertex scalar field size");
  for(double x:vertex_scalar)if(!std::isfinite(x))throw std::invalid_argument("Nonfinite vertex field");
  std::string escaped;
  for(unsigned char ch:field_name){if(ch<32)throw std::invalid_argument("Invalid XML field name");switch(ch){case '&':escaped+="&amp;";break;case '<':escaped+="&lt;";break;case '>':escaped+="&gt;";break;case '"':escaped+="&quot;";break;default:escaped+=static_cast<char>(ch);}}
  detail::StreamFormat guard(out);
  out<<"<?xml version=\"1.0\"?>\n<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\"><UnstructuredGrid><Piece NumberOfPoints=\""<<mesh.positions().size()<<"\" NumberOfCells=\""<<mesh.cells().size()<<"\">\n";
  out<<"<Points><DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n";
  for(auto p:mesh.positions())out<<p.x<<' '<<p.y<<' '<<p.z<<'\n';
  out<<"</DataArray></Points>\n<Cells><DataArray type=\"UInt32\" Name=\"connectivity\" format=\"ascii\">\n";
  for(auto t:mesh.cells())out<<t[0]<<' '<<t[1]<<' '<<t[2]<<' '<<t[3]<<'\n';
  out<<"</DataArray><DataArray type=\"UInt64\" Name=\"offsets\" format=\"ascii\">\n";
  for(std::size_t i=0;i<mesh.cells().size();++i)out<<4*(i+1)<<' ';
  out<<"</DataArray><DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
  for(std::size_t i=0;i<mesh.cells().size();++i)out<<"10 ";
  out<<"</DataArray></Cells>\n<PointData>";
  if(!vertex_scalar.empty()){out<<"<DataArray type=\"Float64\" Name=\""<<escaped<<"\" format=\"ascii\">";for(double x:vertex_scalar)out<<x<<' ';out<<"</DataArray>";}
  out<<"</PointData><CellData><DataArray type=\"UInt32\" Name=\"region\" format=\"ascii\">";
  for(auto region:mesh.regions())out<<region<<' ';
  out<<"</DataArray></CellData></Piece></UnstructuredGrid></VTKFile>\n";
  if(!out)throw std::ios_base::failure("VTU write failed");
}
} // namespace algorithm_assets::mesh
