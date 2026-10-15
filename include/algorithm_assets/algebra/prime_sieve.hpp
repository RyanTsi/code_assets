#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>
namespace algorithm_assets {
class PrimeSieve {
public:
  explicit PrimeSieve(std::uint32_t limit) : limit_(limit), composite_((std::size_t{limit}/2+1+63)/64) {
    mark(1);
    for (std::uint64_t p=3;p<=limit/p;p+=2) if (!marked(p))
      for (auto multiple=p*p;multiple<=limit;multiple+=2*p) mark(multiple);
  }
  [[nodiscard]] bool is_prime(std::uint32_t x) const {
    if (x>limit_) throw std::out_of_range("Primality query outside sieve");
    return x==2 || (x>=3 && (x&1U) && !marked(x));
  }
  template<class Emit> void for_each_prime(Emit&& emit) const {
    if (limit_>=2) std::invoke(emit,std::uint32_t{2});
    for (std::uint64_t p=3;p<=limit_;p+=2) if (!marked(p)) std::invoke(emit,static_cast<std::uint32_t>(p));
  }
  [[nodiscard]] std::vector<std::uint32_t> primes() const {
    std::vector<std::uint32_t> result; for_each_prime([&](auto p){result.push_back(p);}); return result;
  }
private:
  bool marked(std::uint64_t odd) const noexcept { const auto i=odd/2; return (composite_[i/64]>>(i%64))&1U; }
  void mark(std::uint64_t odd) noexcept { const auto i=odd/2; composite_[i/64]|=std::uint64_t{1}<<(i%64); }
  std::uint32_t limit_; std::vector<std::uint64_t> composite_;
};
} // namespace algorithm_assets
