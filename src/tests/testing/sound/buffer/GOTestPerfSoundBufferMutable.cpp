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
  {32, 2680},  // 2680 Mframes/sec (lowered: min observed 2975.4, -10% margin)
  {128, 5400}, // 5400 Mframes/sec (lowered: min observed 5120.3, -10% margin)
  {512, 5500}, // 5500 Mframes/sec (lowered: min observed 6112.9, -10% margin)
  {2048, 8600} // 8600 Mframes/sec (lowered: min observed 9653.4, -10% margin)
#else
  {32,
   2330}, // 2330 Mframes/sec (debug, lowered: min observed 2590.4, -10% margin)
  {128,
   4050}, // 4050 Mframes/sec (debug, lowered: min observed 4507.3, -10% margin)
  {512,
   5800}, // 5800 Mframes/sec (debug, raised: min observed 6548.4, -10% margin)
  {2048,
   8080} // 8080 Mframes/sec (debug, lowered: min observed 8979.2, -10% margin)
#endif
};

static constexpr Baseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 4240},  // 4240 Mframes/sec (lowered: min observed 4718.7, -10% margin)
  {128, 5980}, // 5980 Mframes/sec (lowered: min observed 6647.4, -10% margin)
  {512, 7500}, // 7500 Mframes/sec (raised: min observed 8354.3, -10% margin)
  {2048, 8530} // 8530 Mframes/sec (lowered: min observed 9483.2, -10% margin)
#else
  {32,
   1550}, // 1550 Mframes/sec (debug, lowered: min observed 1727.1, -10% margin)
  {128,
   5750}, // 5750 Mframes/sec (debug, lowered: min observed 6390.3, -10% margin)
  {512,
   5410}, // 5410 Mframes/sec (debug, lowered: min observed 6018.0, -10% margin)
  {2048,
   7400} // 7400 Mframes/sec (debug, lowered: min observed 8231.6, -10% margin)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 3100},  // 3100 Mframes/sec (measured: 3492.3, with 10% margin)
  {128, 4000}, // 4000 Mframes/sec (lowered: min observed 4485.6, -10% margin)
  {512, 4240}, // 4240 Mframes/sec (lowered: min observed 4720.2, -10% margin)
  {2048, 4900} // 4900 Mframes/sec (raised: min observed 5542.0, -10% margin)
#else
  {32,
   480}, // 480 Mframes/sec (debug, lowered: min observed 533.6, -10% margin)
  {128,
   580}, // 580 Mframes/sec (debug, lowered: min observed 650.5, -10% margin)
  {512,
   630}, // 630 Mframes/sec (debug, lowered: min observed 696.8, -10% margin)
  {2048,
   620} // 620 Mframes/sec (debug, lowered: min observed 691.8, -10% margin)
#endif
};

static constexpr Baseline BASELINE_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 2800},  // 2800 Mframes/sec (lowered: min observed 3136.9, -10% margin)
  {128, 3940}, // 3940 Mframes/sec (lowered: min observed 4373.0, -10% margin)
  {512, 4800}, // 4800 Mframes/sec (raised: min observed 5452.5, -10% margin)
  {2048, 4300} // 4300 Mframes/sec (lowered: min observed 4781.1, -10% margin)
#else
  {32, 450}, // 450 Mframes/sec (debug, raised: min observed 506.4, -10% margin)
  {128,
   470}, // 470 Mframes/sec (debug, raised: min observed 533.3, -10% margin)
  {512,
   500}, // 500 Mframes/sec (debug, raised: min observed 557.2, -10% margin)
  {2048,
   430} // 430 Mframes/sec (debug, lowered: min observed 478.3, -10% margin)
#endif
};

static constexpr Baseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 2400},  // 2400 Mframes/sec (raised for modern hardware)
  {128, 2700}, // 2700 Mframes/sec (raised for modern hardware)
  {512, 2470}, // 2470 Mframes/sec (lowered: min observed 2746.1, -10% margin)
  {2048, 2510} // 2510 Mframes/sec (lowered: min observed 2797.1, -10% margin)
#else
  {32,
   710}, // 710 Mframes/sec (debug, lowered: min observed 790.2, -10% margin)
  {128,
   1230}, // 1230 Mframes/sec (debug, lowered: min observed 1365.5, -10% margin)
  {512,
   1220}, // 1220 Mframes/sec (debug, lowered: min observed 1356.2, -10% margin)
  {2048,
   1260} // 1260 Mframes/sec (debug, lowered: min observed 1395.1, -10% margin)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 1900},  // 1900 Mframes/sec (measured: 2139.4, with 10% margin)
  {128, 2200}, // 2200 Mframes/sec (measured: 2430.7, with 10% margin)
  {512, 2400}, // 2400 Mframes/sec (measured: 2688.1, with 10% margin)
  {2048, 2400} // 2400 Mframes/sec (lowered: min observed 2693.2, -10% margin)
#else
  {32,
   820}, // 820 Mframes/sec (debug, lowered: min observed 908.9, -10% margin)
  {128,
   1160}, // 1160 Mframes/sec (debug, raised: min observed 1295.5, -10% margin)
  {512,
   1300}, // 1300 Mframes/sec (debug, raised: min observed 1445.8, -10% margin)
  {2048,
   1300} // 1300 Mframes/sec (debug, raised: min observed 1496.9, -10% margin)
#endif
};

static constexpr Baseline BASELINE_ADD_CHANNEL_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 1800},  // 1800 Mframes/sec (measured: 2016.6, with 10% margin)
  {128, 2200}, // 2200 Mframes/sec (measured: 2425.6, with 10% margin)
  {512, 2400}, // 2400 Mframes/sec (raised for modern hardware)
  {2048, 2200} // 2200 Mframes/sec (lowered: min observed 2458.4, -10% margin)
#else
  {32,
   730}, // 730 Mframes/sec (debug, lowered: min observed 812.2, -10% margin)
  {128,
   1200}, // 1200 Mframes/sec (debug, raised: min observed 1352.8, -10% margin)
  {512,
   1300}, // 1300 Mframes/sec (debug, raised: min observed 1533.3, -10% margin)
  {2048,
   1250} // 1250 Mframes/sec (debug, lowered: min observed 1395.6, -10% margin)
#endif
};

// Extract one channel from stereo source to mono, then vectorizable AddFrom —
// compare with AddChannelFrom+coeff (stereo dst) to evaluate benefit of
// using a mono destination buffer with channel extraction.
static constexpr Baseline BASELINE_MONO_COPY_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 1420},  // 1420 Mframes/sec (lowered: min observed 1575.8, -10% margin)
  {128, 1790}, // 1790 Mframes/sec (lowered: min observed 1996.9, -10% margin)
  {512, 2100}, // 2100 Mframes/sec (measured: 2434.9, -10% margin)
  {2048, 2100} // 2100 Mframes/sec (measured: 2435.9, -10% margin)
#else
  {32,
   450}, // 450 Mframes/sec (debug, lowered: min observed 502.5, -10% margin)
  {128,
   590}, // 590 Mframes/sec (debug, raised: min observed 659.3, -10% margin)
  {512,
   640}, // 640 Mframes/sec (debug, raised: min observed 724.3, -10% margin)
  {2048,
   660} // 660 Mframes/sec (debug, raised: min observed 744.4, -10% margin)
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
    GO_DECLARE_LOCAL_SOUND_BUFFER(buffer, NUM_CHANNELS, baseline.m_BufferSize)

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
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

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
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

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
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

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
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

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
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

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
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

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
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)

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