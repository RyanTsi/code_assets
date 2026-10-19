#include <algorithm_assets/mesh/cad_patch.hpp>
#include <algorithm_assets/mesh/bvh.hpp>
#include <algorithm_assets/mesh/edit.hpp>
#include <algorithm_assets/backends/eigen/mesh_operators.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>

using Clock = std::chrono::steady_clock;
double elapsed(Clock::time_point begin) {
    return std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
}
int main() {
    using namespace algorithm_assets::mesh;
    std::cout
        << "grid_segments,vertices,faces,surface_capacity_bytes,validation_ms,bvh_build_ms,query_1000_ms,operators_ms,split_commit_ms\n"
        << std::setprecision(8);
    for (unsigned segments : {32u, 96u, 192u}) {
        auto patch = sample_cad_patch(
            0, 1, 0, 1, segments, segments,
            [](double x, double y) {
                return Point3{x, y, 0.1 * x * y};
            },
            "benchmark", 1, 1);
        const auto& m = *patch.surface.mesh;
        const auto& d = m.data();
        const auto payload =
            d.positions.capacity() * sizeof(Point3) + d.corner_vertices.capacity() * sizeof(Index) +
            d.face_offsets.capacity() * sizeof(std::uint64_t) +
            d.corner_uv.capacity() * sizeof(Uv) + d.corner_normals.capacity() * sizeof(Vector3) +
            d.vertex_ids.capacity() * sizeof(LogicalId) +
            d.face_ids.capacity() * sizeof(LogicalId) +
            d.face_material.capacity() * sizeof(std::uint32_t);
        auto begin = Clock::now();
        auto valid = validate_surface(m);
        double validation = elapsed(begin);
        if (!valid.editable_triangles())
            return 1;
        begin = Clock::now();
        TriangleBvh bvh(patch.surface);
        double build = elapsed(begin);
        begin = Clock::now();
        double checksum = 0;
        for (unsigned i = 0; i < 1000; ++i)
            checksum += bvh.closest_point({(i % 97) / 96.0, (i % 71) / 70.0, 0.5})
                            ->closest.distance_squared;
        double queries = elapsed(begin);
        begin = Clock::now();
        auto operators = cotangent_operators(m);
        double assembly = elapsed(begin);
        if (operators.stiffness.rows() != static_cast<int>(m.vertex_count()) || !(checksum > 0))
            return 1;
        begin = Clock::now();
        MeshDocument document(patch.surface, 1);
        MeshEditSession edit(document);
        edit.split_edge(0);
        edit.commit();
        double editing = elapsed(begin);
        std::cout << segments << ',' << m.vertex_count() << ',' << m.face_count() << ',' << payload
                  << ',' << validation << ',' << build << ',' << queries << ',' << assembly << ','
                  << editing << '\n';
    }
}
