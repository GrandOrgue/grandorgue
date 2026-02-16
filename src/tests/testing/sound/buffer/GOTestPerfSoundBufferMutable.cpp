/*
 * Copyright 2006 Milan Digital Audio LLC
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
// Baseline values updated based on actual performance measurements
// from Intel i7-8700K (bare-metal) and AMD EPYC 7763 (Azure VM)
static constexpr Baseline BASELINE_FILL_WITH_SILENCE[] = {
#ifdef NDEBUG
  {32, 7500},   // 7500 Msamples/sec (raised for modern hardware)
  {128, 9500},  // 9500 Msamples/sec (raised for modern hardware)
  {512, 7500},  // 7500 Msamples/sec (raised for modern hardware)
  {2048, 10000} // 10000 Msamples/sec (raised for modern hardware)
#else
  {32, 160},   // 160 Msamples/sec (debug, raised for modern hardware)
  {128, 600},  // 600 Msamples/sec (debug, raised for modern hardware)
  {512, 2100}, // 2100 Msamples/sec (debug, raised for modern hardware)
  {2048, 6000} // 6000 Msamples/sec (debug, raised for modern hardware)
#endif
};

// Note: Large memcpy operations (512+ samples) show significant overhead
// in Azure VM environments (~25x slower) due to hypervisor optimizations.
// Baselines for large buffers are set conservatively to pass on both
// bare-metal and virtualized environments.
static constexpr Baseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 7000},  // 7000 Msamples/sec (raised for modern hardware)
  {128, 9000}, // 9000 Msamples/sec (raised for modern hardware)
  {512, 350},  // 350 Msamples/sec (lowered for Azure VM compatibility)
  {2048, 350}  // 350 Msamples/sec (lowered for Azure VM compatibility)
#else
  {32, 140},   // 140 Msamples/sec (debug, raised for modern hardware)
  {128, 550},  // 550 Msamples/sec (debug, raised for modern hardware)
  {512, 300},  // 300 Msamples/sec (debug, lowered for Azure VM compatibility)
  {2048, 300}  // 300 Msamples/sec (debug, lowered for Azure VM compatibility)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 3500},  // 3500 Msamples/sec (raised for modern hardware)
  {128, 4500}, // 4500 Msamples/sec (raised for modern hardware)
  {512, 4500}, // 4500 Msamples/sec (raised for modern hardware)
  {2048, 4500} // 4500 Msamples/sec (raised for modern hardware)
#else
  {32, 50},    // 50 Msamples/sec (debug, raised for modern hardware)
  {128, 70},   // 70 Msamples/sec (debug, raised for modern hardware)
  {512, 80},   // 80 Msamples/sec (debug, raised for modern hardware)
  {2048, 80}   // 80 Msamples/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 3200},  // 3200 Msamples/sec (raised for modern hardware)
  {128, 4400}, // 4400 Msamples/sec (raised for modern hardware)
  {512, 4300}, // 4300 Msamples/sec (raised for modern hardware)
  {2048, 4300} // 4300 Msamples/sec (raised for modern hardware)
#else
  {32, 50},    // 50 Msamples/sec (debug, raised for modern hardware)
  {128, 70},   // 70 Msamples/sec (debug, raised for modern hardware)
  {512, 80},   // 80 Msamples/sec (debug, raised for modern hardware)
  {2048, 80}   // 80 Msamples/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 2400},  // 2400 Msamples/sec (raised for modern hardware)
  {128, 2700}, // 2700 Msamples/sec (raised for modern hardware)
  {512, 2800}, // 2800 Msamples/sec (raised for modern hardware)
  {2048, 2800} // 2800 Msamples/sec (raised for modern hardware)
#else
  {32, 80},    // 80 Msamples/sec (debug, raised for modern hardware)
  {128, 130},  // 130 Msamples/sec (debug, raised for modern hardware)
  {512, 150},  // 150 Msamples/sec (debug, raised for modern hardware)
  {2048, 160}  // 160 Msamples/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 2200},  // 2200 Msamples/sec (raised for modern hardware)
  {128, 2600}, // 2600 Msamples/sec (raised for modern hardware)
  {512, 2700}, // 2700 Msamples/sec (raised for modern hardware)
  {2048, 2700} // 2700 Msamples/sec (raised for modern hardware)
#else
  {32, 80},    // 80 Msamples/sec (debug, raised for modern hardware)
  {128, 130},  // 130 Msamples/sec (debug, raised for modern hardware)
  {512, 150},  // 150 Msamples/sec (debug, raised for modern hardware)
  {2048, 160}  // 160 Msamples/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 2100},  // 2100 Msamples/sec (raised for modern hardware)
  {128, 2500}, // 2500 Msamples/sec (raised for modern hardware)
  {512, 2400}, // 2400 Msamples/sec (raised for modern hardware)
  {2048, 2500} // 2500 Msamples/sec (raised for modern hardware)
#else
  {32, 80},    // 80 Msamples/sec (debug, raised for modern hardware)
  {128, 130},  // 130 Msamples/sec (debug, raised for modern hardware)
  {512, 150},  // 150 Msamples/sec (debug, raised for modern hardware)
  {2048, 160}  // 160 Msamples/sec (debug, raised for modern hardware)
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
    // Use macro to declare local buffer on stack
    GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, NUM_CHANNELS, baseline.m_BufferSize);

    fill_with_sine_wave(buffer);

    RunAndEvaluateTest(
      "FillWithSilence", baseline, [&buffer]() { buffer.FillWithSilence(); });
  }
}

void GOTestPerfSoundBufferMutable::TestPerfCopyFrom() {
  std::cout << "\nPerformance test: CopyFrom\n";

  for (const Baseline &baseline : BASELINE_COPY_FROM) {
    // Declare source and destination buffers using macro
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize);

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
    // Declare source and destination buffers using macro
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize);

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
    // Declare source and destination buffers using macro
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize);

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
    // Declare source and destination buffers using macro
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize);

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
    // Declare source and destination buffers using macro
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize);

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
    // Declare source and destination buffers using macro
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize);
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize);

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