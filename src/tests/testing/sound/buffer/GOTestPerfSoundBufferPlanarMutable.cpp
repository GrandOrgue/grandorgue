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
// Baseline values updated based on CI history (5 GitHub Actions runs of
// this branch, 2026-08-19..08-24, Debug and Release). The original
// baselines were seeded from a handful of local dev-machine runs and were
// consistently too high for GitHub's shared runners; retuned to -10% of
// the observed CI minimum, matching the convention used in
// GOTestPerfSoundBufferMutable.cpp.
// A subsequent round of CI runs (2026-08-26..08-28, both upstream and a
// fork's runners) showed runner-to-runner variance exceeding 10% for
// several entries (e.g. AddChannelFrom+coeff{512} ranged 7762.8..9305.8
// Mframes/sec across runs) and repeated near-baseline/false-positive FAILs
// for others. Those entries were widened to a -20% margin; see per-entry
// comments below.
// A further recalibration pass (2026-08-29..09-01, ~9 CI runs across both
// GrandOrgue/grandorgue and oleg68/GrandOrgue-official) found several
// entries were still failing under contention from concurrently-running
// jobs on the shared runners. Those were rebaselined to -20% of the
// lowest throughput actually observed across all of these runs.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_FILL_WITH_SILENCE[] = {
#ifdef NDEBUG
  {32, 2450},  // lowered: min observed 2721.6, -10% margin
  {128, 4990}, // widened to -20% margin: CI run 33184580207 observed
               // 6239.4, well below the previous 9087.9 minimum
  {512, 5700}, // lowered: min observed 6355.7, -10% margin
  {2048, 9000} // lowered: min observed 10022.3, -10% margin
#else
  {32, 1600},    // debug, rebaselined 2026-09-01: min observed 2023.2
                 // under contention, -20% margin
  {128, 4900},   // debug, rebaselined 2026-09-01: min observed 6123.3
                 // under contention, -20% margin
  {512, 5900},   // debug, lowered: min observed 6548.2, -10% margin
  {2048, 8900}   // debug, lowered: min observed 9834.6, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 5300},  // lowered: min observed 5924.0, -10% margin
  {128, 6900}, // rebaselined 2026-09-01: min observed 8623.6 under
               // contention, -20% margin
  {512, 6310}, // rebaselined 2026-09-01: min observed 7891.0 under
               // contention, -20% margin
  {2048, 8300} // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 9346.1 on 2026-08-26/28)
#else
  {32, 1500},    // debug, lowered: min observed 1686.3, -10% margin
  {128, 7300},   // debug, lowered: min observed 8081.5, -10% margin
  {512, 7300},   // debug, lowered: min observed 8152.6, -10% margin
  {2048, 8400}   // debug, lowered: min observed 9359.1, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 3300},  // lowered: min observed 3653.5, -10% margin
  {128, 4500}, // lowered: min observed 4989.4, -10% margin
  {512, 4900}, // lowered: min observed 5440.6, -10% margin
  {2048, 3920} // rebaselined 2026-09-01: min observed 4907.9 under
               // contention, -20% margin
#else
  {32, 530},     // debug, lowered: min observed 593.9, -10% margin
  {128, 610},    // debug, lowered: min observed 677.4, -10% margin
  {512, 630},    // debug, lowered: min observed 695.1, -10% margin
  {2048, 640}    // debug, lowered: min observed 706.5, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_FROM_COEFF[] = {
#ifdef NDEBUG
  {32, 2500},  // lowered: min observed 2750.1, -10% margin
  {128, 3500}, // rebaselined 2026-09-01: min observed 4385.8 under
               // contention, -20% margin
  {512, 3700}, // rebaselined 2026-09-01: min observed 4627.1 under
               // contention, -20% margin
  {2048, 4350} // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 4836.7 on 2026-08-26/28)
#else
  {32, 540},     // debug, lowered: min observed 594.6, -10% margin
  {128, 610},    // debug, lowered: min observed 677.0, -10% margin
  {512, 630},    // debug, lowered: min observed 696.1, -10% margin
  {2048, 640}    // debug, lowered: min observed 706.4, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 6000},   // lowered: min observed 6685.3, -10% margin
  {128, 15300}, // lowered: min observed 16955.4, -10% margin
  {512, 16000}, // rebaselined 2026-09-01: min observed 20014.5 under
                // contention, -20% margin
  {2048, 17600} // lowered: min observed 19606.0, -10% margin
#else
  {32, 1600},    // debug, lowered: min observed 1750.4, -10% margin
  {128, 5600},   // debug, lowered: min observed 6258.1, -10% margin
  {512, 12200},  // debug, lowered: min observed 13507.9, -10% margin
  {2048, 16000}  // debug, lowered: min observed 17738.8, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 5300},  // lowered: min observed 5873.9, -10% margin
  {128, 5100}, // rebaselined 2026-09-01: min observed 6382.6 under
               // contention, -20% margin
  {512, 6300}, // rebaselined 2026-09-01: min observed 7870.8 under
               // contention, -20% margin
  {2048, 9900} // lowered: min observed 10988.3, -10% margin
#else
  {32, 670},     // debug, rebaselined 2026-09-01: min observed 833.8
                 // under contention, -20% margin
  {128, 1150},   // debug, lowered: min observed 1299.8, -10% margin
  {512, 1020},   // debug, rebaselined 2026-09-01: min observed 1270.6
                 // under contention, -20% margin
  {2048, 1080}   // debug, rebaselined 2026-09-01: min observed 1352.8
                 // under contention, -20% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_CHANNEL_FROM_COEFF[]
  = {
#ifdef NDEBUG
    {32, 5100},  // lowered: min observed 5678.2, -10% margin
    {128, 6090}, // rebaselined 2026-09-01: min observed 7617.8 under
                 // contention, -20% margin
    {512, 7100}, // widened to -20% margin: CI runner variance exceeds 10%
                 // (observed as low as 7762.8 on 2026-08-26/28, a >20%
                 // spread against other runs of 9305.8)
    {2048, 7200} // rebaselined 2026-09-01: min observed 9009.1 under
                 // contention, -20% margin
#else
    {32, 750},   // debug, lowered: min observed 834.1, -10%
                 // margin
    {128, 1100}, // debug, lowered: min observed 1205.3, -10%
                 // margin
    {512, 1200}, // debug, lowered: min observed 1339.1, -10%
                 // margin
    {2048, 1250} // debug, lowered: min observed 1398.8, -10%
                 // margin
#endif
};

// DeinterleaveFrom/InterleaveTo have no interleaved equivalent to compare
// against - this is the raw cost of the conversion boundary itself.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_DEINTERLEAVE_FROM[] = {
#ifdef NDEBUG
  {32, 1150},  // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 1169.6 on 2026-08-26/28)
  {128, 1200}, // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 1383.6 on 2026-08-26/28)
  {512, 1400}, // lowered: min observed 1576.9, -10% margin
  {2048, 1400} // lowered: min observed 1581.0, -10% margin
#else
  {32, 380},     // debug, lowered: min observed 422.5, -10% margin
  {128, 550},    // debug, lowered: min observed 606.2, -10% margin
  {512, 600},    // debug, lowered: min observed 669.7, -10% margin
  {2048, 630}    // debug, lowered: min observed 696.3, -10% margin
#endif
};

// AddDeinterleavedFrom has no interleaved equivalent to compare against -
// this is the raw cost of the conversion boundary itself, same as
// BASELINE_DEINTERLEAVE_FROM above.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_DEINTERLEAVE_FROM[]
  = {
#ifdef NDEBUG
    {32, 1150},  // widened to -20% margin: CI runner variance exceeds 10%
                 // (observed as low as 1371.7 on 2026-08-26/28)
    {128, 1500}, // widened to -20% margin: CI runner variance exceeds 10%
                 // (observed as low as 1812.3 on 2026-08-26/28)
    {512, 1800}, // widened to -20% margin: CI runner variance exceeds 10%
                 // (observed as low as 2071.4 on 2026-08-26/28)
    {2048, 1610} // rebaselined 2026-09-01: min observed 2013.1 under
                 // contention, -20% margin
#else
    {32, 380},   // debug, lowered: min observed 426.2, -10% margin
    {128, 540},  // debug, lowered: min observed 605.2, -10% margin
    {512, 600},  // debug, lowered: min observed 668.9, -10% margin
    {2048, 630}  // debug, lowered: min observed 696.7, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_INTERLEAVE_TO[] = {
#ifdef NDEBUG
  {32, 1200},  // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 1414.1 on 2026-08-26/28)
  {128, 1200}, // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 1418.3 on 2026-08-26/28)
  {512, 1350}, // lowered: min observed 1513.0, -10% margin
  {2048, 1400} // lowered: min observed 1560.2, -10% margin
#else
  {32, 420},     // debug, lowered: min observed 462.9, -10% margin
  {128, 560},    // debug, lowered: min observed 625.9, -10% margin
  {512, 610},    // debug, lowered: min observed 674.8, -10% margin
  {2048, 630}    // debug, lowered: min observed 699.3, -10% margin
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
