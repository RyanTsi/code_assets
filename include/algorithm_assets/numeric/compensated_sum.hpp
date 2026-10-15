#pragma once

#include <cmath>
#include <span>
#include <stdexcept>

namespace algorithm_assets {

// Neumaier compensation. Requires ordinary IEEE floating-point evaluation:
// reassociation/fast-math can invalidate the compensation formula.
class CompensatedSum {
public:
  void add(double value) {
    if (!std::isfinite(value)) {
      throw std::invalid_argument("CompensatedSum requires finite inputs");
    }
    const auto next = sum_ + value;
    if (!std::isfinite(next)) {
      throw std::overflow_error("CompensatedSum working sum overflowed");
    }
    const auto error = std::abs(sum_) >= std::abs(value) ? (sum_ - next) + value
                                                       : (value - next) + sum_;
    const auto correction = correction_ + error;
    if (!std::isfinite(correction) || !std::isfinite(next + correction)) {
      throw std::overflow_error("CompensatedSum correction overflowed");
    }
    sum_ = next;
    correction_ = correction;
  }

  [[nodiscard]] double value() const noexcept { return sum_ + correction_; }
  void reset() noexcept { sum_ = correction_ = 0.0; }

private:
  double sum_ = 0.0;
  double correction_ = 0.0;
};

inline double compensated_sum(std::span<const double> values) {
  CompensatedSum accumulator;
  for (const auto value : values) {
    accumulator.add(value);
  }
  return accumulator.value();
}

}  // namespace algorithm_assets
