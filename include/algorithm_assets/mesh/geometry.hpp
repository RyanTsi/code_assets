#pragma once
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <algorithm_assets/numeric/compensated_sum.hpp>

namespace algorithm_assets::mesh {
struct SurfaceGeometry {
    std::vector<double> face_areas;
    std::vector<Vector3> face_normals, vertex_normals;
    double total_area{};
};
struct TriangleGeometry {
    double area;
    Vector3 normal;
};
inline TriangleGeometry triangle_geometry(const SurfaceMesh& mesh, Index face) {
    const auto t = mesh.triangle(face);
    const auto p = mesh.positions();
    const auto n = geometry3d::cross(p[t[1]] - p[t[0]], p[t[2]] - p[t[0]]);
    const double size = geometry3d::length(n);
    if (!(size > 0))
        throw std::domain_error("Degenerate triangle");
    return {size / 2, n / size};
}
inline SurfaceGeometry surface_geometry(const SurfaceMesh& mesh) {
    if (!mesh.all_triangles())
        throw std::invalid_argument("Geometry operator requires triangles");
    SurfaceGeometry result;
    result.vertex_normals.resize(mesh.vertex_count());
    CompensatedSum sum;
    for (Index f = 0; f < mesh.face_count(); ++f) {
        auto t = mesh.triangle(f);
        const auto p = mesh.positions();
        const auto n = geometry3d::cross(p[t[1]] - p[t[0]], p[t[2]] - p[t[0]]);
        const double twice_area = geometry3d::length(n);
        if (!(twice_area > 0))
            throw std::domain_error("Degenerate triangle");
        result.face_areas.push_back(twice_area / 2);
        result.face_normals.push_back(n / twice_area);
        sum.add(twice_area / 2);
        for (auto v : t)
            result.vertex_normals[v] = result.vertex_normals[v] + n;
    }
    for (auto& n : result.vertex_normals) {
        const double s = geometry3d::length(n);
        if (s > 0)
            n = n / s;
    }
    result.total_area = sum.value();
    return result;
}
} // namespace algorithm_assets::mesh
