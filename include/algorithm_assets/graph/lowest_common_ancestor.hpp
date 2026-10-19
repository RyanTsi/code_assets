#pragma once
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace algorithm_assets {
class LowestCommonAncestor {
public:
    using Vertex = std::uint32_t;
    // Parent forest: every root points to itself. Other cycles are rejected.
    explicit LowestCommonAncestor(std::span<const Vertex> parents) : n_(parents.size()) {
        if (n_ >= std::numeric_limits<Vertex>::max())
            throw std::length_error("Too many forest vertices");
        for (auto p : parents)
            if (p >= n_)
                throw std::out_of_range("Invalid parent");
        depth_.resize(n_);
        root_.resize(n_);
        std::vector<std::uint8_t> state(n_);
        std::vector<Vertex> path;
        for (std::size_t start = 0; start < n_; ++start) {
            if (state[start] == 2)
                continue;
            auto u = static_cast<Vertex>(start);
            path.clear();
            while (!state[u]) {
                state[u] = 1;
                path.push_back(u);
                if (parents[u] == u) {
                    depth_[u] = 0;
                    root_[u] = u;
                    state[u] = 2;
                    path.pop_back();
                    break;
                }
                u = parents[u];
            }
            if (state[u] == 1)
                throw std::invalid_argument("Parent relation contains a non-root cycle");
            while (!path.empty()) {
                const auto v = path.back();
                path.pop_back();
                depth_[v] = depth_[parents[v]] + 1;
                root_[v] = root_[parents[v]];
                state[v] = 2;
            }
        }
        const auto maximum = depth_.empty() ? 0 : *std::max_element(depth_.begin(), depth_.end());
        levels_ = static_cast<unsigned>(std::max(1, std::bit_width(maximum)));
        if (n_ && levels_ > up_.max_size() / n_)
            throw std::length_error("Ancestor table too large");
        up_.resize(n_ * levels_);
        std::copy(parents.begin(), parents.end(), up_.begin());
        for (unsigned level = 1; level < levels_; ++level)
            for (std::size_t v = 0; v < n_; ++v)
                up_[level * n_ + v] = up_[(level - 1) * n_ + up_[(level - 1) * n_ + v]];
    }
    [[nodiscard]] std::optional<Vertex> ancestor(Vertex v, std::uint32_t steps) const {
        validate(v);
        if (steps > depth_[v])
            return std::nullopt;
        for (unsigned bit = 0; steps; steps >>= 1, ++bit)
            if (steps & 1U)
                v = up_[bit * n_ + v];
        return v;
    }
    [[nodiscard]] std::optional<Vertex> query(Vertex a, Vertex b) const {
        validate(a);
        validate(b);
        if (root_[a] != root_[b])
            return std::nullopt;
        if (depth_[a] < depth_[b])
            std::swap(a, b);
        a = *ancestor(a, depth_[a] - depth_[b]);
        if (a == b)
            return a;
        for (unsigned level = levels_; level-- > 0;)
            if (up_[level * n_ + a] != up_[level * n_ + b]) {
                a = up_[level * n_ + a];
                b = up_[level * n_ + b];
            }
        return up_[a];
    }
    [[nodiscard]] std::optional<std::uint64_t> distance(Vertex a, Vertex b) const {
        const auto common = query(a, b);
        if (!common)
            return std::nullopt;
        return std::uint64_t{depth_[a]} + depth_[b] - 2 * std::uint64_t{depth_[*common]};
    }

private:
    void validate(Vertex v) const {
        if (v >= n_)
            throw std::out_of_range("Invalid forest vertex");
    }
    std::size_t n_;
    unsigned levels_ = 1;
    std::vector<Vertex> depth_, root_, up_;
};
} // namespace algorithm_assets
