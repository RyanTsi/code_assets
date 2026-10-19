#pragma once
// Application-side composition example, using documented asset APIs only.
#include <algorithm_assets/numeric/fft.hpp>
#include <algorithm_assets/numeric/compensated_sum.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numbers>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace audio_spectrum_demo {
struct Bin {
    double frequency_hz = 0, peak_tone_amplitude = 0, psd = 0;
};
struct Band {
    double power = 0, rms = 0, peak_bin_amplitude = 0;
    std::size_t bin_count = 0;
};
struct Frame {
    std::uint64_t sample_begin, sample_end; // End exclusive, relative to last reset.
    double center_seconds;
    std::span<const Bin> bins;
    std::span<const Band> bands; // Views overwritten by next emitted frame/reset.
};

class Analyzer {
public:
    // One PCM channel, finite normalized samples in [-1, 1].
    // Edges strictly partition [0, fs/2]; the last band includes Nyquist.
    Analyzer(double sample_rate, std::size_t fft_size, std::size_t hop,
             std::vector<double> band_edges)
        : fs_(checked_rate(sample_rate)), n_(checked_size(fft_size)), hop_(checked_hop(hop, n_)),
          plan_(n_), edges_(std::move(band_edges)), ring_(n_), window_(n_), work_(n_),
          bins_(n_ / 2 + 1), until_frame_(n_) {
        if (edges_.size() < 2 || edges_.front() != 0 || edges_.back() != fs_ / 2)
            throw std::invalid_argument("Band edges must cover [0, sample_rate/2]");
        for (std::size_t i = 0; i < edges_.size(); ++i)
            if (!std::isfinite(edges_[i]) || (i && edges_[i] <= edges_[i - 1]))
                throw std::invalid_argument("Band edges must be finite and strictly increasing");
        algorithm_assets::CompensatedSum window_sum, window_energy;
        for (std::size_t i = 0; i < n_; ++i) {
            window_[i] = 0.5 - 0.5 * std::cos(2 * std::numbers::pi * static_cast<double>(i) /
                                              static_cast<double>(n_));
            window_sum.add(window_[i]);
            window_energy.add(window_[i] * window_[i]);
        }
        window_sum_ = window_sum.value();
        window_energy_ = window_energy.value();
        for (std::size_t k = 0; k < bins_.size(); ++k)
            bins_[k].frequency_hz = static_cast<double>(k) * fs_ / static_cast<double>(n_);
        const auto lower_bin = [&](double hz) {
            return static_cast<std::size_t>(std::lower_bound(bins_.begin(), bins_.end(), hz,
                                                             [](const Bin& bin, double frequency) {
                                                                 return bin.frequency_hz <
                                                                        frequency;
                                                             }) -
                                            bins_.begin());
        };
        ranges_.reserve(edges_.size() - 1);
        bands_.resize(edges_.size() - 1);
        for (std::size_t b = 0; b < bands_.size(); ++b) {
            const auto last = b + 1 == bands_.size() ? bins_.size() : lower_bin(edges_[b + 1]);
            ranges_.push_back({lower_bin(edges_[b]), last});
        }
    }
    Analyzer(const Analyzer&) = delete;
    Analyzer& operator=(const Analyzer&) = delete;

    template <class Emit> void push(std::span<const double> samples, Emit&& emit) {
        // Reject a bad chunk before consuming its prefix.
        for (const auto sample : samples)
            if (!std::isfinite(sample) || std::abs(sample) > 1)
                throw std::invalid_argument("PCM sample must be finite and in [-1,1]");
        if (samples.size() > std::numeric_limits<std::uint64_t>::max() - received_)
            throw std::overflow_error("Audio sample counter exhausted");
        for (const auto sample : samples) {
            ring_[write_] = sample;
            write_ = (write_ + 1) & (n_ - 1);
            ++received_;
            if (--until_frame_ == 0) {
                until_frame_ = hop_;
                analyze_frame();
                const auto start = received_ - n_;
                const Frame frame{start, received_,
                                  (static_cast<double>(start) + 0.5 * static_cast<double>(n_ - 1)) /
                                      fs_,
                                  bins_, bands_};
                std::invoke(emit, frame);
            }
        }
    }
    void reset() {
        write_ = 0;
        received_ = 0;
        until_frame_ = n_;
        std::fill(ring_.begin(), ring_.end(), 0);
        for (auto& bin : bins_) {
            bin.peak_tone_amplitude = 0;
            bin.psd = 0;
        }
        std::fill(bands_.begin(), bands_.end(), Band{});
    }
    [[nodiscard]] std::uint64_t samples_consumed() const noexcept {
        return received_;
    }
    [[nodiscard]] std::span<const double> band_edges() const noexcept {
        return edges_;
    }

private:
    static double checked_rate(double fs) {
        if (!std::isfinite(fs) || fs < 1 || fs > 1000000)
            throw std::invalid_argument("Sample rate must be finite and in [1,1000000] Hz");
        return fs;
    }
    static std::size_t checked_size(std::size_t n) {
        if (n < 4 || n > (std::size_t{1} << 20) || !std::has_single_bit(n))
            throw std::invalid_argument("FFT size must be a power of two in [4,2^20]");
        return n;
    }
    static std::size_t checked_hop(std::size_t hop, std::size_t n) {
        if (hop == 0 || hop > n)
            throw std::invalid_argument("Hop must be in [1,N]");
        return hop;
    }
    void analyze_frame() {
        for (std::size_t i = 0; i < n_; ++i)
            work_[i] = {ring_[(write_ + i) & (n_ - 1)] * window_[i], 0};
        plan_.transform(work_);
        for (std::size_t k = 0; k < bins_.size(); ++k) {
            const auto fold = k == 0 || k == n_ / 2 ? 1.0 : 2.0;
            bins_[k].peak_tone_amplitude = fold * std::abs(work_[k]) / window_sum_;
            bins_[k].psd = fold * std::norm(work_[k]) / (fs_ * window_energy_);
        }
        // Non-overlapping ranges visit each bin exactly once, regardless of B.
        // Direct compensated accumulation avoids subtracting two large prefixes.
        const auto delta_f = fs_ / static_cast<double>(n_);
        for (std::size_t b = 0; b < bands_.size(); ++b) {
            algorithm_assets::CompensatedSum power;
            double peak = 0;
            const auto [first, last] = ranges_[b];
            for (auto k = first; k < last; ++k) {
                power.add(bins_[k].psd * delta_f);
                peak = std::max(peak, bins_[k].peak_tone_amplitude);
            }
            const auto value = power.value();
            bands_[b] = {value, std::sqrt(value), peak, last - first};
        }
    }
    double fs_;
    std::size_t n_, hop_;
    algorithm_assets::Radix2Fft plan_;
    std::vector<double> edges_, ring_, window_;
    std::vector<std::complex<double>> work_;
    std::vector<Bin> bins_;
    std::vector<std::array<std::size_t, 2>> ranges_;
    std::vector<Band> bands_;
    std::size_t write_ = 0, until_frame_;
    std::uint64_t received_ = 0;
    double window_sum_ = 0, window_energy_ = 0;
};
} // namespace audio_spectrum_demo
