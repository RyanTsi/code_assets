#include "audio_spectrum_demo.hpp"
#include <array>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char** argv) try {
  const std::string mode = argc > 1 ? argv[1] : "--demo";
  if ((mode != "--demo" && mode != "--stdin") || (argc != 1 && argc != 2 && argc != 6 && argc != 7))
    throw std::invalid_argument("Usage: audio_spectrum [--demo|--stdin [fs N hop band_count [--bins]]]");
  const double fs = argc >= 6 ? std::stod(argv[2]) : 48000;
  const auto n = argc >= 6 ? static_cast<std::size_t>(std::stoull(argv[3])) : 4096;
  const auto hop = argc >= 6 ? static_cast<std::size_t>(std::stoull(argv[4])) : 1024;
  const auto band_count = argc >= 6 ? static_cast<std::size_t>(std::stoull(argv[5])) : 4;
  const bool dump_bins = argc == 7 && std::string(argv[6]) == "--bins";
  if (argc == 7 && !dump_bins) throw std::invalid_argument("Unknown output option");
  std::vector<double> edges;
  if (band_count == 0 || band_count >= edges.max_size()) throw std::invalid_argument("Invalid band count");
  edges.resize(band_count + 1);
  for (std::size_t b = 0; b <= band_count; ++b) edges[b] = (fs / 2) * (static_cast<double>(b) / static_cast<double>(band_count));
  audio_spectrum_demo::Analyzer analyzer(fs, n, hop, std::move(edges));
  std::cout << std::setprecision(17);
  const auto emit = [&](const audio_spectrum_demo::Frame& frame) {
    std::cout << "{\"sample_begin\":" << frame.sample_begin << ",\"sample_end\":" << frame.sample_end
      << ",\"center_seconds\":" << frame.center_seconds << ",\"bands\":[";
    for (std::size_t b = 0; b < frame.bands.size(); ++b) {
      if (b) std::cout << ',';
      const auto& band = frame.bands[b];
      std::cout << "{\"low_hz\":" << analyzer.band_edges()[b] << ",\"high_hz\":" << analyzer.band_edges()[b + 1]
        << ",\"power\":" << band.power << ",\"rms\":" << band.rms
        << ",\"peak_bin_amplitude\":" << band.peak_bin_amplitude << ",\"bin_count\":" << band.bin_count << '}';
    }
    std::cout << ']';
    if (dump_bins) {
      std::cout << ",\"bins\":[";
      for (std::size_t k = 0; k < frame.bins.size(); ++k) {
        if (k) std::cout << ',';
        const auto& bin = frame.bins[k];
        std::cout << "{\"hz\":" << bin.frequency_hz << ",\"peak_tone_amplitude\":" << bin.peak_tone_amplitude << ",\"psd\":" << bin.psd << '}';
      }
      std::cout << ']';
    }
    std::cout << "}\n";
  };
  if (mode == "--stdin") {
    std::array<double, 256> chunk{};
    std::size_t used = 0;
    while (std::cin >> chunk[used]) {
      if (++used == chunk.size()) { analyzer.push(chunk, emit); used = 0; }
    }
    if (!std::cin.eof()) throw std::invalid_argument("Input must contain whitespace-separated normalized PCM numbers");
    analyzer.push(std::span<const double>(chunk).first(used), emit);
  } else {
    if (n < 16) throw std::invalid_argument("Two-tone demo requires N >= 16");
    std::vector<double> samples(n + 3 * hop);
    for (std::size_t i = 0; i < samples.size(); ++i)
      samples[i] = 0.6 * std::sin(2 * std::numbers::pi * static_cast<double>(i) / 16)
                 + 0.2 * std::sin(6 * std::numbers::pi * static_cast<double>(i) / 16);
    constexpr std::array<std::size_t, 4> sizes{37, 511, 2048, 17};
    for (std::size_t offset = 0, block = 0; offset < samples.size(); ++block) {
      const auto count = std::min(sizes[block % sizes.size()], samples.size() - offset);
      analyzer.push(std::span<const double>(samples).subspan(offset, count), emit);
      offset += count;
    }
  }
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n'; return 1;
}
