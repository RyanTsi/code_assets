#pragma once
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <algorithm_assets/mesh/detail/stream_format.hpp>
#include <charconv>
#include <iomanip>
#include <istream>
#include <locale>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string_view>

namespace algorithm_assets::mesh {
namespace detail {
inline std::int64_t integer_token(std::string_view s) {
    std::int64_t x{};
    const auto [end, error] = std::from_chars(s.data(), s.data() + s.size(), x);
    if (error != std::errc{} || end != s.data() + s.size())
        throw std::invalid_argument("Invalid OBJ integer");
    return x;
}
inline Index obj_index(std::string_view s, std::size_t size) {
    const auto x = integer_token(s);
    if (x == 0)
        throw std::out_of_range("OBJ index zero");
    if (size >= invalid_index)
        throw std::length_error("OBJ index domain");
    const auto i = x > 0 ? x - 1 : static_cast<std::int64_t>(size) + x;
    if (i < 0 || static_cast<std::uint64_t>(i) >= size)
        throw std::out_of_range("OBJ forward or invalid index");
    return static_cast<Index>(i);
}
inline void require_record_end(std::istringstream& row) {
    row.clear();
    std::string token;
    if (row >> token)
        if (!token.starts_with('#'))
            throw std::invalid_argument("Unexpected trailing OBJ data");
}
inline LogicalId logical_id_token(std::string_view token) {
    LogicalId id{};
    const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), id);
    if (error != std::errc{} || end != token.data() + token.size() || id == 0)
        throw std::invalid_argument("Invalid OBJ logical ID");
    return id;
}
} // namespace detail
struct ObjImport {
    SurfaceMesh mesh;
    std::map<std::uint32_t, std::string> materials;
    std::vector<std::string> omitted_records;
};
inline ObjImport read_obj(std::istream& input) {
    SurfaceData d;
    std::vector<Uv> texcoords;
    std::vector<Vector3> normals;
    std::map<std::string, std::uint32_t> material_ids;
    std::map<std::uint32_t, std::string> material_names;
    std::set<std::string> omitted;
    std::uint32_t material = 0;
    std::optional<LogicalId> next_vertex, next_face;
    for (std::string line; std::getline(input, line);) {
        std::istringstream row(line);
        row.imbue(std::locale::classic());
        std::string kind;
        if (!(row >> kind))
            continue;
        if (kind == "#") {
            std::string tag;
            row >> tag;
            if (tag == "aa_vid" || tag == "aa_fid") {
                std::string token;
                if (!(row >> token))
                    throw std::invalid_argument("Missing OBJ logical ID");
                (tag == "aa_vid" ? next_vertex : next_face) = detail::logical_id_token(token);
            } else if (tag == "aa_unit") {
                if (!(row >> std::quoted(d.length_unit)))
                    throw std::invalid_argument("Invalid OBJ unit");
            }
            continue;
        }
        if (kind == "v") {
            Point3 p;
            if (!(row >> p.x >> p.y >> p.z))
                throw std::invalid_argument("Invalid OBJ vertex");
            double weight;
            if (row >> weight) {
                if (weight != 1)
                    throw std::invalid_argument("Homogeneous OBJ vertices unsupported");
            }
            detail::require_record_end(row);
            d.positions.push_back(p);
            d.vertex_ids.push_back(next_vertex.value_or(d.positions.size()));
            next_vertex.reset();
        } else if (kind == "vt") {
            Uv uv;
            if (!(row >> uv[0] >> uv[1]))
                throw std::invalid_argument("OBJ requires two texture coordinates");
            double w;
            if (row >> w) {
                if (w != 0)
                    throw std::invalid_argument("3D OBJ texture coordinates unsupported");
            }
            detail::require_record_end(row);
            texcoords.push_back(uv);
        } else if (kind == "vn") {
            Vector3 n;
            if (!(row >> n.x >> n.y >> n.z))
                throw std::invalid_argument("Invalid OBJ normal");
            detail::require_record_end(row);
            normals.push_back(n);
        } else if (kind == "usemtl") {
            std::string name;
            if (!(row >> name))
                throw std::invalid_argument("Missing material name");
            constexpr std::string_view prefix = "aa_material_";
            if (name.starts_with(prefix)) {
                const auto number =
                    detail::integer_token(std::string_view(name).substr(prefix.size()));
                if (number < 0 ||
                    static_cast<std::uint64_t>(number) > std::numeric_limits<std::uint32_t>::max())
                    throw std::out_of_range("Material ID");
                material = static_cast<std::uint32_t>(number);
            } else {
                if (!material_ids.contains(name)) {
                    std::uint32_t id = 1;
                    while (material_names.contains(id)) {
                        if (id == std::numeric_limits<std::uint32_t>::max())
                            throw std::length_error("Material ID exhausted");
                        ++id;
                    }
                    material_ids[name] = id;
                }
                material = material_ids.at(name);
            }
            material_names[material] = name;
        } else if (kind == "f") {
            const auto first = d.corner_vertices.size();
            for (std::string token; row >> token;) {
                if (token.starts_with('#'))
                    break;
                const auto a = token.find('/'),
                           b = a == std::string::npos ? std::string::npos : token.find('/', a + 1);
                d.corner_vertices.push_back(
                    detail::obj_index(std::string_view(token).substr(0, a), d.positions.size()));
                if (a != std::string::npos) {
                    const auto uv = std::string_view(token).substr(
                        a + 1, b == std::string::npos ? std::string::npos : b - a - 1);
                    if (!uv.empty())
                        d.corner_uv.push_back(texcoords[detail::obj_index(uv, texcoords.size())]);
                    if (b != std::string::npos) {
                        const auto normal = std::string_view(token).substr(b + 1);
                        if (!normal.empty())
                            d.corner_normals.push_back(
                                normals[detail::obj_index(normal, normals.size())]);
                    }
                }
            }
            if (d.corner_vertices.size() - first < 3)
                throw std::invalid_argument("OBJ face needs three vertices");
            d.face_offsets.push_back(d.corner_vertices.size());
            d.face_material.push_back(material);
            d.face_ids.push_back(next_face.value_or(d.face_material.size()));
            next_face.reset();
        } else if (kind == "o" || kind == "g" || kind == "s" || kind == "mtllib")
            omitted.insert(kind);
        else
            throw std::invalid_argument("Unsupported OBJ record: " + kind);
    }
    if (input.bad())
        throw std::ios_base::failure("OBJ read failed");
    return {SurfaceMesh(std::move(d)), std::move(material_names), {omitted.begin(), omitted.end()}};
}
inline void write_obj(std::ostream& output, const SurfaceMesh& mesh) {
    detail::StreamFormat guard(output);
    const auto& d = mesh.data();
    output << "# algorithm_assets_obj 1\n# aa_unit " << std::quoted(d.length_unit) << '\n';
    for (std::size_t v = 0; v < mesh.vertex_count(); ++v) {
        const auto p = d.positions[v];
        output << "# aa_vid " << d.vertex_ids[v] << "\nv " << p.x << ' ' << p.y << ' ' << p.z
               << '\n';
    }
    for (auto uv : d.corner_uv)
        output << "vt " << uv[0] << ' ' << uv[1] << '\n';
    for (auto n : d.corner_normals)
        output << "vn " << n.x << ' ' << n.y << ' ' << n.z << '\n';
    for (std::size_t f = 0; f < mesh.face_count(); ++f) {
        output << "# aa_fid " << d.face_ids[f] << "\nusemtl aa_material_" << d.face_material[f]
               << "\nf";
        for (auto c = d.face_offsets[f]; c < d.face_offsets[f + 1]; ++c) {
            output << ' ' << d.corner_vertices[c] + 1;
            if (!d.corner_uv.empty() || !d.corner_normals.empty()) {
                output << '/';
                if (!d.corner_uv.empty())
                    output << c + 1;
                if (!d.corner_normals.empty())
                    output << '/' << c + 1;
            }
        }
        output << '\n';
    }
    if (!output)
        throw std::ios_base::failure("OBJ write failed");
}
} // namespace algorithm_assets::mesh
