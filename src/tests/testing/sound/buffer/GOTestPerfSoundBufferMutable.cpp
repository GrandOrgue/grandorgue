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
  {32, 100},   // 100 Msamples/sec (debug)
  {128, 400},  // 400 Msamples/sec (debug)
  {512, 1200}, // 1200 Msamples/sec (debug)
  {2048, 3000} // 3000 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 50},    // 50 Msamples/sec
  {128, 200},  // 200 Msamples/sec
  {512, 500},  // 500 Msamples/sec
  {2048, 1000} // 1000 Msamples/sec
#else
  {32, 80},    // 80 Msamples/sec (debug)
  {128, 330},  // 330 Msamples/sec (debug)
  {512, 1200}, // 1200 Msamples/sec (debug)
  {2048, 2300} // 2300 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 50},   // 50 Msamples/sec
  {128, 200}, // 200 Msamples/sec
  {512, 400}, // 400 Msamples/sec
  {2048, 800} // 800 Msamples/sec
#else
  {32, 30},    // 30 Msamples/sec (debug)
  {128, 40},   // 40 Msamples/sec (debug)
  {512, 45},   // 45 Msamples/sec (debug)
  {2048, 50}   // 50 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 30},   // 30 Msamples/sec
  {128, 100}, // 100 Msamples/sec
  {512, 300}, // 300 Msamples/sec
  {2048, 600} // 600 Msamples/sec
#else
  {32, 30},    // 30 Msamples/sec (debug)
  {128, 40},   // 40 Msamples/sec (debug)
  {512, 45},   // 45 Msamples/sec (debug)
  {2048, 50}   // 50 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 20},   // 20 Msamples/sec
  {128, 80},  // 80 Msamples/sec
  {512, 200}, // 200 Msamples/sec
  {2048, 500} // 500 Msamples/sec
#else
  {32, 50},    // 50 Msamples/sec (debug)
  {128, 80},   // 80 Msamples/sec (debug)
  {512, 90},   // 90 Msamples/sec (debug)
  {2048, 100}  // 100 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 20},   // 20 Msamples/sec
  {128, 80},  // 80 Msamples/sec
  {512, 200}, // 200 Msamples/sec
  {2048, 400} // 400 Msamples/sec
#else
  {32, 50},    // 50 Msamples/sec (debug)
  {128, 80},   // 80 Msamples/sec (debug)
  {512, 90},   // 90 Msamples/sec (debug)
  {2048, 100}  // 100 Msamples/sec (debug)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 15},   // 15 Msamples/sec
  {128, 60},  // 60 Msamples/sec
  {512, 150}, // 150 Msamples/sec
  {2048, 300} // 300 Msamples/sec
#else
  {32, 50},    // 50 Msamples/sec (debug)
  {128, 80},   // 80 Msamples/sec (debug)
  {512, 90},   // 90 Msamples/sec (debug)
  {2048, 100}  // 100 Msamples/sec (debug)
#endif
};

// Helper function to fill buffer with sine wave signal
// Each channel gets a different frequency to make data more realistic
static void fill_with_sine_wave(GOSoundBufferMutable &buffer) {
  constexpr double PI = 3.14159265358979323846;
  constexpr double baseFrequency = 440.0; // A4 note
  constexpr double sampleRate = 48000.0;

  const unsigned nChannels = buffer.GetNChannels();
  const unsigned nSamples = buffer.GetNSamples();
  GOSoundBuffer::SoundUnit *pData = buffer.GetData();

  for (unsigned sampleI = 0; sampleI < nSamples; ++sampleI) {
    for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
      // Different frequency for each channel
      double frequency = baseFrequency * (channelI + 1);
      double phase = 2.0 * PI * frequency * sampleI / sampleRate;

      *pData++ = static_cast<float>(std::sin(phase));
    }
  }
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

  for (const Baseline &baseline : BASELINE_FILL_WITH_SILENCE) {
    std::vector<GOSoundBuffer::SoundUnit> data(
      GOSoundBuffer::getNUnits(NUM_CHANNELS, baseline.m_BufferSize));
    GOSoundBufferMutable buffer(
      data.data(), NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(buffer);

    RunAndEvaluateTest(
      "FillWithSilence", baseline, [&buffer]() { buffer.FillWithSilence(); });
  }
}

void GOTestPerfSoundBufferMutable::TestPerfCopyFrom() {
  std::cout << "\nPerformance test: CopyFrom\n";

  for (const Baseline &baseline : BASELINE_COPY_FROM) {
    const unsigned nUnits
      = GOSoundBuffer::getNUnits(NUM_CHANNELS, baseline.m_BufferSize);
    std::vector<GOSoundBuffer::SoundUnit> srcData(nUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(nUnits);
    GOSoundBufferMutable srcBuffer(
      srcData.data(), NUM_CHANNELS, baseline.m_BufferSize);
    GOSoundBufferMutable dstBuffer(
      dstData.data(), NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(srcBuffer);
    dstBuffer.FillWithSilence();

    RunAndEvaluateTest("CopyFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.CopyFrom(srcBuffer);
    });
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddFrom() {
  std::cout << "\nPerformance test: AddFrom\n";

  for (const Baseline &baseline : BASELINE_ADD_FROM) {
    const unsigned nUnits
      = GOSoundBuffer::getNUnits(NUM_CHANNELS, baseline.m_BufferSize);
    std::vector<GOSoundBuffer::SoundUnit> srcData(nUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(nUnits);
    GOSoundBufferMutable srcBuffer(
      srcData.data(), NUM_CHANNELS, baseline.m_BufferSize);
    GOSoundBufferMutable dstBuffer(
      dstData.data(), NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstBuffer);

    RunAndEvaluateTest("AddFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.AddFrom(srcBuffer);
    });
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddFromWithCoefficient() {
  std::cout << "\nPerformance test: AddFrom (with coefficient)\n";

  for (const Baseline &baseline : BASELINE_ADD_FROM_COEFF) {
    const unsigned nUnits
      = GOSoundBuffer::getNUnits(NUM_CHANNELS, baseline.m_BufferSize);
    std::vector<GOSoundBuffer::SoundUnit> srcData(nUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(nUnits);
    GOSoundBufferMutable srcBuffer(
      srcData.data(), NUM_CHANNELS, baseline.m_BufferSize);
    GOSoundBufferMutable dstBuffer(
      dstData.data(), NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstBuffer);

    constexpr float coeff = 0.5f;

    RunAndEvaluateTest("AddFrom+coeff", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.AddFrom(srcBuffer, coeff);
    });
  }
}

void GOTestPerfSoundBufferMutable::TestPerfCopyChannelFrom() {
  std::cout << "\nPerformance test: CopyChannelFrom\n";

  for (const Baseline &baseline : BASELINE_COPY_CHANNEL_FROM) {
    const unsigned nUnits
      = GOSoundBuffer::getNUnits(NUM_CHANNELS, baseline.m_BufferSize);
    std::vector<GOSoundBuffer::SoundUnit> srcData(nUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(nUnits);
    GOSoundBufferMutable srcBuffer(
      srcData.data(), NUM_CHANNELS, baseline.m_BufferSize);
    GOSoundBufferMutable dstBuffer(
      dstData.data(), NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(srcBuffer);
    dstBuffer.FillWithSilence();

    RunAndEvaluateTest("CopyChannelFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.CopyChannelFrom(srcBuffer, 0, 1);
    });
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddChannelFrom() {
  std::cout << "\nPerformance test: AddChannelFrom\n";

  for (const Baseline &baseline : BASELINE_ADD_CHANNEL_FROM) {
    const unsigned nUnits
      = GOSoundBuffer::getNUnits(NUM_CHANNELS, baseline.m_BufferSize);
    std::vector<GOSoundBuffer::SoundUnit> srcData(nUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(nUnits);
    GOSoundBufferMutable srcBuffer(
      srcData.data(), NUM_CHANNELS, baseline.m_BufferSize);
    GOSoundBufferMutable dstBuffer(
      dstData.data(), NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstBuffer);

    RunAndEvaluateTest("AddChannelFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.AddChannelFrom(srcBuffer, 0, 1);
    });
  }
}

void GOTestPerfSoundBufferMutable::TestPerfAddChannelFromWithCoefficient() {
  std::cout << "\nPerformance test: AddChannelFrom (with coefficient)\n";

  for (const Baseline &baseline : BASELINE_ADD_CHANNEL_FROM_COEFF) {
    const unsigned nUnits
      = GOSoundBuffer::getNUnits(NUM_CHANNELS, baseline.m_BufferSize);
    std::vector<GOSoundBuffer::SoundUnit> srcData(nUnits);
    std::vector<GOSoundBuffer::SoundUnit> dstData(nUnits);
    GOSoundBufferMutable srcBuffer(
      srcData.data(), NUM_CHANNELS, baseline.m_BufferSize);
    GOSoundBufferMutable dstBuffer(
      dstData.data(), NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstBuffer);

    constexpr float coeff = 0.5f;

    RunAndEvaluateTest(
      "AddChannelFrom+coeff", baseline, [&dstBuffer, &srcBuffer]() {
        dstBuffer.AddChannelFrom(srcBuffer, 0, 1, coeff);
      });
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
