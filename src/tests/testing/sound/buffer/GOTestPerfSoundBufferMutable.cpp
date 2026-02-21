/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundBufferMutable.h"

#include <chrono>
#include <cmath>
#include <format>
#include <functional>
#include <iostream>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/buffer/GOSoundBufferMutableMono.h"

const std::string GOTestPerfSoundBufferMutable::TEST_NAME
  = "GOTestPerfSoundBufferMutable";

// Number of channels (stereo)
static constexpr unsigned NUM_CHANNELS = 2;

// Number of iterations for performance tests
static constexpr unsigned NUM_ITERATIONS = 1000000;

// Baseline performance in millions of frames per second
struct Baseline {
  unsigned m_BufferSize;
  double m_MFramesPerSecond;
};

// Baseline values for each function and buffer size
// Format: {buffer_size, min_MFrames_per_second}
// Baseline values updated based on actual performance measurements
// from Intel i7-8700K (bare-metal) and AMD EPYC 7763 (Azure VM).
// Baselines are set ~10% below the minimum observed value across all CI runs.
static constexpr Baseline BASELINE_FILL_WITH_SILENCE[] = {
#ifdef NDEBUG
  {32, 7500},   // 7500 Mframes/sec (raised for modern hardware)
  {128, 5900},  // 5900 Mframes/sec (measured: 6636.1, -10% margin)
  {512, 5500},  // 5500 Mframes/sec (lowered: min observed 6112.9, -10% margin)
  {2048, 10000} // 10000 Mframes/sec (raised for modern hardware)
#else
  {32, 160},   // 160 Mframes/sec (debug, raised for modern hardware)
  {128, 600},  // 600 Mframes/sec (debug, raised for modern hardware)
  {512, 2100}, // 2100 Mframes/sec (debug, raised for modern hardware)
  {2048, 6000} // 6000 Mframes/sec (debug, raised for modern hardware)
#endif
};

// Note: Large memcpy operations (512+ frames) show significant overhead
// in Azure VM environments (~25x slower) due to hypervisor optimizations.
// Baselines for large buffers are set conservatively to pass on both
// bare-metal and virtualized environments.
static constexpr Baseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 5500},  // 5500 Mframes/sec (lowered: min observed 6285.4, -10% margin)
  {128, 7500}, // 7500 Mframes/sec (lowered: min observed 8511.4, -10% margin)
  {512, 220},  // 220 Mframes/sec (lowered: min observed 255.5, -10% margin)
  {2048, 220}  // 220 Mframes/sec (lowered for Azure VM compatibility)
#else
  {32, 140},   // 140 Mframes/sec (debug, raised for modern hardware)
  {128, 550},  // 550 Mframes/sec (debug, raised for modern hardware)
  {512,
   220}, // 220 Mframes/sec (debug, lowered: min observed 255.0, -10% margin)
  {2048, 220} // 220 Mframes/sec (debug, lowered for Azure VM compatibility)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 3100},  // 3100 Mframes/sec (measured: 3492.3, with 10% margin)
  {128, 4000}, // 4000 Mframes/sec (lowered: min observed 4485.6, -10% margin)
  {512, 4000}, // 4000 Mframes/sec (measured: 4476.4, with 10% margin)
  {2048, 3400} // 3400 Mframes/sec (lowered: min observed 3861.3, -10% margin)
#else
  {32, 50},   // 50 Mframes/sec (debug, raised for modern hardware)
  {128, 70},  // 70 Mframes/sec (debug, raised for modern hardware)
  {512, 80},  // 80 Mframes/sec (debug, raised for modern hardware)
  {2048, 80}  // 80 Mframes/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 2800},  // 2800 Mframes/sec (lowered: min observed 3136.9, -10% margin)
  {128, 3200}, // 3200 Mframes/sec (lowered: min observed 3654.9, -10% margin)
  {512, 3500}, // 3500 Mframes/sec (measured: 3890.1, with 10% margin)
  {2048, 3300} // 3300 Mframes/sec (lowered: min observed 3692.3, -10% margin)
#else
  {32, 50},   // 50 Mframes/sec (debug, raised for modern hardware)
  {128, 70},  // 70 Mframes/sec (debug, raised for modern hardware)
  {512, 80},  // 80 Mframes/sec (debug, raised for modern hardware)
  {2048, 80}  // 80 Mframes/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 2400},  // 2400 Mframes/sec (raised for modern hardware)
  {128, 2700}, // 2700 Mframes/sec (raised for modern hardware)
  {512, 2800}, // 2800 Mframes/sec (raised for modern hardware)
  {2048, 2800} // 2800 Mframes/sec (raised for modern hardware)
#else
  {32, 80},   // 80 Mframes/sec (debug, raised for modern hardware)
  {128, 130}, // 130 Mframes/sec (debug, raised for modern hardware)
  {512, 150}, // 150 Mframes/sec (debug, raised for modern hardware)
  {2048, 160} // 160 Mframes/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 1900},  // 1900 Mframes/sec (measured: 2139.4, with 10% margin)
  {128, 2200}, // 2200 Mframes/sec (measured: 2430.7, with 10% margin)
  {512, 2400}, // 2400 Mframes/sec (measured: 2688.1, with 10% margin)
  {2048, 2700} // 2700 Mframes/sec (raised for modern hardware)
#else
  {32, 80},   // 80 Mframes/sec (debug, raised for modern hardware)
  {128, 130}, // 130 Mframes/sec (debug, raised for modern hardware)
  {512, 150}, // 150 Mframes/sec (debug, raised for modern hardware)
  {2048, 160} // 160 Mframes/sec (debug, raised for modern hardware)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 1800},  // 1800 Mframes/sec (measured: 2016.6, with 10% margin)
  {128, 2200}, // 2200 Mframes/sec (measured: 2425.6, with 10% margin)
  {512, 2400}, // 2400 Mframes/sec (raised for modern hardware)
  {2048, 2500} // 2500 Mframes/sec (raised for modern hardware)
#else
  {32, 80},   // 80 Mframes/sec (debug, raised for modern hardware)
  {128, 130}, // 130 Mframes/sec (debug, raised for modern hardware)
  {512, 150}, // 150 Mframes/sec (debug, raised for modern hardware)
  {2048, 160} // 160 Mframes/sec (debug, raised for modern hardware)
#endif
};

// Extract one channel from stereo source to mono, then vectorizable AddFrom —
// compare with AddChannelFrom+coeff (stereo dst) to evaluate benefit of
// using a mono destination buffer with channel extraction.
static constexpr Baseline BASELINE_MONO_COPY_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 1600},  // 1600 Mframes/sec (measured: 1829.5, -10% margin)
  {128, 2000}, // 2000 Mframes/sec (measured: 2250.6, -10% margin)
  {512, 2100}, // 2100 Mframes/sec (measured: 2434.9, -10% margin)
  {2048, 2100} // 2100 Mframes/sec (measured: 2435.9, -10% margin)
#else
  {32, 50},   // 50 Mframes/sec (debug, measured: 57.3, -10% margin)
  {128, 60},  // 60 Mframes/sec (debug, measured: 74.4, -10% margin)
  {512, 70},  // 70 Mframes/sec (debug, measured: 79.4, -10% margin)
  {2048, 70}  // 70 Mframes/sec (debug, measured: 81.7, -10% margin)
#endif
};

// Helper function to fill buffer with sine wave signal
// Each channel gets a different frequency to make data more realistic
static void fill_with_sine_wave(GOSoundBufferMutable &buffer) {
  constexpr double PI = 3.14159265358979323846;
  constexpr double baseFrequency = 440.0; // A4 note
  constexpr double frameRate = 48000.0;

  const unsigned nChannels = buffer.GetNChannels();
  const unsigned nFrames = buffer.GetNFrames();
  GOSoundBuffer::Item *pData = buffer.GetData();

  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
      // Different frequency for each channel
      double frequency = baseFrequency * (channelI + 1);
      double phase = 2.0 * PI * frequency * frameI / frameRate;

      *pData++ = static_cast<float>(std::sin(phase));
    }
  }
}

// Helper function to measure performance
// Returns performance in millions of frames per second
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

  // Calculate millions of frames per second
  double totalFrames = static_cast<double>(bufferSize) * numIterations;
  return (totalFrames / elapsed.count()) / 1e6;
}

void GOTestPerfSoundBufferMutable::RunAndEvaluateTest(
  const std::string &functionName,
  const Baseline &baseline,
  std::function<void()> operation) {
  double mFramesPerSecond
    = measure_performance(baseline.m_BufferSize, NUM_ITERATIONS, operation);
  bool passed = mFramesPerSecond >= baseline.m_MFramesPerSecond;
  double ratio = mFramesPerSecond / baseline.m_MFramesPerSecond;

#ifdef NDEBUG
  const char *buildMode = "Release";
#else
  const char *buildMode = "Debug  ";
#endif

  std::string message = std::format(
    "{:<7} {:<21} (size={:4}): {:8.1f} Mframes/sec (baseline: {:8.1f}, "
    "ratio: {:5.2f}x)",
    buildMode,
    functionName,
    baseline.m_BufferSize,
    mFramesPerSecond,
    baseline.m_MFramesPerSecond,
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

void GOTestPerfSoundBufferMutable::TestPerfAddChannelFromMonoRecipient() {
  std::cout << "\nPerformance test: extract channel to mono + AddFrom+coeff\n";
  std::cout << "  Compare with AddChannelFrom+coeff (stereo dst) above\n";

  for (const Baseline &baseline : BASELINE_MONO_COPY_ADD_FROM_COEFF) {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize);

    GOSoundBuffer::Item srcMonoMemory[baseline.m_BufferSize];
    GOSoundBufferMutableMono srcMono(srcMonoMemory, baseline.m_BufferSize);

    GOSoundBuffer::Item dstMonoMemory[baseline.m_BufferSize];
    GOSoundBufferMutableMono dstMono(dstMonoMemory, baseline.m_BufferSize);

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstMono);

    constexpr float coeff = 0.5f;

    RunAndEvaluateTest(
      "MonoCopyThenAdd+coeff", baseline, [&dstMono, &srcMono, &srcBuffer]() {
        srcMono.CopyChannelFrom(srcBuffer, 0);
        dstMono.AddFrom(srcMono, coeff);
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
  TestPerfAddChannelFromMonoRecipient();

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