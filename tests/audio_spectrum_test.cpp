#include "../examples/audio_spectrum_demo.hpp"
#include <iostream>
#include <random>

using audio_spectrum_demo::Analyzer;
using audio_spectrum_demo::Frame;
void require_audio(bool condition) { if (!condition) throw std::runtime_error("Audio composition check failed"); }
bool near_audio(double a, double b) { return std::abs(a - b) <= 1e-11 + 1e-10 * std::abs(b); }
template<class Function> void rejects_audio(Function function) {
  try { function(); } catch (const std::invalid_argument&) { return; }
  throw std::runtime_error("Invalid audio configuration/input accepted");
}
struct OwnedFrame { std::uint64_t start; std::vector<audio_spectrum_demo::Bin> bins; std::vector<audio_spectrum_demo::Band> bands; };
int main() {
  constexpr std::size_t n = 256, hop = 73;
  constexpr double fs = 48000;
  std::mt19937 rng(1015);
  std::vector<double> samples(n + 8 * hop);
  for (auto& x : samples) x = static_cast<double>(rng() % 2001) / 1000 - 1;
  const std::vector<double> edges{0, 200, 1000, 1001, 6000, fs / 2};
  Analyzer one(fs, n, hop, edges), chunks(fs, n, hop, edges);
  std::vector<OwnedFrame> expected;
  one.push(samples, [&](const Frame& frame) {
    require_audio(frame.sample_end - frame.sample_begin == n);
    require_audio(near_audio(frame.center_seconds, (static_cast<double>(frame.sample_begin) + 127.5) / fs));
    double time_power = 0, energy = 0;
    for (std::size_t i = 0; i < n; ++i) {
      const auto window = 0.5 - 0.5 * std::cos(2 * std::numbers::pi * static_cast<double>(i) / n);
      const auto x = samples[static_cast<std::size_t>(frame.sample_begin) + i];
      time_power += x * x * window * window; energy += window * window;
    }
    double band_power = 0; std::size_t assigned = 0;
    for (const auto& band : frame.bands) { band_power += band.power; assigned += band.bin_count; require_audio(near_audio(band.rms * band.rms, band.power)); }
    require_audio(assigned == n / 2 + 1 && near_audio(band_power, time_power / energy));
    expected.push_back({frame.sample_begin, {frame.bins.begin(), frame.bins.end()}, {frame.bands.begin(), frame.bands.end()}});
  });
  std::size_t observed = 0;
  for (std::size_t offset = 0; offset < samples.size();) {
    const auto count = std::min<std::size_t>(1 + rng() % 200, samples.size() - offset);
    chunks.push(std::span<const double>(samples).subspan(offset, count), [&](const Frame& frame) {
      const auto& reference = expected.at(observed++); require_audio(reference.start == frame.sample_begin);
      for (std::size_t k = 0; k < frame.bins.size(); ++k) {
        require_audio(near_audio(frame.bins[k].psd, reference.bins[k].psd));
        require_audio(near_audio(frame.bins[k].peak_tone_amplitude, reference.bins[k].peak_tone_amplitude));
      }
      for (std::size_t b = 0; b < frame.bands.size(); ++b) require_audio(near_audio(frame.bands[b].power, reference.bands[b].power));
    });
    offset += count;
  }
  require_audio(observed == expected.size() && observed == 9);
  // Full-band power, coherent amplitude, and the one-sided endpoint factors.
  for (int type = 0; type < 4; ++type) {
    std::vector<double> signal(n);
    for (std::size_t i = 0; i < n; ++i) {
      if (type == 0) signal[i] = 0;
      if (type == 1) signal[i] = 0.25;
      if (type == 2) signal[i] = i % 2 ? -0.25 : 0.25;
      if (type == 3) signal[i] = 0.5 * std::sin(2 * std::numbers::pi * 16 * static_cast<double>(i) / n);
    }
    Analyzer analyzer(fs, n, n, {0, fs / 2});
    analyzer.push(signal, [&](const Frame& frame) {
      const auto power = type == 0 ? 0 : type == 3 ? 0.125 : 0.0625;
      require_audio(near_audio(frame.bands[0].power, power));
      const auto k = type == 1 ? 0 : type == 2 ? n / 2 : 16;
      require_audio(near_audio(frame.bins[k].peak_tone_amplitude, type == 0 ? 0 : type == 3 ? 0.5 : 0.25));
    });
  }
  // B > frequency-bin count is allowed, but some reported bands have no bin.
  std::vector<double> narrow(514);
  for (std::size_t i = 0; i < narrow.size(); ++i) narrow[i] = (fs / 2) * static_cast<double>(i) / 513;
  Analyzer many(fs, n, n, narrow);
  many.push(std::vector<double>(n, 0), [&](const Frame& frame) {
    std::size_t empty = 0, total = 0;
    for (const auto& band : frame.bands) { empty += band.bin_count == 0; total += band.bin_count; }
    require_audio(empty > 0 && total == n / 2 + 1);
  });
  Analyzer reset(fs, n, hop, edges);
  const auto ignore = [](const Frame&) {};
  reset.push(std::span<const double>(samples).first(100), ignore);
  rejects_audio([&] { reset.push(std::vector<double>{0.1, std::numeric_limits<double>::quiet_NaN()}, ignore); });
  require_audio(reset.samples_consumed() == 100);
  rejects_audio([&] { reset.push(std::vector<double>{1.01}, ignore); });
  reset.reset(); require_audio(reset.samples_consumed() == 0);
  std::size_t reset_frames = 0;
  reset.push(std::vector<double>(n - 1, 0), [&](const Frame&) { ++reset_frames; });
  require_audio(reset_frames == 0);
  reset.push(std::vector<double>{0}, [&](const Frame& frame) { ++reset_frames; require_audio(frame.sample_begin == 0); });
  require_audio(reset_frames == 1);
  rejects_audio([&] { Analyzer a(0, n, hop, edges); });
  rejects_audio([&] { Analyzer a(fs, 255, hop, edges); });
  rejects_audio([&] { Analyzer a(fs, n, 0, edges); });
  rejects_audio([&] { Analyzer a(fs, n, n + 1, edges); });
  rejects_audio([&] { Analyzer a(fs, n, hop, {0, 1000, 1000, fs / 2}); });
  rejects_audio([&] { Analyzer a(fs, n, hop, {0, 1000}); });
  std::cout << "Audio consumer composition: chunk invariance, Parseval power, DC/Nyquist/amplitude, narrow bands, reset and invalid input passed\n";
}
