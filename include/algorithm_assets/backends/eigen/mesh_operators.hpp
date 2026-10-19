#pragma once
#include <algorithm_assets/mesh/topology.hpp>
#include <algorithm_assets/connectivity/disjoint_set.hpp>
#include <Eigen/SparseCholesky>

namespace algorithm_assets::mesh {

struct CotangentOperators {
    Eigen::SparseMatrix<double> stiffness; // Positive semidefinite sign convention.
    std::vector<double> lumped_mass;       // Barycentric area, not mixed Voronoi area.
};

inline CotangentOperators cotangent_operators(const SurfaceMesh& mesh) {
    if (!mesh.all_triangles())
        throw std::invalid_argument("Cotangent operators require triangles");
    if (mesh.vertex_count() > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        mesh.face_count() > static_cast<std::size_t>(std::numeric_limits<int>::max()) / 12)
        throw std::length_error("Eigen int storage/triplet range exceeded");
    const auto n = static_cast<int>(mesh.vertex_count());
    CotangentOperators out;
    out.stiffness.resize(n, n);
    out.lumped_mass.assign(mesh.vertex_count(), 0);
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(12 * mesh.face_count());
    for (Index f = 0; f < mesh.face_count(); ++f) {
        const auto t = mesh.triangle(f);
        const auto p = mesh.positions();
        const double twice_area =
            geometry3d::length(geometry3d::cross(p[t[1]] - p[t[0]], p[t[2]] - p[t[0]]));
        if (!(twice_area > 0))
            throw std::domain_error("Degenerate triangle in cotangent operator");
        for (unsigned i = 0; i < 3; ++i) {
            const auto a = t[(i + 1) % 3], b = t[(i + 2) % 3];
            const double w = geometry3d::dot(p[a] - p[t[i]], p[b] - p[t[i]]) / (2 * twice_area);
            if (!std::isfinite(w))
                throw std::overflow_error("Nonfinite cotangent weight");
            triplets.emplace_back(a, a, w);
            triplets.emplace_back(b, b, w);
            triplets.emplace_back(a, b, -w);
            triplets.emplace_back(b, a, -w);
            out.lumped_mass[t[i]] += twice_area / 6;
        }
    }
    out.stiffness.setFromTriplets(triplets.begin(), triplets.end());
    out.stiffness.makeCompressed();
    for (int k = 0; k < out.stiffness.nonZeros(); ++k)
        if (!std::isfinite(out.stiffness.valuePtr()[k]))
            throw std::overflow_error("Stiffness assembly overflow");
    for (double m : out.lumped_mass)
        if (!std::isfinite(m))
            throw std::overflow_error("Mass assembly overflow");
    return out;
}

struct DirichletResult {
    std::vector<double> values;
    double relative_residual{};
    std::size_t free_dofs{};
};

// Reuses one factorization for fixed mesh and fixed constrained vertex indices.
class DirichletSystem {
public:
    DirichletSystem(const SurfaceMesh& mesh, std::span<const Index> fixed)
        : operators_(cotangent_operators(mesh)), fixed_(fixed.begin(), fixed.end()),
          free_index_(mesh.vertex_count(), -1) {
        const auto n = mesh.vertex_count();
        std::vector<bool> is_fixed(n);
        for (auto v : fixed_) {
            if (v >= n)
                throw std::out_of_range("Dirichlet vertex");
            if (is_fixed[v])
                throw std::invalid_argument("Duplicate Dirichlet vertex");
            is_fixed[v] = true;
        }
        DisjointSet components(n);
        for (Index f = 0; f < mesh.face_count(); ++f) {
            auto t = mesh.triangle(f);
            components.merge(t[0], t[1]);
            components.merge(t[1], t[2]);
        }
        std::vector<bool> anchored(n);
        for (auto v : fixed_)
            anchored[components.leader(v)] = true;
        for (Index v = 0; v < n; ++v)
            if (!anchored[components.leader(v)])
                throw std::invalid_argument(
                    "Every connected component needs a Dirichlet constraint");
        int count = 0;
        for (Index v = 0; v < n; ++v)
            if (!is_fixed[v])
                free_index_[v] = count++;
        reduced_.resize(count, count);
        std::vector<Eigen::Triplet<double>> entries;
        for (int col = 0; col < operators_.stiffness.outerSize(); ++col)
            for (Eigen::SparseMatrix<double>::InnerIterator it(operators_.stiffness, col); it; ++it)
                if (free_index_[it.row()] >= 0 && free_index_[it.col()] >= 0)
                    entries.emplace_back(free_index_[it.row()], free_index_[it.col()], it.value());
        reduced_.setFromTriplets(entries.begin(), entries.end());
        reduced_.makeCompressed();
        if (count) {
            solver_.compute(reduced_);
            if (solver_.info() != Eigen::Success || !(solver_.vectorD().minCoeff() > 0))
                throw std::domain_error("Dirichlet factorization is not numerically SPD");
        }
    }
    DirichletSystem(const DirichletSystem&) = delete;
    DirichletSystem& operator=(const DirichletSystem&) = delete;
    [[nodiscard]] const CotangentOperators& operators() const noexcept {
        return operators_;
    }
    [[nodiscard]] DirichletResult solve(std::span<const double> fixed_values,
                                        std::span<const double> load = {},
                                        double tolerance = 1e-9) const {
        const auto n = free_index_.size();
        if (fixed_values.size() != fixed_.size() || (!load.empty() && load.size() != n) ||
            !std::isfinite(tolerance) || tolerance <= 0 || tolerance > 1e-2)
            throw std::invalid_argument("Dirichlet input shape/tolerance");
        for (double x : fixed_values)
            if (!std::isfinite(x))
                throw std::invalid_argument("Nonfinite boundary value");
        for (double x : load)
            if (!std::isfinite(x))
                throw std::invalid_argument("Nonfinite load");
        DirichletResult result;
        result.values.assign(n, 0);
        result.free_dofs = reduced_.rows();
        for (std::size_t i = 0; i < fixed_.size(); ++i)
            result.values[fixed_[i]] = fixed_values[i];
        Eigen::VectorXd rhs = Eigen::VectorXd::Zero(reduced_.rows());
        for (Index v = 0; v < n; ++v)
            if (free_index_[v] >= 0 && !load.empty())
                rhs[free_index_[v]] = load[v];
        for (auto v : fixed_)
            for (Eigen::SparseMatrix<double>::InnerIterator it(operators_.stiffness,
                                                               static_cast<int>(v));
                 it; ++it)
                if (free_index_[it.row()] >= 0)
                    rhs[free_index_[it.row()]] -= it.value() * result.values[v];
        if (!rhs.allFinite())
            throw std::overflow_error("Dirichlet right hand side overflow");
        if (reduced_.rows()) {
            const Eigen::VectorXd x = solver_.solve(rhs);
            if (solver_.info() != Eigen::Success || !x.allFinite())
                throw std::domain_error("Dirichlet solve failed");
            const double denominator = rhs.norm() + reduced_.norm() * x.norm();
            const double residual = (reduced_ * x - rhs).norm();
            if (!std::isfinite(denominator) || !std::isfinite(residual))
                throw std::overflow_error("Dirichlet residual overflow");
            result.relative_residual =
                residual / std::max(denominator, std::numeric_limits<double>::min());
            if (result.relative_residual > tolerance)
                throw std::domain_error("Dirichlet residual exceeds tolerance");
            for (Index v = 0; v < n; ++v)
                if (free_index_[v] >= 0)
                    result.values[v] = x[free_index_[v]];
        }
        return result;
    }

private:
    CotangentOperators operators_;
    std::vector<Index> fixed_;
    std::vector<int> free_index_;
    Eigen::SparseMatrix<double> reduced_;
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver_;
};
} // namespace algorithm_assets::mesh
