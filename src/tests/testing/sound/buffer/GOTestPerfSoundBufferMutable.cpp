/*
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundBufferMutable.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "sound/buffer/GOSoundBufferMutable.h"

const std::string GOTestPerfSoundBufferMutable::TEST_NAME
  = "GOTestPerfSoundBufferMutable";

// Buffer sizes to test (in samples)
static constexpr unsigned BUFFER_SIZES[] = {32, 128, 512, 2048};
static constexpr unsigned NUM_BUFFER_SIZES
  = sizeof(BUFFER_SIZES) / sizeof(BUFFER_SIZES[0]);

// Number of channels (stereo)
static constexpr unsigned NUM_CHANNELS = 2;

// Number of iterations for performance tests
static constexpr unsigned NUM_ITERATIONS = 1000000;

// Baseline performance in millions of samples per second
struct Baseline {
  unsigned m_BufferSize;
  double m_MSamplesPerSecond;
};

// Baseline values for each function and buffer size
// Format: {buffer_size, min_MSamples_per_second}
// These are conservative estimates that should pass on most modern hardware
static constexpr Baseline BASELINE_FILL_WITH_SILENCE[] = {
#ifdef NDEBUG
  {32, 100},   // 100 Msamples/sec
  {128, 300},  // 300 Msamples/sec
  {512, 800},  // 800 Msamples/sec
  {2048, 2000} // 2000 Msamples/sec
#else
  {32, 10},   // 10 Msamples/sec (debug)
  {128, 30},  // 30 Msamples/sec (debug)
  {512, 80},  // 80 Msamples/sec (debug)
  {2048, 200} // 200 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 50},    // 50 Msamples/sec
  {128, 200},  // 200 Msamples/sec
  {512, 500},  // 500 Msamples/sec
  {2048, 1000} // 1000 Msamples/sec
#else
  {32, 5},    // 5 Msamples/sec (debug)
  {128, 20},  // 20 Msamples/sec (debug)
  {512, 50},  // 50 Msamples/sec (debug)
  {2048, 100} // 100 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 50},   // 50 Msamples/sec
  {128, 200}, // 200 Msamples/sec
  {512, 400}, // 400 Msamples/sec
  {2048, 800} // 800 Msamples/sec
#else
  {32, 5},    // 5 Msamples/sec (debug)
  {128, 20},  // 20 Msamples/sec (debug)
  {512, 40},  // 40 Msamples/sec (debug)
  {2048, 80}  // 80 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 30},   // 30 Msamples/sec
  {128, 100}, // 100 Msamples/sec
  {512, 300}, // 300 Msamples/sec
  {2048, 600} // 600 Msamples/sec
#else
  {32, 3},    // 3 Msamples/sec (debug)
  {128, 10},  // 10 Msamples/sec (debug)
  {512, 30},  // 30 Msamples/sec (debug)
  {2048, 60}  // 60 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 20},   // 20 Msamples/sec
  {128, 80},  // 80 Msamples/sec
  {512, 200}, // 200 Msamples/sec
  {2048, 500} // 500 Msamples/sec
#else
  {32, 2},    // 2 Msamples/sec (debug)
  {128, 8},   // 8 Msamples/sec (debug)
  {512, 20},  // 20 Msamples/sec (debug)
  {2048, 50}  // 50 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 20},   // 20 Msamples/sec
  {128, 80},  // 80 Msamples/sec
  {512, 200}, // 200 Msamples/sec
  {2048, 400} // 400 Msamples/sec
#else
  {32, 2},    // 2 Msamples/sec (debug)
  {128, 8},   // 8 Msamples/sec (debug)
  {512, 20},  // 20 Msamples/sec (debug)
  {2048, 40}  // 40 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 15},   // 15 Msamples/sec
  {128, 60},  // 60 Msamples/sec
  {512, 150}, // 150 Msamples/sec
  {2048, 300} // 300 Msamples/sec
#else
  {32, 1},    // 1 Msamples/sec (debug)
  {128, 6},   // 6 Msamples/sec (debug)
  {512, 15},  // 15 Msamples/sec (debug)
  {2048, 30}  // 30 Msamples/sec (debug)
#endif
};

// Helper function to fill buffer with sine wave signal
// Each channel gets a different frequency to make data more realistic
static void fill_with_sine_wave(
  std::vector<GOSoundBuffer::SoundUnit> &data,
  unsigned nChannels,
  unsigned nSamples) {
  constexpr double PI = 3.14159265358979323846;
  constexpr double baseFrequency = 440.0; // A4 note
  constexpr double sampleRate = 48000.0;

  for (unsigned sample = 0; sample < nSamples; ++sample) {
    for (unsigned channel = 0; channel < nChannels; ++channel) {
      // Different frequency for each channel
      double frequency = baseFrequency * (channel + 1);
      double phase = 2.0 * PI * frequency * sample / sampleRate;
      data[sample * nChannels + channel] = static_cast<float>(std::sin(phase));
    }
  }
}

// Helper function to find baseline for a given buffer size
static const Baseline *get_baseline(
  const Baseline *baselines, unsigned bufferSize) {
  const Baseline *result = nullptr;

  for (unsigned i = 0; i < NUM_BUFFER_SIZES; ++i) {
    if (baselines[i].m_BufferSize == bufferSize) {
      result = &baselines[i];
      break;
    }
  }

  return result;
}

// Helper function to measure performance
// Returns performance in millions of samples per second
static double measure_performance(
  unsigned bufferSize,
  unsigned numIterations,
  std::function<void()> operation) {
  auto start = std::chrono::high_resolution_clock::now();

  for (unsigned i = 0; i < numIterations; ++i) {
    operation();
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  // Calculate millions of samples per second
  double totalSamples = static_cast<double>(bufferSize) * numIterations;
  return (totalSamples / elapsed.count()) / 1e6;
}

void GOTestPerfSoundBufferMutable::RunAndEvaluateTest(
  const std::string &functionName,
  const Baseline &baseline,
  std::function<void()> operation) {
  double mSamplesPerSecond
    = measure_performance(baseline.m_BufferSize, NUM_ITERATIONS, operation);
  bool passed = mSamplesPerSecond >= baseline.m_MSamplesPerSecond;
  double ratio = mSamplesPerSecond / baseline.m_MSamplesPerSecond;

#ifdef NDEBUG
  const char *buildMode = "Release";
#else
  const char *buildMode = "Debug  ";
#endif

  std::string message = std::format(
    "{:<7} {:<20} (size={:4}): {:8.1f} Msamples/sec (baseline: {:8.1f}, "
    "ratio: {:5.2f}x)",
    buildMode,
    functionName,
    baseline.m_BufferSize,
    mSamplesPerSecond,
    baseline.m_MSamplesPerSecond,
    ratio);

  const char *status = passed ? "PASS" : "FAIL";
  std::cout << std::format("  [{}] {}\n", status, message);

  if (!passed) {
    m_failedTests.push_back(message);
  }
}

void GOTestPerfSoundBufferMutable::TestPerfFillWithSilence() {
  std::cout << "\nPerformance test: FillWithSilence\n";

  for (unsigned bufSize : BUFFER_SIZES) {
    const unsigned totalUnits = NUM_CHANNELS * bufSize;
    std::vector<GOSoundBuffer::SoundUnit> data(totalUnits);

    fill_with_sine_wave(data, NUM_CHANNELS, bufSize);

    GOSoundBufferMutable buffer(data.data(), NUM_CHANNELS, bufSize);
    const Baseline *baseline
      = get_baseline(BASELINE_FILL_WITH_SILENCE, bufSize);

    auto operation = [&buffer]() { buffer.FillWithSilence(); };

    RunAndEvaluateTest("FillWithSilence", *baseline, operation);
  }
}

void GOTestPerfSoundBufferMutable::TestPerfCopyFrom() {
  std::cout << "\nPerformance test: CopyFrom\n";

  for (unsigned bufSize : BUFFER_SIZES) {
    const unsigned totalUnits = NUM_CHANNELS * bufSize;
    std::vector<GOSoundBuffer::SoundUnit> srcData(totalUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(totalUnits);

    fill_with_sine_wave(srcData, NUM_CHANNELS, bufSize);
    std::fill(dstData.begin(), dstData.end(), 0.0f);

    GOSoundBuffer srcBuffer(srcData.data(), NUM_CHANNELS, bufSize);
    GOSoundBufferMutable dstBuffer(dstData.data(), NUM_CHANNELS, bufSize);
    const Baseline *baseline = get_baseline(BASELINE_COPY_FROM, bufSize);

    auto operation
      = [&dstBuffer, &srcBuffer]() { dstBuffer.CopyFrom(srcBuffer); };

    RunAndEvaluateTest("CopyFrom", *baseline, operation);
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddFrom() {
  std::cout << "\nPerformance test: AddFrom\n";

  for (unsigned bufSize : BUFFER_SIZES) {
    const unsigned totalUnits = NUM_CHANNELS * bufSize;
    std::vector<GOSoundBuffer::SoundUnit> srcData(totalUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(totalUnits);

    fill_with_sine_wave(srcData, NUM_CHANNELS, bufSize);
    fill_with_sine_wave(dstData, NUM_CHANNELS, bufSize);

    GOSoundBuffer srcBuffer(srcData.data(), NUM_CHANNELS, bufSize);
    GOSoundBufferMutable dstBuffer(dstData.data(), NUM_CHANNELS, bufSize);
    const Baseline *baseline = get_baseline(BASELINE_ADD_FROM, bufSize);

    auto operation
      = [&dstBuffer, &srcBuffer]() { dstBuffer.AddFrom(srcBuffer); };

    RunAndEvaluateTest("AddFrom", *baseline, operation);
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddFromWithCoefficient() {
  std::cout << "\nPerformance test: AddFrom (with coefficient)\n";

  for (unsigned bufSize : BUFFER_SIZES) {
    const unsigned totalUnits = NUM_CHANNELS * bufSize;
    std::vector<GOSoundBuffer::SoundUnit> srcData(totalUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(totalUnits);

    fill_with_sine_wave(srcData, NUM_CHANNELS, bufSize);
    fill_with_sine_wave(dstData, NUM_CHANNELS, bufSize);

    GOSoundBuffer srcBuffer(srcData.data(), NUM_CHANNELS, bufSize);
    GOSoundBufferMutable dstBuffer(dstData.data(), NUM_CHANNELS, bufSize);
    const Baseline *baseline = get_baseline(BASELINE_ADD_FROM_COEFF, bufSize);

    const float coeff = 0.5f;
    auto operation = [&dstBuffer, &srcBuffer, coeff]() {
      dstBuffer.AddFrom(srcBuffer, coeff);
    };

    RunAndEvaluateTest("AddFrom+coeff", *baseline, operation);
  }
}

void GOTestPerfSoundBufferMutable::TestPerfCopyChannelFrom() {
  std::cout << "\nPerformance test: CopyChannelFrom\n";

  for (unsigned bufSize : BUFFER_SIZES) {
    const unsigned totalUnits = NUM_CHANNELS * bufSize;
    std::vector<GOSoundBuffer::SoundUnit> srcData(totalUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(totalUnits);

    fill_with_sine_wave(srcData, NUM_CHANNELS, bufSize);
    std::fill(dstData.begin(), dstData.end(), 0.0f);

    GOSoundBuffer srcBuffer(srcData.data(), NUM_CHANNELS, bufSize);
    GOSoundBufferMutable dstBuffer(dstData.data(), NUM_CHANNELS, bufSize);
    const Baseline *baseline
      = get_baseline(BASELINE_COPY_CHANNEL_FROM, bufSize);

    auto operation = [&dstBuffer, &srcBuffer]() {
      dstBuffer.CopyChannelFrom(srcBuffer, 0, 1);
    };

    RunAndEvaluateTest("CopyChannelFrom", *baseline, operation);
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddChannelFrom() {
  std::cout << "\nPerformance test: AddChannelFrom\n";

  for (unsigned bufSize : BUFFER_SIZES) {
    const unsigned totalUnits = NUM_CHANNELS * bufSize;
    std::vector<GOSoundBuffer::SoundUnit> srcData(totalUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(totalUnits);

    fill_with_sine_wave(srcData, NUM_CHANNELS, bufSize);
    fill_with_sine_wave(dstData, NUM_CHANNELS, bufSize);

    GOSoundBuffer srcBuffer(srcData.data(), NUM_CHANNELS, bufSize);
    GOSoundBufferMutable dstBuffer(dstData.data(), NUM_CHANNELS, bufSize);
    const Baseline *baseline = get_baseline(BASELINE_ADD_CHANNEL_FROM, bufSize);

    auto operation = [&dstBuffer, &srcBuffer]() {
      dstBuffer.AddChannelFrom(srcBuffer, 0, 1);
    };

    RunAndEvaluateTest("AddChannelFrom", *baseline, operation);
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddChannelFromWithCoefficient() {
  std::cout << "\nPerformance test: AddChannelFrom (with coefficient)\n";

  for (unsigned bufSize : BUFFER_SIZES) {
    const unsigned totalUnits = NUM_CHANNELS * bufSize;
    std::vector<GOSoundBuffer::SoundUnit> srcData(totalUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(totalUnits);

    fill_with_sine_wave(srcData, NUM_CHANNELS, bufSize);
    fill_with_sine_wave(dstData, NUM_CHANNELS, bufSize);

    GOSoundBuffer srcBuffer(srcData.data(), NUM_CHANNELS, bufSize);
    GOSoundBufferMutable dstBuffer(dstData.data(), NUM_CHANNELS, bufSize);
    const Baseline *baseline
      = get_baseline(BASELINE_ADD_CHANNEL_FROM_COEFF, bufSize);

    const float coeff = 0.5f;
    auto operation = [&dstBuffer, &srcBuffer, coeff]() {
      dstBuffer.AddChannelFrom(srcBuffer, 0, 1, coeff);
    };

    RunAndEvaluateTest("AddChannelFrom+coeff", *baseline, operation);
  }
}

void GOTestPerfSoundBufferMutable::run() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundBufferMutable "
               "==========\n";
#ifdef NDEBUG
  std::cout << "Build mode: Release\n";
#else
  std::cout << "Build mode: Debug\n";
#endif
  std::cout << "Testing with " << NUM_ITERATIONS
            << " iterations per buffer size\n";
  std::cout << "Buffer configuration: " << NUM_CHANNELS
            << " channels (stereo)\n";

  TestPerfFillWithSilence();
  TestPerfCopyFrom();
  TestPerfAddFrom();
  TestPerfAddFromWithCoefficient();
  TestPerfCopyChannelFrom();
  TestPerfAddChannelFrom();
  TestPerfAddChannelFromWithCoefficient();

  std::cout << "\n========== Performance Tests Completed ==========\n";

  // Report all failures at the end
  if (!m_failedTests.empty()) {
    std::string errorMsg
      = std::format("{} performance test(s) failed:\n", m_failedTests.size());
    for (const auto &failedTest : m_failedTests) {
      errorMsg += "  - " + failedTest + "\n";
    }
    GOAssert(false, errorMsg);
  }
}
