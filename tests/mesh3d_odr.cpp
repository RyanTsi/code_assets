#include <algorithm_assets/mesh/edit.hpp>
#include <algorithm_assets/backends/eigen/mesh_operators.hpp>
#include <algorithm_assets/mesh/vtu_io.hpp>
std::size_t mesh3d_odr_probe(){return algorithm_assets::mesh::SurfaceMesh{}.face_count();}
