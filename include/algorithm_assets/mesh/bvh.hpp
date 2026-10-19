#pragma once
#include <algorithm_assets/mesh/surface_mesh.hpp>
#include <optional>

namespace algorithm_assets::mesh {
struct MeshClosestPoint {
    Index face{};
    geometry3d::TriangleClosestPoint closest;
    std::uint64_t revision{};
};
struct MeshRayHit {
    Index face{};
    double distance{};
    std::array<double, 3> barycentric{};
    std::uint64_t revision{};
};

class TriangleBvh {
public:
    explicit TriangleBvh(MeshSnapshot snapshot, std::size_t leaf_size = 4)
        : source_(std::move(snapshot)), leaf_size_(leaf_size) {
        if (!source_.mesh || !source_.mesh->all_triangles() || leaf_size == 0 || leaf_size > 64)
            throw std::invalid_argument("BVH input");
        const auto f = source_.mesh->face_count();
        if (f >= invalid_index / 2)
            throw std::length_error("BVH node range");
        order_.resize(f);
        std::iota(order_.begin(), order_.end(), Index{0});
        nodes_.reserve(2 * f);
        if (f)
            build(0, static_cast<Index>(f));
    }
    [[nodiscard]] bool matches(const MeshSnapshot& s) const {
        return s.mesh == source_.mesh && s.revision == source_.revision;
    }
    void require_current(const MeshSnapshot& s) const {
        if (!matches(s))
            throw std::invalid_argument("Stale BVH snapshot");
    }
    [[nodiscard]] std::optional<MeshClosestPoint> closest_point(Point3 point) const {
        geometry3d::validate(point);
        if (nodes_.empty())
            return std::nullopt;
        std::optional<MeshClosestPoint> best;
        double distance = std::numeric_limits<double>::infinity();
        std::vector<Index> stack{0};
        while (!stack.empty()) {
            const auto index = stack.back();
            stack.pop_back();
            const auto& node = nodes_[index];
            if (node.box.distance_squared(point) > distance)
                continue;
            if (node.count) {
                for (Index i = node.first; i < node.first + node.count; ++i) {
                    const auto f = order_[i];
                    const auto t = source_.mesh->triangle(f);
                    const auto p = source_.mesh->positions();
                    auto value =
                        geometry3d::closest_point_on_triangle(point, p[t[0]], p[t[1]], p[t[2]]);
                    if (value.distance_squared < distance ||
                        (value.distance_squared == distance && (!best || f < best->face))) {
                        distance = value.distance_squared;
                        best = MeshClosestPoint{f, value, source_.revision};
                    }
                }
            } else {
                const double a = nodes_[node.left].box.distance_squared(point),
                             b = nodes_[node.right].box.distance_squared(point);
                if (a < b) {
                    stack.push_back(node.right);
                    stack.push_back(node.left);
                } else {
                    stack.push_back(node.left);
                    stack.push_back(node.right);
                }
            }
        }
        return best;
    }
    [[nodiscard]] std::optional<MeshRayHit>
    raycast(Point3 origin, Vector3 direction,
            double maximum_distance = std::numeric_limits<double>::infinity()) const {
        geometry3d::validate(origin);
        direction = geometry3d::normalized(direction);
        if (std::isnan(maximum_distance) || maximum_distance < 0)
            throw std::invalid_argument("Ray maximum distance");
        std::optional<MeshRayHit> best;
        if (nodes_.empty())
            return best;
        std::vector<Index> stack{0};
        while (!stack.empty()) {
            const auto& node = nodes_[stack.back()];
            stack.pop_back();
            if (!ray_box(node.box, origin, direction, maximum_distance))
                continue;
            if (!node.count) {
                stack.push_back(node.right);
                stack.push_back(node.left);
                continue;
            }
            for (Index i = node.first; i < node.first + node.count; ++i) {
                const auto f = order_[i];
                const auto t = source_.mesh->triangle(f);
                const auto p = source_.mesh->positions();
                const auto a = p[t[0]];
                const auto e1 = p[t[1]] - a, e2 = p[t[2]] - a;
                const auto h = geometry3d::cross(direction, e2);
                const double det = geometry3d::dot(e1, h);
                if (std::abs(det) <= 64 * std::numeric_limits<double>::epsilon() *
                                         geometry3d::length(e1) * geometry3d::length(e2))
                    continue;
                const auto s = origin - a;
                const double u = geometry3d::dot(s, h) / det;
                const auto q = geometry3d::cross(s, e1);
                const double v = geometry3d::dot(direction, q) / det;
                if (u < 0 || v < 0 || u + v > 1)
                    continue;
                const double distance = geometry3d::dot(e2, q) / det;
                if (distance >= 0 && distance <= maximum_distance &&
                    (!best || distance < maximum_distance || f < best->face)) {
                    maximum_distance = distance;
                    best = MeshRayHit{f, distance, {1 - u - v, u, v}, source_.revision};
                }
            }
        }
        return best;
    }

private:
    struct Node {
        geometry3d::Aabb3 box;
        Index left{}, right{}, first{}, count{};
    };
    MeshSnapshot source_;
    std::size_t leaf_size_;
    std::vector<Index> order_;
    std::vector<Node> nodes_;
    geometry3d::Aabb3 face_box(Index f) const {
        geometry3d::Aabb3 b;
        for (auto v : source_.mesh->triangle(f))
            b.include(source_.mesh->positions()[v]);
        return b;
    }
    Index build(Index first, Index last) {
        const auto id = static_cast<Index>(nodes_.size());
        nodes_.push_back({});
        geometry3d::Aabb3 box, centers;
        for (auto i = first; i < last; ++i) {
            const auto b = face_box(order_[i]);
            box.include(b);
            centers.include(geometry3d::interpolate(b.minimum, b.maximum, 0.5));
        }
        nodes_[id].box = box;
        if (last - first <= leaf_size_) {
            nodes_[id].first = first;
            nodes_[id].count = last - first;
            return id;
        }
        const auto extent = centers.maximum - centers.minimum;
        const unsigned axis = extent.x >= extent.y && extent.x >= extent.z ? 0
                              : extent.y >= extent.z                       ? 1
                                                                           : 2;
        const Index middle = first + (last - first) / 2;
        std::nth_element(
            order_.begin() + first, order_.begin() + middle, order_.begin() + last,
            [&](Index a, Index b) {
                const auto ba = face_box(a), bb = face_box(b);
                const double ca = geometry3d::coordinate(
                                 geometry3d::interpolate(ba.minimum, ba.maximum, 0.5), axis),
                             cb = geometry3d::coordinate(
                                 geometry3d::interpolate(bb.minimum, bb.maximum, 0.5), axis);
                return ca < cb || (ca == cb && a < b);
            });
        const auto left = build(first, middle), right = build(middle, last);
        nodes_[id].left = left;
        nodes_[id].right = right;
        return id;
    }
    static bool ray_box(const geometry3d::Aabb3& box, Point3 origin, Vector3 d, double maximum) {
        double first = 0, last = maximum;
        const std::array<double, 3> direction{d.x, d.y, d.z};
        for (unsigned axis = 0; axis < 3; ++axis) {
            const double x = geometry3d::coordinate(origin, axis),
                         lo = geometry3d::coordinate(box.minimum, axis),
                         hi = geometry3d::coordinate(box.maximum, axis);
            if (direction[axis] == 0) {
                if (x < lo || x > hi)
                    return false;
                continue;
            }
            double a = (lo - x) / direction[axis], b = (hi - x) / direction[axis];
            if (a > b)
                std::swap(a, b);
            first = std::max(first, a);
            last = std::min(last, b);
            if (first > last)
                return false;
        }
        return true;
    }
};

// Single-writer cache. Readers may share the returned immutable BVH until the next get().
class MeshQueryCache {
public:
    const TriangleBvh& get(const MeshSnapshot& source) {
        if (!cache_ || !cache_->matches(source)) {
            auto replacement = std::make_unique<TriangleBvh>(source);
            cache_ = std::move(replacement);
        }
        return *cache_;
    }

private:
    std::unique_ptr<TriangleBvh> cache_;
};
} // namespace algorithm_assets::mesh
