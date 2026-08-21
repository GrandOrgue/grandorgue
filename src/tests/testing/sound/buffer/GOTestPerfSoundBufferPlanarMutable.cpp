/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestPerfSoundBufferPlanarMutable.h"

#include <cmath>
#include <iostream>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"

const std::string GOTestPerfSoundBufferPlanarMutable::TEST_NAME
  = "GOTestPerfSoundBufferPlanarMutable";

// Number of channels (stereo)
static constexpr unsigned NUM_CHANNELS = 2;

// Baseline values for each function and buffer size
// Format: {buffer_size, min_MFrames_per_second}
// Baseline values measured on this development machine, from a handful of
// local runs (see GOTestPerfSoundBufferMutable.cpp for the CI-measured
// baselines this file mirrors, tuned from many runs with a -10% margin).
// Local sampling here showed FillWithSilence/CopyFrom dip much harder than
// 25% below their peak under any background system load (memset/memcpy on
// small buffers is the most noise-sensitive shape), so those two use a
// wide, deliberately loose floor instead of a fixed percentage; the rest
// use a -25% margin. All of this should be tightened once real CI history
// across many runs is available - these are a starting point, not tuned
// baselines.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_FILL_WITH_SILENCE[] = {
#ifdef NDEBUG
  {32, 5000},   // peak measured: 10929.0; wide floor, see note above
  {128, 10000}, // peak measured: 20566.1; wide floor, see note above
  {512, 11000}, // peak measured: 22299.9; wide floor, see note above
  {2048, 16000} // peak measured: 31290.4; wide floor, see note above
#else
  {32, 2000},   // debug, peak measured: 6960.3; wide floor, see note above
  {128, 6000},  // debug, peak measured: 19156.3; wide floor, see note above
  {512, 8000},  // debug, peak measured: 15670.0; wide floor, see note above
  {2048, 14000} // debug, peak measured: 26532.7; wide floor, see note above
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 6000},   // peak measured: 12135.2; wide floor, see note above
  {128, 8000},  // peak measured: 14340.6; wide floor, see note above
  {512, 10000}, // peak measured: 19693.8; wide floor, see note above
  {2048, 15000} // peak measured: 28647.6; wide floor, see note above
#else
  {32, 2500},   // debug, peak measured: 5604.9; wide floor, see note above
  {128, 7000},  // debug, peak measured: 13195.5; wide floor, see note above
  {512, 9000},  // debug, peak measured: 18243.2; wide floor, see note above
  {2048, 14000} // debug, peak measured: 26708.5; wide floor, see note above
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 3050},  // measured: 4067.4, -25% margin
  {128, 4240}, // measured: 5663.7, -25% margin
  {512, 4420}, // measured: 5894.0, -25% margin
  {2048, 4480} // measured: 5978.6, -25% margin
#else
  {32, 710},    // debug, measured: 951.8, -25% margin
  {128, 850},   // debug, measured: 1139.7, -25% margin
  {512, 890},   // debug, measured: 1194.7, -25% margin
  {2048, 910}   // debug, measured: 1219.9, -25% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 3200},  // measured: 4285.1, -25% margin
  {128, 4320}, // measured: 5764.3, -25% margin
  {512, 4280}, // measured: 5708.4, -25% margin
  {2048, 4500} // measured: 6010.9, -25% margin
#else
  {32, 620},    // debug, measured: 835.7, -25% margin
  {128, 760},   // debug, measured: 1024.3, -25% margin
  {512, 820},   // debug, measured: 1100.2, -25% margin
  {2048, 820}   // debug, measured: 1095.3, -25% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 9600},   // measured: 12890.5, -25% margin
  {128, 18700}, // measured: 25054.5, -25% margin
  {512, 25600}, // measured: 34246.0, -25% margin
  {2048, 40000} // measured: 53476.6, -25% margin
#else
  {32, 2890},   // debug, measured: 3859.6, -25% margin
  {128, 7500}, // debug, measured: 12487.6, wider margin (observed dip to ~9100)
  {512, 20100},  // debug, measured: 26860.5, -25% margin
  {2048, 35900}  // debug, measured: 47995.8, -25% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 6000},  // measured: 8119.9, -25% margin
  {128, 7700}, // measured: 10384.0, -25% margin
  {512, 8700}, // measured: 11693.1, -25% margin
  {2048, 8900} // measured: 11963.8, -25% margin
#else
  {32, 910},     // debug, measured: 1221.8, -25% margin
  {128, 1480},   // debug, measured: 1980.9, -25% margin
  {512, 1760},   // debug, measured: 2353.4, -25% margin
  {2048, 1810}   // debug, measured: 2421.1, -25% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_CHANNEL_FROM_COEFF[]
  = {
#ifdef NDEBUG
    {32, 6000},  // measured: 8057.2, -25% margin
    {128, 7900}, // measured: 10632.6, -25% margin
    {512, 8400}, // measured: 11264.0, -25% margin
    {2048, 8900} // measured: 11988.0, -25% margin
#else
    {32, 1040},  // debug, measured: 1390.7, -25% margin
    {128, 1330}, // debug, measured: 1773.9, -25% margin
    {512, 1590}, // debug, measured: 2120.8, -25% margin
    {2048, 1650} // debug, measured: 2206.0, -25% margin
#endif
};

// DeinterleaveFrom/InterleaveTo have no interleaved equivalent to compare
// against - this is the raw cost of the conversion boundary itself.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_DEINTERLEAVE_FROM[] = {
#ifdef NDEBUG
  {32, 1300},  // measured: 1776.6, -25% margin
  {128, 1700}, // measured: 2279.4, -25% margin
  {512, 1580}, // measured: 2119.1, -25% margin
  {2048, 1580} // measured: 2120.8, -25% margin
#else
  {32, 460},     // debug, measured: 623.4, -25% margin
  {128, 720},    // debug, measured: 962.4, -25% margin
  {512, 850},    // debug, measured: 1132.9, -25% margin
  {2048, 890}    // debug, measured: 1193.5, -25% margin
#endif
};

// AddDeinterleavedFrom has no interleaved equivalent to compare against -
// this is the raw cost of the conversion boundary itself, same as
// BASELINE_DEINTERLEAVE_FROM above.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_DEINTERLEAVE_FROM[]
  = {
#ifdef NDEBUG
    {32, 1300},  // measured: 1768.3, -25% margin
    {128, 1800}, // measured: 2425.3, -25% margin
    {512, 1900}, // measured: 2562.8, -25% margin
    {2048, 1900} // measured: 2595.4, -25% margin
#else
    {32, 480},   // debug, measured: 685.3, -25% margin
    {128, 620},  // debug, measured: 886.1, -25% margin
    {512, 720},  // debug, measured: 1027.0, -25% margin
    {2048, 750}  // debug, measured: 1067.8, -25% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_INTERLEAVE_TO[] = {
#ifdef NDEBUG
  {32, 1700},  // measured: 2296.4, -25% margin
  {128, 1950}, // measured: 2616.0, -25% margin
  {512, 1950}, // measured: 2610.6, -25% margin
  {2048, 2000} // measured: 2682.2, -25% margin
#else
  {32, 590},     // debug, measured: 795.3, -25% margin
  {128, 750},    // debug, measured: 1011.5, -25% margin
  {512, 880},    // debug, measured: 1175.5, -25% margin
  {2048, 890}    // debug, measured: 1193.8, -25% margin
#endif
};

// Helper function to fill a planar buffer with sine wave signal
// Each channel gets a different frequency to make data more realistic
static void fill_with_sine_wave(GOSoundBufferPlanarMutable &buffer) {
  constexpr double PI = 3.14159265358979323846;
  constexpr double baseFrequency = 440.0; // A4 note
  constexpr double frameRate = 48000.0;

  const unsigned nChannels = buffer.GetNChannels();
  const unsigned nFrames = buffer.GetNFrames();

  for (unsigned channelI = 0; channelI < nChannels; ++channelI) {
    // Different frequency for each channel
    double frequency = baseFrequency * (channelI + 1);
    GOSoundBuffer::Item *pData = buffer.GetChannelBuffer(channelI).GetData();

    for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
      double phase = 2.0 * PI * frequency * frameI / frameRate;

      *pData++ = static_cast<float>(std::sin(phase));
    }
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfFillWithSilence() {
  std::cout << "\nPerformance test: FillWithSilence (planar)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_FILL_WITH_SILENCE) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      buffer, NUM_CHANNELS, baseline.m_BufferSize)

    fill_with_sine_wave(buffer);

    RunAndEvaluateTest(
      "FillWithSilence", baseline, [&buffer]() { buffer.FillWithSilence(); });
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfCopyFrom() {
  std::cout << "\nPerformance test: CopyFrom (planar)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline : BASELINE_COPY_FROM) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    fill_with_sine_wave(srcBuffer);
    dstBuffer.FillWithSilence();

    RunAndEvaluateTest("CopyFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.CopyFrom(srcBuffer);
    });
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfAddFrom() {
  std::cout << "\nPerformance test: AddFrom (planar)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline : BASELINE_ADD_FROM) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstBuffer);

    RunAndEvaluateTest("AddFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.AddFrom(srcBuffer);
    });
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfAddFromWithCoefficient() {
  std::cout << "\nPerformance test: AddFrom (planar, with coefficient)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_ADD_FROM_COEFF) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstBuffer);

    constexpr float coeff = 0.5f;

    RunAndEvaluateTest("AddFrom+coeff", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.AddFrom(srcBuffer, coeff);
    });
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfCopyChannelFrom() {
  std::cout << "\nPerformance test: CopyChannelFrom (planar)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_COPY_CHANNEL_FROM) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    fill_with_sine_wave(srcBuffer);
    dstBuffer.FillWithSilence();

    RunAndEvaluateTest("CopyChannelFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.CopyChannelFrom(srcBuffer, 0, 1);
    });
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfAddChannelFrom() {
  std::cout << "\nPerformance test: AddChannelFrom (planar)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_ADD_CHANNEL_FROM) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    fill_with_sine_wave(srcBuffer);
    fill_with_sine_wave(dstBuffer);

    RunAndEvaluateTest("AddChannelFrom", baseline, [&dstBuffer, &srcBuffer]() {
      dstBuffer.AddChannelFrom(srcBuffer, 0, 1);
    });
  }
}

void GOTestPerfSoundBufferPlanarMutable::
  TestPerfAddChannelFromWithCoefficient() {
  std::cout
    << "\nPerformance test: AddChannelFrom (planar, with coefficient)\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_ADD_CHANNEL_FROM_COEFF) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
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

void GOTestPerfSoundBufferPlanarMutable::TestPerfDeinterleaveFrom() {
  std::cout << "\nPerformance test: DeinterleaveFrom\n";
  std::cout << "  No interleaved equivalent - this is the raw cost of the "
               "conversion boundary\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_DEINTERLEAVE_FROM) {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    srcBuffer.FillWithSilence();
    dstBuffer.FillWithSilence();

    RunAndEvaluateTest(
      "DeinterleaveFrom", baseline, [&dstBuffer, &srcBuffer]() {
        dstBuffer.DeinterleaveFrom(srcBuffer);
      });
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfAddDeinterleaveFrom() {
  std::cout << "\nPerformance test: AddDeinterleavedFrom\n";
  std::cout << "  No interleaved equivalent - this is the raw cost of the "
               "conversion boundary\n";

  for (const GOTestPerfSoundBufferBaseline &baseline :
       BASELINE_ADD_DEINTERLEAVE_FROM) {
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    srcBuffer.FillWithSilence();
    fill_with_sine_wave(dstBuffer);

    RunAndEvaluateTest(
      "AddDeinterleavedFrom", baseline, [&dstBuffer, &srcBuffer]() {
        dstBuffer.AddDeinterleavedFrom(srcBuffer);
      });
  }
}

void GOTestPerfSoundBufferPlanarMutable::TestPerfInterleaveTo() {
  std::cout << "\nPerformance test: InterleaveTo\n";
  std::cout << "  No interleaved equivalent - this is the raw cost of the "
               "conversion boundary\n";

  for (const GOTestPerfSoundBufferBaseline &baseline : BASELINE_INTERLEAVE_TO) {
    GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(
      srcBuffer, NUM_CHANNELS, baseline.m_BufferSize)
    GO_DECLARE_LOCAL_SOUND_BUFFER(
      dstBuffer, NUM_CHANNELS, baseline.m_BufferSize)

    fill_with_sine_wave(srcBuffer);
    dstBuffer.FillWithSilence();

    RunAndEvaluateTest("InterleaveTo", baseline, [&srcBuffer, &dstBuffer]() {
      srcBuffer.InterleaveTo(dstBuffer);
    });
  }
}

void GOTestPerfSoundBufferPlanarMutable::run() {
  m_failedTests.clear();

  std::cout << "\n========== Performance Tests for GOSoundBufferPlanarMutable "
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
  TestPerfDeinterleaveFrom();
  TestPerfAddDeinterleaveFrom();
  TestPerfInterleaveTo();

  std::cout << "\n========== Performance Tests Completed ==========\n";

  ReportFailedTests();
}
