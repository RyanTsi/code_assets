#include <algorithm_assets/backends/eigen/mesh_operators.hpp>
#include <algorithm_assets/mesh/edit.hpp>
#include <algorithm_assets/mesh/bvh.hpp>
#include <algorithm_assets/mesh/render.hpp>
#include <algorithm_assets/mesh/cad_patch.hpp>
#include <algorithm_assets/mesh/vtu_io.hpp>
#include <algorithm_assets/mesh/obj_io.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    try {
        using namespace algorithm_assets::mesh;
        const std::filesystem::path output = argc > 1 ? argv[1] : "mesh3d-output";
        std::filesystem::create_directories(output);
        auto patch = sample_cad_patch(
            0, 1, 0, 1, 8, 8,
            [](double u, double v) {
                constexpr double pi = 3.14159265358979323846;
                // Nonuniform planar triangles exercise general cotangents in the independent
                // oracle.
                return Point3{u + 0.08 * std::sin(pi * u) * std::sin(pi * v),
                              v + 0.05 * std::sin(2 * pi * u) * std::sin(pi * v), 0};
            },
            "analytic_plane", 42, 1);
        MeshDocument document(patch.surface);
        const auto mesh = document.snapshot().mesh;
        std::vector<Index> pins;
        std::vector<double> values;
        for (Index v = 0; v < mesh->vertex_count(); ++v) {
            const auto p = mesh->positions()[v];
            if (p.x == 0 || p.x == 1 || p.y == 0 || p.y == 1) {
                pins.push_back(v);
                values.push_back(p.x + 2 * p.y);
            }
        }
        DirichletSystem system(*mesh, pins);
        const auto solved = system.solve(values);
        std::ofstream obj(output / "surface.obj");
        write_obj(obj, *mesh);
        obj.close();
        std::ifstream input(output / "surface.obj");
        const auto imported = read_obj(input);
        std::ofstream matrix(output / "stiffness.mtx");
        matrix << std::setprecision(17) << "%%MatrixMarket matrix coordinate real general\n"
               << mesh->vertex_count() << ' ' << mesh->vertex_count() << ' '
               << system.operators().stiffness.nonZeros() << '\n';
        for (int col = 0; col < system.operators().stiffness.outerSize(); ++col)
            for (Eigen::SparseMatrix<double>::InnerIterator it(system.operators().stiffness, col);
                 it; ++it)
                matrix << it.row() + 1 << ' ' << it.col() + 1 << ' ' << it.value() << '\n';
        std::ofstream field(output / "solution.csv");
        field << "vertex,x,y,z,value\n" << std::setprecision(17);
        for (Index v = 0; v < mesh->vertex_count(); ++v) {
            const auto p = mesh->positions()[v];
            field << v << ',' << p.x << ',' << p.y << ',' << p.z << ',' << solved.values[v] << '\n';
        }
        TetrahedralMesh tetrahedra({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, -1}},
                                   {{0, 1, 2, 3}, {0, 2, 1, 4}}, {3, 5});
        const std::array<double, 5> temperature{0, 1, 2, 3, 4};
        std::ofstream vtu(output / "analysis.vtu");
        write_vtu(vtu, tetrahedra, temperature, "temperature");
        const auto boundary = extract_tetrahedral_boundary(tetrahedra);
        std::ofstream boundary_file(output / "tet_boundary.obj");
        write_obj(boundary_file, boundary.surface);
        MeshEditSession edit(document);
        SurfaceTopology topology(edit.preview());
        for (Index e = 0; e < topology.edges.size(); ++e)
            if (topology.incident_corners(e).size() == 2) {
                edit.split_edge(e);
                break;
            }
        edit.commit();
        const bool undone = document.undo(), redone = document.redo();
        TriangleBvh bvh(document.snapshot());
        const auto closest = bvh.closest_point({0.2, 0.3, 1});
        const auto render = make_render_snapshot(document.snapshot());
        std::cout << "Imported faces: " << imported.mesh.face_count()
                  << "; constrained solve free DOFs: " << solved.free_dofs
                  << "; residual: " << solved.relative_residual << "\n";
        std::cout << "Undo/redo: " << undone << '/' << redone
                  << "; closest distance squared: " << closest->closest.distance_squared
                  << "; render vertices: " << render.positions.size()
                  << "; tetrahedral boundary faces: " << boundary.surface.face_count() << "\n";
        std::cout << "Outputs: " << std::filesystem::absolute(output).string() << "\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
