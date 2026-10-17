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
 * Passes bufferSize through an out-of-line, externally-defined function -
 * one the caller's translation unit cannot see the body of - so the
 * optimizer can no longer prove it a compile-time constant. Without this,
 * the compiler fully unrolls the operation under test (e.g. AddFrom()) into
 * straight-line scalar code instead of the packed vector loop real
 * (runtime-sized) buffers get, making the test measure a code path
 * production traffic never actually takes.
 *
 * This relies on nothing but ordinary extern linkage across translation
 * units built without link-time optimization (the project does not enable
 * LTO), unlike a GCC/Clang inline-asm register barrier, so it works with
 * any compiler.
 */
unsigned GOTestPerfOpaqueSize(unsigned bufferSize);

/**
 * Shared performance-test infrastructure for sound buffer classes.
 * GOTestPerfSoundBufferMutable (interleaved) and
 * GOTestPerfSoundBufferPlanarMutable (planar) both derive from this, so the
 * baseline-comparison machinery is written once for both memory layouts;
 * each subclass supplies its own baseline tables and Test* cases.
 */
class GOTestPerfSoundBufferBase : public GOTest {
protected:
  std::vector<std::string> m_failedTests;

  /**
   * Number of iterations for performance tests. Overridable so a subclass
   * with an unusually large number of Test*() cases (e.g. one covering
   * several resampler/buffer-size combinations) can shorten its own total
   * runtime without affecting every other perf-test subclass's measurement
   * precision.
   */
  virtual unsigned GetNumIterations() const { return 1000000; }

  /**
   * Measures performance of operation.
   * @return Throughput in millions of units (see nUnitsPerFrame in
   *   RunAndEvaluateTest()) per second
   */
  static double measure_performance(
    unsigned bufferSize,
    unsigned numIterations,
    std::function<void()> operation);

  /**
   * Runs operation GetNumIterations() times, compares the measured throughput
   * against baseline, prints a PASS/FAIL line, and records failures into
   * m_failedTests.
   * @param nUnitsPerFrame scales baseline.m_BufferSize (a frame count) for
   *   the throughput calculation - 1 (the default) means the measured
   *   throughput is already in frames.
   * @param isItemsPerSecond selects the printed unit label: false (the
   *   default) prints Mframes/sec, unchanged from today for every existing
   *   caller; a subclass passing true (with nUnitsPerFrame set to its own
   *   nOutChannels) prints Mitems/sec instead.
   */
  void RunAndEvaluateTest(
    const std::string &functionName,
    const GOTestPerfSoundBufferBaseline &baseline,
    std::function<void()> operation,
    unsigned nUnitsPerFrame = 1,
    bool isItemsPerSecond = false);

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
