#pragma once
#include <algorithm_assets/mesh/geometry.hpp>
#include <bit>
#include <unordered_map>

namespace algorithm_assets::mesh {
struct RenderSnapshot {
    Point3 origin;
    std::vector<std::array<float, 3>> positions, normals;
    std::vector<std::array<float, 2>> uv;
    std::vector<Index> indices, source_corners;
    std::vector<LogicalId> source_faces;
    std::vector<std::uint32_t> materials;
    std::uint64_t revision{};
};
inline RenderSnapshot make_render_snapshot(const MeshSnapshot& source) {
    if (!source.mesh)
        throw std::invalid_argument("Null render source");
    const auto& mesh = *source.mesh;
    const auto& d = mesh.data();
    if (!mesh.all_triangles())
        throw std::invalid_argument("Render snapshot requires triangles");
    RenderSnapshot result;
    result.revision = source.revision;
    geometry3d::Aabb3 box;
    for (auto p : mesh.positions())
        box.include(p);
    if (!box.empty())
        result.origin = geometry3d::interpolate(box.minimum, box.maximum, 0.5);
    using Key = std::array<std::uint32_t, 7>;
    struct Hash {
        std::size_t operator()(const Key& key) const noexcept {
            std::size_t h = 0;
            for (auto x : key)
                h ^= std::hash<std::uint32_t>{}(x) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
    std::unordered_map<Key, Index, Hash> vertices;
    vertices.reserve(mesh.corner_count());
    auto narrow = [](double x) {
        if (!std::isfinite(x) || std::abs(x) > std::numeric_limits<float>::max())
            throw std::overflow_error("Render float conversion");
        return x == 0 ? 0.0f : static_cast<float>(x);
    };
    for (Index f = 0; f < mesh.face_count(); ++f) {
        const auto face_geometry = triangle_geometry(mesh, f);
        result.source_faces.push_back(d.face_ids[f]);
        result.materials.push_back(d.face_material[f]);
        for (unsigned j = 0; j < 3; ++j) {
            const Index c = 3 * f + j, v = d.corner_vertices[c];
            const auto n = d.corner_normals.empty() ? face_geometry.normal
                                                    : geometry3d::normalized(d.corner_normals[c]);
            const std::array<float, 3> normal{narrow(n.x), narrow(n.y), narrow(n.z)};
            const auto input_uv = d.corner_uv.empty() ? Uv{} : d.corner_uv[c];
            const std::array<float, 2> uv{narrow(input_uv[0]), narrow(input_uv[1])};
            Key key{v,
                    std::bit_cast<std::uint32_t>(normal[0]),
                    std::bit_cast<std::uint32_t>(normal[1]),
                    std::bit_cast<std::uint32_t>(normal[2]),
                    std::bit_cast<std::uint32_t>(uv[0]),
                    std::bit_cast<std::uint32_t>(uv[1]),
                    d.face_material[f]};
            const auto [it, added] =
                vertices.emplace(key, static_cast<Index>(result.positions.size()));
            if (added) {
                const auto p = d.positions[v] - result.origin;
                result.positions.push_back({narrow(p.x), narrow(p.y), narrow(p.z)});
                result.normals.push_back(normal);
                result.uv.push_back(uv);
            }
            result.indices.push_back(it->second);
            result.source_corners.push_back(c);
        }
    }
    return result;
}
} // namespace algorithm_assets::mesh
