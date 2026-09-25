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

#include "GOTestScope.h"

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
// A further pass (2026-09-21..09-23, reviewing 3 days of CI history across
// both repos) found CopyFrom{32}, CopyChannelFrom{128}, and
// AddChannelFrom+coeff{32,128} each hit a single-run collapse to roughly
// half their usual throughput while every other run in the same window
// stayed in its normal range and no related code had changed - shared-
// runner contention, not a regression. Those entries were rebaselined to
// -20% of the lowest throughput actually observed.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_FILL_WITH_SILENCE[] = {
#ifdef NDEBUG
  {32, 1880},  // rebaselined 2026-09-17: min observed 2089.8, -10% margin
  {128, 4990}, // widened to -20% margin: CI run 33184580207 observed
               // 6239.4, well below the previous 9087.9 minimum
  {512, 5700}, // lowered: min observed 6355.7, -10% margin
  {2048, 9000} // lowered: min observed 10022.3, -10% margin
#else
  {32, 1600},  // debug, rebaselined 2026-09-01: min observed 2023.2
               // under contention, -20% margin
  {128, 4900}, // debug, rebaselined 2026-09-01: min observed 6123.3
               // under contention, -20% margin
  {512,
   5160}, // debug, rebaselined 2026-09-17: min observed 5744.2, -10% margin
  {2048, 8900} // debug, lowered: min observed 9834.6, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_COPY_FROM[] = {
#ifdef NDEBUG
  {32, 2080},  // rebaselined 2026-09-21: min observed 2602.6 under
               // contention, -20% margin
  {128, 5690}, // rebaselined 2026-09-17: min observed 6324.9, -10% margin
  {512, 6310}, // rebaselined 2026-09-01: min observed 7891.0 under
               // contention, -20% margin
  {2048, 8300} // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 9346.1 on 2026-08-26/28)
#else
  {32, 1500},  // debug, lowered: min observed 1686.3, -10% margin
  {128,
   3710}, // debug, rebaselined 2026-09-17: min observed 4129.9, -10% margin
  {512, 7300}, // debug, lowered: min observed 8152.6, -10% margin
  {2048, 8400} // debug, lowered: min observed 9359.1, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_FROM[] = {
#ifdef NDEBUG
  {32, 2040},  // rebaselined 2026-09-17: min observed 2271.0, -10% margin
  {128, 3770}, // rebaselined 2026-09-04: min observed 4719.7, -20% margin
  {512, 4070}, // rebaselined 2026-09-04: min observed 5089.7, -20% margin
  {2048, 3920} // rebaselined 2026-09-01: min observed 4907.9 under
               // contention, -20% margin
#else
  {32, 530},   // debug, lowered: min observed 593.9, -10% margin
  {128, 610},  // debug, lowered: min observed 677.4, -10% margin
  {512, 630},  // debug, lowered: min observed 695.1, -10% margin
  {2048, 640}  // debug, lowered: min observed 706.5, -10% margin
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
  {32, 380},   // debug, rebaselined 2026-09-25 after adding a size barrier to
               // defeat constant-folding: min observed 422.2, -10% margin
  {128, 450},  // debug, rebaselined 2026-09-25: min observed 509.2, -10% margin
  {512, 480},  // debug, rebaselined 2026-09-25: min observed 533.9, -10% margin
  {2048, 490}  // debug, rebaselined 2026-09-25: min observed 544.4, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_COPY_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 4440},   // rebaselined 2026-09-04: min observed 5559.7, -20% margin
  {128, 6570},  // rebaselined 2026-09-22: min observed 8222.2 under
                // contention, -20% margin
  {512, 12330}, // rebaselined 2026-09-17: min observed 13700.6, -10% margin
  {2048, 17600} // lowered: min observed 19606.0, -10% margin
#else
  {32, 1390}, // debug, rebaselined 2026-09-17: min observed 1544.6, -10% margin
  {128, 5600},  // debug, lowered: min observed 6258.1, -10% margin
  {512, 12200}, // debug, lowered: min observed 13507.9, -10% margin
  {2048,
   13260}    // debug, rebaselined 2026-09-04: min observed 16585.3, -20% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_CHANNEL_FROM[] = {
#ifdef NDEBUG
  {32, 4660},  // rebaselined 2026-09-17: min observed 5179.1, -10% margin
  {128, 5100}, // rebaselined 2026-09-01: min observed 6382.6 under
               // contention, -20% margin
  {512, 6300}, // rebaselined 2026-09-01: min observed 7870.8 under
               // contention, -20% margin
  {2048, 8280} // rebaselined 2026-09-04: min observed 10362.2, -20% margin
#else
  {32, 670}, // debug, rebaselined 2026-09-01: min observed 833.8
             // under contention, -20% margin
  {128, 920}, // debug, rebaselined 2026-09-04: min observed 1155.3, -20% margin
  {512, 1020}, // debug, rebaselined 2026-09-01: min observed 1270.6
               // under contention, -20% margin
  {2048, 1080} // debug, rebaselined 2026-09-01: min observed 1352.8
               // under contention, -20% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_CHANNEL_FROM_COEFF[]
  = {
#ifdef NDEBUG
    {32, 2570},  // rebaselined 2026-09-21: min observed 3222.4 under
                 // contention, -20% margin
    {128, 3970}, // rebaselined 2026-09-21: min observed 4968.4 under
                 // contention, -20% margin
    {512, 7100}, // widened to -20% margin: CI runner variance exceeds 10%
                 // (observed as low as 7762.8 on 2026-08-26/28, a >20%
                 // spread against other runs of 9305.8)
    {2048, 7200} // rebaselined 2026-09-01: min observed 9009.1 under
                 // contention, -20% margin
#else
    {32, 730}, // debug, rebaselined 2026-09-25 after adding a size barrier to
               // defeat constant-folding: min observed 814.0, -10% margin
    {128,
     860}, // debug, rebaselined 2026-09-25: min observed 960.1, -10% margin
    {512,
     910}, // debug, rebaselined 2026-09-25: min observed 1016.1, -10% margin
    {2048,
     940}     // debug, rebaselined 2026-09-25: min observed 1050.8, -10% margin
#endif
};

// DeinterleaveFrom/InterleaveTo have no interleaved equivalent to compare
// against - this is the raw cost of the conversion boundary itself.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_DEINTERLEAVE_FROM[] = {
#ifdef NDEBUG
  {32, 960},   // rebaselined 2026-09-04: min observed 1201.8, -20% margin
  {128, 1200}, // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 1383.6 on 2026-08-26/28)
  {512, 1120}, // rebaselined 2026-09-04: min observed 1406.1, -20% margin
  {2048, 1100} // rebaselined 2026-09-04: min observed 1376.5, -20% margin
#else
  {32, 380},  // debug, lowered: min observed 422.5, -10% margin
  {128, 550}, // debug, lowered: min observed 606.2, -10% margin
  {512, 600}, // debug, lowered: min observed 669.7, -10% margin
  {2048, 630} // debug, lowered: min observed 696.3, -10% margin
#endif
};

// AddDeinterleavedFrom has no interleaved equivalent to compare against -
// this is the raw cost of the conversion boundary itself, same as
// BASELINE_DEINTERLEAVE_FROM above.
static constexpr GOTestPerfSoundBufferBaseline BASELINE_ADD_DEINTERLEAVE_FROM[]
  = {
#ifdef NDEBUG
    {32, 900},   // rebaselined 2026-09-04: min observed 1136.7, -20% margin
    {128, 1500}, // widened to -20% margin: CI runner variance exceeds 10%
                 // (observed as low as 1812.3 on 2026-08-26/28)
    {512, 1800}, // widened to -20% margin: CI runner variance exceeds 10%
                 // (observed as low as 2071.4 on 2026-08-26/28)
    {2048, 1610} // rebaselined 2026-09-01: min observed 2013.1 under
                 // contention, -20% margin
#else
    {32, 380},  // debug, lowered: min observed 426.2, -10% margin
    {128, 540}, // debug, lowered: min observed 605.2, -10% margin
    {512, 600}, // debug, lowered: min observed 668.9, -10% margin
    {2048, 630} // debug, lowered: min observed 696.7, -10% margin
#endif
};

static constexpr GOTestPerfSoundBufferBaseline BASELINE_INTERLEAVE_TO[] = {
#ifdef NDEBUG
  {32, 1060},  // rebaselined 2026-09-17: min observed 1183.9, -10% margin
  {128, 1200}, // widened to -20% margin: CI runner variance exceeds 10%
               // (observed as low as 1418.3 on 2026-08-26/28)
  {512, 1070}, // rebaselined 2026-09-04: min observed 1343.8, -20% margin
  {2048, 1090} // rebaselined 2026-09-04: min observed 1374.5, -20% margin
#else
  {32, 420},    // debug, lowered: min observed 462.9, -10% margin
  {128, 560},   // debug, lowered: min observed 625.9, -10% margin
  {512, 610},   // debug, lowered: min observed 674.8, -10% margin
  {2048, 630}   // debug, lowered: min observed 699.3, -10% margin
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

    // Opaque view with a size the optimizer can no longer prove constant -
    // without this, the compiler fully unrolls AddFrom() into straight-line
    // scalar code instead of the packed vector loop real (runtime-sized)
    // buffers get, making this test measure a code path production traffic
    // never actually takes.
    unsigned nFrames = GOTestPerfOpaqueSize(baseline.m_BufferSize);

    GOSoundBufferPlanarMutable srcView(
      srcBuffer.GetData(), NUM_CHANNELS, nFrames);
    GOSoundBufferPlanarMutable dstView(
      dstBuffer.GetData(), NUM_CHANNELS, nFrames);

    RunAndEvaluateTest("AddFrom+coeff", baseline, [&dstView, &srcView]() {
      dstView.AddFrom(srcView, coeff);
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

    // See TestPerfAddFromWithCoefficient() above for why this view exists.
    unsigned nFrames = GOTestPerfOpaqueSize(baseline.m_BufferSize);

    GOSoundBufferPlanarMutable srcView(
      srcBuffer.GetData(), NUM_CHANNELS, nFrames);
    GOSoundBufferPlanarMutable dstView(
      dstBuffer.GetData(), NUM_CHANNELS, nFrames);

    RunAndEvaluateTest(
      "AddChannelFrom+coeff", baseline, [&dstView, &srcView]() {
        dstView.AddChannelFrom(srcView, 0, 1, coeff);
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

  GO_RUN_TEST(TestPerfFillWithSilence())
  GO_RUN_TEST(TestPerfCopyFrom())
  GO_RUN_TEST(TestPerfAddFrom())
  GO_RUN_TEST(TestPerfAddFromWithCoefficient())
  GO_RUN_TEST(TestPerfCopyChannelFrom())
  GO_RUN_TEST(TestPerfAddChannelFrom())
  GO_RUN_TEST(TestPerfAddChannelFromWithCoefficient())
  GO_RUN_TEST(TestPerfDeinterleaveFrom())
  GO_RUN_TEST(TestPerfAddDeinterleaveFrom())
  GO_RUN_TEST(TestPerfInterleaveTo())

  std::cout << "\n========== Performance Tests Completed ==========\n";

  GO_RUN_TEST(ReportFailedTests())
}
