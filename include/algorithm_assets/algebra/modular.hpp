#pragma once
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
namespace algorithm_assets {
class Modulus64 {
public:
    explicit Modulus64(std::uint64_t modulus) : modulus_(modulus) {
        if (modulus < 2)
            throw std::invalid_argument("Modulus64 requires modulus >= 2");
    }
    [[nodiscard]] std::uint64_t modulus() const noexcept {
        return modulus_;
    }
    [[nodiscard]] std::uint64_t add(std::uint64_t a, std::uint64_t b) const noexcept {
        a %= modulus_;
        b %= modulus_;
        return a >= modulus_ - b ? a - (modulus_ - b) : a + b;
    }
    [[nodiscard]] std::uint64_t subtract(std::uint64_t a, std::uint64_t b) const noexcept {
        a %= modulus_;
        b %= modulus_;
        return a >= b ? a - b : modulus_ - (b - a);
    }
    [[nodiscard]] std::uint64_t multiply(std::uint64_t a, std::uint64_t b) const noexcept {
#if defined(__SIZEOF_INT128__) && !defined(ALGORITHM_ASSETS_FORCE_PORTABLE_MODULAR)
        return static_cast<std::uint64_t>((static_cast<__uint128_t>(a) * b) % modulus_);
#else
        a %= modulus_;
        std::uint64_t result = 0;
        for (; b; b >>= 1) {
            if (b & 1U)
                result = add(result, a);
            a = add(a, a);
        }
        return result;
#endif
    }
    [[nodiscard]] std::uint64_t power(std::uint64_t base, std::uint64_t exponent) const noexcept {
        std::uint64_t result = 1;
        for (; exponent; exponent >>= 1) {
            if (exponent & 1U)
                result = multiply(result, base);
            base = multiply(base, base);
        }
        return result;
    }
    [[nodiscard]] std::optional<std::uint64_t> inverse(std::uint64_t value) const noexcept {
        auto r0 = modulus_, r1 = value % modulus_;
        std::uint64_t t0 = 0, t1 = 1;
        while (r1) {
            const auto quotient = r0 / r1, remainder = r0 % r1,
                       coefficient = subtract(t0, multiply(quotient, t1));
            r0 = r1;
            r1 = remainder;
            t0 = t1;
            t1 = coefficient;
        }
        return r0 == 1 ? std::optional<std::uint64_t>{t0} : std::nullopt;
    }

private:
    std::uint64_t modulus_;
};
struct Congruence {
    std::uint64_t residue, modulus;
};
inline std::optional<Congruence> chinese_remainder(std::span<const Congruence> equations) {
    Congruence result{0, 1};
    for (auto equation : equations) {
        if (!equation.modulus)
            throw std::invalid_argument("Zero CRT modulus");
        equation.residue %= equation.modulus;
        const auto common = std::gcd(result.modulus, equation.modulus);
        if (result.residue % common != equation.residue % common)
            return std::nullopt;
        const auto reduced = equation.modulus / common;
        if (result.modulus > std::numeric_limits<std::uint64_t>::max() / reduced)
            throw std::overflow_error("CRT modulus overflow");
        std::uint64_t factor = 0;
        if (reduced > 1) {
            const auto old = result.residue % equation.modulus;
            const auto difference = equation.residue >= old
                                        ? equation.residue - old
                                        : equation.modulus - (old - equation.residue);
            Modulus64 arithmetic(reduced);
            factor = arithmetic.multiply(difference / common,
                                         *arithmetic.inverse(result.modulus / common));
        }
        result.residue += result.modulus * factor;
        result.modulus *= reduced;
    }
    return result;
}
} // namespace algorithm_assets
