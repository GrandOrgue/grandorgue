/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDBUFFERBASE_H
#define GOTESTPERFSOUNDBUFFERBASE_H

#include <functional>
#include <string>
#include <vector>

#include "GOTest.h"

/** One baseline expectation: minimum throughput for a given buffer size. */
struct GOTestPerfSoundBufferBaseline {
  unsigned m_BufferSize;
  double m_MFramesPerSecond;
};

/**
 * Shared performance-test infrastructure for sound buffer classes.
 * GOTestPerfSoundBufferMutable (interleaved) and
 * GOTestPerfSoundBufferPlanarMutable (planar) both derive from this, so the
 * baseline-comparison machinery is written once for both memory layouts;
 * each subclass supplies its own baseline tables and Test* cases.
 */
class GOTestPerfSoundBufferBase : public GOTest {
protected:
  // Number of iterations for performance tests
  static constexpr unsigned NUM_ITERATIONS = 1000000;

  std::vector<std::string> m_failedTests;

  /**
   * Measures performance of operation.
   * @return Throughput in millions of frames per second
   */
  static double measure_performance(
    unsigned bufferSize,
    unsigned numIterations,
    std::function<void()> operation);

  /**
   * Runs operation NUM_ITERATIONS times, compares the measured throughput
   * against baseline, prints a PASS/FAIL line, and records failures into
   * m_failedTests.
   */
  void RunAndEvaluateTest(
    const std::string &functionName,
    const GOTestPerfSoundBufferBaseline &baseline,
    std::function<void()> operation);

  /**
   * Fails the GOTest (via GOAssert) listing every case recorded by
   * RunAndEvaluateTest() that underperformed its baseline. Call at the end
   * of run().
   */
  void ReportFailedTests();

public:
  inline GOTestPerfSoundBufferBase() : GOTest(GOTest::PERF) {}
};

#endif /* GOTESTPERFSOUNDBUFFERBASE_H */
