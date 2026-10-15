"""Optional consumer-level oracle: official SciPy periodogram semantics, no asset source inspection."""
import json
import subprocess
import sys
from pathlib import Path

import numpy as np
import scipy
from scipy import signal

executable = Path(sys.argv[1]).resolve()
rng = np.random.default_rng(1015)
frames_checked = 0
maximum_error = 0.0
for case in range(60):
    n = [32, 64, 128, 256, 512][case % 5]
    fs = [48000, 44100][case % 2]
    hop = [1, n // 4, n // 2, n][case % 4]
    count = [1, 3, 8, n + 3][case % 4]
    length = n + 3 * hop + int(rng.integers(hop))
    samples = rng.uniform(-1, 1, length)
    if case % 3 == 0:
        index = np.arange(length)
        samples = 0.6 * np.sin(2 * np.pi * 3.37 * index / n) + 0.2 * np.cos(2 * np.pi * 8.13 * index / n)
    if case % 10 == 0:
        samples.fill(0); samples[n // 2] = 0.75
    process = subprocess.run([str(executable), '--stdin', str(fs), str(n), str(hop), str(count), '--bins'],
        input=' '.join(format(float(x), '.17g') for x in samples), capture_output=True, text=True, check=True, timeout=15)
    frames = [json.loads(line) for line in process.stdout.splitlines()]
    assert len(frames) == 1 + (length - n) // hop
    window = signal.windows.hann(n, sym=False)
    for number, frame in enumerate(frames):
        start = number * hop
        assert frame['sample_begin'] == start and frame['sample_end'] == start + n
        frequencies, psd = signal.periodogram(samples[start:start + n], fs=fs, window=window,
            detrend=False, return_onesided=True, scaling='density', nfft=n)
        actual = np.array([b['psd'] for b in frame['bins']])
        maximum_error = max(maximum_error, float(np.max(np.abs(actual - psd))))
        np.testing.assert_allclose(actual, psd, rtol=3e-10, atol=2e-14)
        np.testing.assert_allclose([b['hz'] for b in frame['bins']], frequencies, rtol=1e-14, atol=1e-12)
        coherent_amplitude = np.abs(np.fft.rfft(samples[start:start + n] * window)) / np.sum(window)
        coherent_amplitude[1:-1] *= 2
        np.testing.assert_allclose([b['peak_tone_amplitude'] for b in frame['bins']], coherent_amplitude,
            rtol=3e-10, atol=2e-13)
        for band_index, band in enumerate(frame['bands']):
            selected = (frequencies >= band['low_hz']) & (frequencies < band['high_hz'])
            if band_index == len(frame['bands']) - 1:
                selected[-1] = True
            expected_power = float(np.sum(psd[selected]) * fs / n)
            assert band['bin_count'] == int(np.count_nonzero(selected))
            np.testing.assert_allclose(band['power'], expected_power, rtol=3e-10, atol=2e-13)
            np.testing.assert_allclose(band['rms'], np.sqrt(expected_power), rtol=3e-10, atol=2e-13)
        frames_checked += 1
report = {'numpy': np.__version__, 'scipy': scipy.__version__, 'cases': 60,
          'frames': frames_checked, 'maximum_absolute_psd_error': maximum_error,
          'tolerance': {'rtol': 3e-10, 'psd_atol': 2e-14},
          'scope': 'Consumer composition for bounded normalized synthetic PCM; not live-device latency validation'}
print(json.dumps(report, indent=2))
if len(sys.argv) > 2:
    Path(sys.argv[2]).write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
