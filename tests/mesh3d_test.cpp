#include <algorithm_assets/backends/eigen/mesh_operators.hpp>
#include <algorithm_assets/mesh/edit.hpp>
#include <algorithm_assets/mesh/bvh.hpp>
#include <algorithm_assets/mesh/render.hpp>
#include <algorithm_assets/mesh/cad_patch.hpp>
#include <algorithm_assets/mesh/vtu_io.hpp>
#include <algorithm_assets/mesh/obj_io.hpp>
#include <iostream>
#include <random>
#include <source_location>

using namespace algorithm_assets::mesh;
namespace g = algorithm_assets::geometry3d;
namespace {
void require(bool value, std::source_location location = std::source_location::current()) {
    if (!value)
        throw std::runtime_error("Mesh test line " + std::to_string(location.line()));
}
void close(double actual, double expected, double tolerance = 1e-10) {
    require(std::abs(actual - expected) <=
            tolerance * std::max({1.0, std::abs(actual), std::abs(expected)}));
}
template <class Function> void rejects(Function&& f) {
    bool failed = false;
    try {
        f();
    } catch (const std::exception&) {
        failed = true;
    }
    require(failed);
}
SurfaceMesh grid(unsigned segments, bool curved = false) {
    auto patch = sample_cad_patch(
        0, 1, 0, 1, segments, segments,
        [&](double x, double y) {
            return Point3{x, y, curved ? x * x + y * y : 0};
        },
        "test", 9, 1);
    return *patch.surface.mesh;
}
Index edge_between(const SurfaceMesh& mesh, Index a, Index b) {
    SurfaceTopology t(mesh);
    for (Index e = 0; e < t.edges.size(); ++e)
        if (t.edges[e] == std::array<Index, 2>{std::min(a, b), std::max(a, b)})
            return e;
    throw std::logic_error("Missing edge");
}
bool equal(const SurfaceMesh& a, const SurfaceMesh& b) {
    const auto& x = a.data();
    const auto& y = b.data();
    return x.positions == y.positions && x.corner_vertices == y.corner_vertices &&
           x.face_offsets == y.face_offsets && x.corner_uv == y.corner_uv &&
           x.corner_normals == y.corner_normals && x.face_material == y.face_material &&
           x.vertex_ids == y.vertex_ids && x.face_ids == y.face_ids &&
           x.length_unit == y.length_unit;
}

void storage_topology() {
    auto mesh = grid(4);
    const auto report = validate_surface(mesh);
    require(report.editable_triangles());
    require(report.boundary_edges == 16);
    require(report.nonmanifold_vertices == 0);
    const auto loops = boundary_loops(mesh);
    require(loops.size() == 1 && loops[0].size() == 16);
    SurfaceTopology t(mesh);
    require(t.edges.size() == 56);
    require(t.vertex_offsets.back() == 96);
    for (Index c = 0; c < mesh.corner_count(); ++c) {
        require(t.next[t.next[t.next[c]]] == c);
        if (t.twin[c] != invalid_index)
            require(t.twin[t.twin[c]] == c);
    }
    auto invalid = mesh.data();
    invalid.face_offsets.back()--;
    rejects([&] {
        SurfaceMesh x(invalid);
    });
    invalid = mesh.data();
    invalid.corner_vertices[0] = 1000;
    rejects([&] {
        SurfaceMesh x(invalid);
    });
    invalid = mesh.data();
    invalid.corner_uv.pop_back();
    rejects([&] {
        SurfaceMesh x(invalid);
    });
    invalid = mesh.data();
    invalid.vertex_ids[1] = invalid.vertex_ids[0];
    rejects([&] {
        SurfaceMesh x(invalid);
    });
    invalid = mesh.data();
    invalid.positions[0].x = std::numeric_limits<double>::quiet_NaN();
    rejects([&] {
        SurfaceMesh x(invalid);
    });
    const std::array<Triangle, 2> bowtie{{{0, 1, 2}, {0, 3, 4}}};
    auto bow = SurfaceMesh::from_triangles(
        {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {-1, 0, 0}, {0, -1, 0}}, bowtie);
    require(validate_surface(bow).nonmanifold_vertices == 1);
    MeshDocument bad(bow);
    rejects([&] {
        MeshEditSession e(bad);
    });
    SurfaceData quad;
    quad.positions = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    quad.face_offsets = {0, 4};
    quad.corner_vertices = {0, 1, 2, 3};
    SurfaceMesh polygon(quad);
    require(!polygon.all_triangles());
    require(validate_surface(polygon).oriented_manifold());
    rejects([&] {
        surface_geometry(polygon);
    });
    const std::array<Triangle, 3> nonmanifold{{{0, 1, 2}, {1, 0, 3}, {0, 1, 4}}};
    auto nm = SurfaceMesh::from_triangles({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}},
                                          nonmanifold);
    require(validate_surface(nm).nonmanifold_edges == 1);
}

void geometry_operators() {
    const std::array<Triangle, 1> face{{{0, 1, 2}}};
    auto m = SurfaceMesh::from_triangles({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}, face);
    auto geometry = surface_geometry(m);
    close(geometry.total_area, 0.5);
    close(geometry.face_normals[0].z, 1);
    const auto op = cotangent_operators(m);
    close(op.stiffness.coeff(0, 0), 1);
    close(op.stiffness.coeff(0, 1), -0.5);
    close(op.stiffness.coeff(1, 2), 0);
    for (double mass : op.lumped_mass)
        close(mass, 1.0 / 6);
    require((op.stiffness * Eigen::VectorXd::Ones(3)).norm() < 1e-12);
    auto mesh = grid(7);
    std::vector<Index> pins;
    std::vector<double> boundary, expected;
    for (Index v = 0; v < mesh.vertex_count(); ++v) {
        const auto p = mesh.positions()[v];
        expected.push_back(p.x + 2 * p.y);
        if (p.x == 0 || p.x == 1 || p.y == 0 || p.y == 1) {
            pins.push_back(v);
            boundary.push_back(expected.back());
        }
    }
    DirichletSystem system(mesh, pins);
    auto solution = system.solve(boundary);
    require(solution.free_dofs == 36);
    for (Index v = 0; v < mesh.vertex_count(); ++v)
        close(solution.values[v], expected[v]);
    std::fill(boundary.begin(), boundary.end(), 3);
    solution = system.solve(boundary);
    for (auto x : solution.values)
        close(x, 3);
    std::vector<double> load(mesh.vertex_count());
    Eigen::VectorXd x(mesh.vertex_count());
    for (Index v = 0; v < mesh.vertex_count(); ++v) {
        const auto p = mesh.positions()[v];
        x[v] = p.x * p.x + p.y * p.y;
    }
    const Eigen::VectorXd rhs = system.operators().stiffness * x;
    for (Index v = 0; v < mesh.vertex_count(); ++v)
        load[v] = rhs[v];
    for (std::size_t i = 0; i < pins.size(); ++i)
        boundary[i] = x[pins[i]];
    solution = system.solve(boundary, load);
    for (Index v = 0; v < mesh.vertex_count(); ++v)
        close(solution.values[v], x[v]);
    rejects([&] {
        DirichletSystem no_constraints(mesh, {});
    });
    std::array<Index, 2> duplicate{0, 0};
    rejects([&] {
        DirichletSystem duplicate_constraints(mesh, duplicate);
    });
    rejects([&] {
        (void)system.solve({});
    });
    auto bad = boundary;
    bad[0] = std::numeric_limits<double>::infinity();
    rejects([&] {
        (void)system.solve(bad);
    });
    const auto point = g::closest_point_on_triangle({0.2, 0.3, 2}, {0, 0, 0}, {1, 0, 0}, {0, 1, 0});
    close(point.point.x, 0.2);
    close(point.point.y, 0.3);
    close(point.distance_squared, 4);
    auto degenerate = g::closest_point_on_triangle({0.5, 1, 0}, {0, 0, 0}, {1, 0, 0}, {1, 0, 0});
    close(degenerate.distance_squared, 1);
    close(degenerate.point.x, 0.5);
    close(g::normalized({1e-320, 0, 0}).x, 1);
}

void edits_history() {
    const auto mesh = grid(1);
    MeshDocument document(mesh);
    const auto original = document.snapshot();
    const auto handle = document.vertex_handle(0);
    MeshEditSession editor(document);
    editor.flip_edge(edge_between(editor.preview(), 0, 3));
    editor.commit();
    require(document.snapshot().revision == 2);
    rejects([&] {
        (void)document.resolve(handle);
    });
    require(document.undo());
    require(equal(*document.snapshot().mesh, mesh));
    rejects([&] {
        (void)document.resolve(handle);
    });
    require(document.redo());
    require(document.undo());
    MeshEditSession split(document);
    const auto added = split.split_edge(edge_between(split.preview(), 0, 3), 0.25);
    require(added == 4);
    require(split.preview().face_count() == 4);
    close(split.preview().positions()[added].x, 0.25);
    close(surface_geometry(split.preview()).total_area, 1);
    const auto id = split.preview().data().vertex_ids[added];
    split.commit();
    require(document.undo());
    MeshEditSession another(document);
    const auto again = another.split_edge(edge_between(another.preview(), 0, 3));
    require(another.preview().data().vertex_ids[again] > id);
    another.commit();
    require(!document.redo());
    MeshEditSession conflict1(document), conflict2(document);
    conflict1.commit();
    const auto before_conflict = document.snapshot();
    rejects([&] {
        conflict2.commit();
    });
    require(document.snapshot().mesh == before_conflict.mesh);
    MeshDocument collapse_document(grid(4));
    MeshEditSession collapse(collapse_document);
    const auto before = collapse.preview().face_count();
    collapse.collapse_edge(edge_between(collapse.preview(), 12, 13));
    require(collapse.preview().vertex_count() == 24 &&
            collapse.preview().face_count() == before - 2);
    collapse.commit();
    require(collapse_document.undo());
    require(collapse_document.snapshot().mesh->vertex_count() == 25);
    auto seam = mesh.data();
    seam.corner_uv[3] = {7, 9};
    MeshDocument seam_document{SurfaceMesh(seam)};
    MeshEditSession seam_edit(seam_document);
    const auto saved = seam_edit.preview();
    rejects([&] {
        seam_edit.flip_edge(edge_between(seam_edit.preview(), 0, 3));
    });
    require(equal(saved, seam_edit.preview()));
    for (unsigned i = 0; i < 30; ++i) {
        MeshEditSession e(collapse_document);
        SurfaceTopology t(e.preview());
        bool changed = false;
        for (Index edge = 0; edge < t.edges.size(); ++edge)
            if (t.incident_corners(edge).size() == 2) {
                try {
                    e.flip_edge(edge);
                    changed = true;
                    break;
                } catch (const std::exception&) {
                }
            }
        require(changed);
        e.commit();
        require(validate_surface(*collapse_document.snapshot().mesh).editable_triangles());
        require(collapse_document.undo());
    }
    MeshDocument foreign(mesh);
    rejects([&] {
        (void)foreign.resolve(document.vertex_handle(0));
    });
    MeshDocument bounded(mesh, 1);
    bounded.set_position(bounded.vertex_handle(0), {0, 0, 0.1});
    bounded.set_position(bounded.vertex_handle(0), {0, 0, 0.2});
    require(bounded.undo());
    require(!bounded.undo());
    require(original.mesh->positions()[0] == Point3{0, 0, 0});
    MeshDocument retired(mesh);
    MeshEditSession r(retired);
    r.split_edge(edge_between(r.preview(), 0, 3));
    r.commit();
    require(retired.undo());
    auto reused = retired.snapshot().mesh->data();
    reused.vertex_ids[0] = 5;
    const auto before_reuse = retired.snapshot();
    rejects([&] {
        retired.commit(before_reuse, SurfaceMesh(reused));
    });
    require(retired.snapshot().mesh == before_reuse.mesh);
}

void spatial_render_io() {
    MeshDocument document(grid(8, true));
    TriangleBvh tree(document.snapshot());
    std::mt19937 random(7182);
    std::uniform_real_distribution<double> uniform(-0.5, 1.5);
    for (unsigned test = 0; test < 400; ++test) {
        const Point3 p{uniform(random), uniform(random), uniform(random)};
        const auto hit = tree.closest_point(p);
        require(hit.has_value());
        double best = std::numeric_limits<double>::infinity();
        const auto mesh = document.snapshot().mesh;
        for (Index f = 0; f < mesh->face_count(); ++f) {
            auto t = mesh->triangle(f);
            best = std::min(best, g::closest_point_on_triangle(p, mesh->positions()[t[0]],
                                                               mesh->positions()[t[1]],
                                                               mesh->positions()[t[2]])
                                      .distance_squared);
        }
        close(hit->closest.distance_squared, best);
    }
    MeshDocument flat(grid(8));
    TriangleBvh ray_tree(flat.snapshot());
    std::uniform_real_distribution<double> inside(0.01, 0.99);
    for (unsigned i = 0; i < 200; ++i) {
        auto hit = ray_tree.raycast({inside(random), inside(random), 2}, {0, 0, -4});
        require(hit.has_value());
        close(hit->distance, 2);
    }
    require(!ray_tree.raycast({3, 3, 2}, {0, 0, -1}));
    rejects([&] {
        (void)ray_tree.raycast({0, 0, 1}, {});
    });
    MeshQueryCache cache;
    cache.get(flat.snapshot());
    const auto stale = flat.snapshot();
    flat.set_position(flat.vertex_handle(0), {0, 0, 1});
    rejects([&] {
        ray_tree.require_current(flat.snapshot());
    });
    require(cache.get(flat.snapshot()).matches(flat.snapshot()));
    require(!cache.get(flat.snapshot()).matches(stale));
    auto data = grid(1).data();
    data.length_unit = "mm";
    data.corner_uv[3] = {5, 6};
    data.vertex_ids = {10, 20, 30, 40};
    data.face_ids = {101, 201};
    data.face_material = {3, 4};
    MeshDocument seams{SurfaceMesh(data)};
    const auto render = make_render_snapshot(seams.snapshot());
    require(render.positions.size() == 6 && render.indices.size() == 6 &&
            render.source_corners.size() == 6);
    require(render.source_faces == data.face_ids);
    for (std::size_t i = 0; i < render.indices.size(); ++i) {
        const auto index = render.indices[i];
        const auto corner = render.source_corners[i];
        close(render.uv[index][0], data.corner_uv[corner][0], 1e-6);
    }
    std::stringstream obj;
    write_obj(obj, *seams.snapshot().mesh);
    const auto loaded = read_obj(obj);
    require(equal(loaded.mesh, *seams.snapshot().mesh));
    std::istringstream negative("v 0 0 0\nv 1 0 0\nv 0 1 0\nf -3 -2 -1\n");
    require(read_obj(negative).mesh.triangle(0) == Triangle{0, 1, 2});
    std::istringstream partial("v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nf 1/1 2 3\n");
    rejects([&] {
        read_obj(partial);
    });
    std::istringstream trailing("v 0 0 1garbage\n");
    rejects([&] {
        read_obj(trailing);
    });
    std::istringstream negative_id("# aa_vid -1\nv 0 0 0\n");
    rejects([&] {
        read_obj(negative_id);
    });
}

void cad_cae() {
    auto patch = sample_cad_patch(
        0, 2, 0, 3, 5, 7,
        [](double u, double v) {
            return Point3{u, v, 0};
        },
        "plane", 42, 7);
    close(surface_geometry(*patch.surface.mesh).total_area, 6);
    require(patch.vertex_parameters.size() == 48);
    require(patch.source_faces.size() == 70);
    MeshDocument document(patch.surface);
    require(patch.matches(document.snapshot(), 7));
    require(!patch.matches(document.snapshot(), 8));
    document.set_position(document.vertex_handle(0), {0, 0, 0.1});
    require(!patch.matches(document.snapshot(), 7));
    TetrahedralMesh tet({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, -1}},
                        {{0, 1, 2, 3}, {0, 2, 1, 4}}, {3, 5});
    close(tet.volumes()[0] + tet.volumes()[1], 1.0 / 3);
    auto boundary = extract_tetrahedral_boundary(tet);
    require(boundary.surface.face_count() == 6);
    require(validate_surface(boundary.surface).boundary_edges == 0);
    for (Index f = 0; f < boundary.surface.face_count(); ++f) {
        auto t = boundary.surface.triangle(f);
        const auto p = boundary.surface.positions();
        const auto cell = tet.cells()[boundary.source_cells[f]];
        Point3 center{};
        for (auto v : cell)
            center = center + Vector3{p[v].x / 4, p[v].y / 4, p[v].z / 4};
        require(g::dot(g::cross(p[t[1]] - p[t[0]], p[t[2]] - p[t[0]]), center - p[t[0]]) < 0);
    }
    std::ostringstream xml;
    const std::array<double, 5> scalar{0, 1, 2, 3, 4};
    write_vtu(xml, tet, scalar, "p<&\"");
    require(xml.str().find("p&lt;&amp;&quot;") != std::string::npos);
    rejects([&] {
        TetrahedralMesh invalid({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, {{0, 2, 1, 3}});
    });
    TetrahedralMesh duplicate({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                              {{0, 1, 2, 3}, {0, 1, 2, 3}});
    rejects([&] {
        extract_tetrahedral_boundary(duplicate);
    });
}
} // namespace
int main() {
    try {
        storage_topology();
        geometry_operators();
        edits_history();
        spatial_render_io();
        cad_cae();
        std::cout
            << "Mesh3D: topology, Eigen operators/solves, atomic edits/history, 600 spatial queries, seam IO, CAD and tetrahedra passed\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
