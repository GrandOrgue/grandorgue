/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestVersioned.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

#include "threading/GOVersioned.h"

#include "GOTestScope.h"

const std::string GOTestVersioned::TEST_NAME = "GOTestVersioned";

/** Larger than any atomic, so that a torn copy is possible. All the fields of
 * a consistent value are equal. */
struct TestValue {
  uint64_t a, b, c, d;
};

using TestVersioned = GOVersioned<TestValue>;

/** The concurrent test runs at least this many reader iterations... */
static constexpr unsigned MIN_ITERATIONS = 2000000;
/** ...and until the reader has observed at least this many publications... */
static constexpr unsigned MIN_RENEWED = 1000;
/** ...but not longer than this */
static constexpr std::chrono::seconds TIME_LIMIT(10);
/** The reader yields after this many failed renewals in a row */
static constexpr unsigned YIELD_PERIOD = 1000;
/** The reader checks the time limit every this many iterations */
static constexpr unsigned TIME_CHECK_PERIOD = 1024;

static TestValue make_value(uint64_t n) { return {n, n, n, n}; }

static bool is_consistent(const TestValue &v) {
  return v.b == v.a && v.c == v.a && v.d == v.a;
}

static bool is_value(const TestValue &v, uint64_t n) {
  return is_consistent(v) && v.a == n;
}

void GOTestVersioned::TestNothingPublished() {
  TestVersioned versioned;
  TestVersioned::Snapshot snapshot;

  GOAssert(
    !versioned.RenewSnapshot(snapshot),
    "there is nothing to renew before the first Publish()");
  GOAssert(
    is_value(snapshot.Get(), 0),
    "a snapshot that was never renewed must hold a default-constructed value");
}

void GOTestVersioned::TestRenewFollowsPublish() {
  TestVersioned versioned;
  TestVersioned::Snapshot snapshot;

  versioned.Publish(make_value(1));
  GOAssert(
    versioned.RenewSnapshot(snapshot), "the first value must be received");
  GOAssert(
    is_value(snapshot.Get(), 1), "the snapshot must hold the first value");

  versioned.Publish(make_value(2));
  GOAssert(
    versioned.RenewSnapshot(snapshot), "the next value must be received");
  GOAssert(
    is_value(snapshot.Get(), 2), "the snapshot must hold the next value");
}

void GOTestVersioned::TestRenewIsIdempotent() {
  TestVersioned versioned;
  TestVersioned::Snapshot snapshot;

  versioned.Publish(make_value(1));
  GOAssert(versioned.RenewSnapshot(snapshot), "a new value must renew");
  GOAssert(
    !versioned.RenewSnapshot(snapshot),
    "the same publication must not renew again");
  GOAssert(
    !versioned.RenewSnapshot(snapshot),
    "repeated calls without a Publish() must not renew");
  GOAssert(is_value(snapshot.Get(), 1), "repeated calls must keep the value");

  versioned.Publish(make_value(1));
  GOAssert(
    versioned.RenewSnapshot(snapshot),
    "publishing an equal value is a new publication");
}

void GOTestVersioned::TestSnapshotsAreIndependent() {
  TestVersioned versioned;
  TestVersioned::Snapshot snapshot1;
  TestVersioned::Snapshot snapshot2;

  versioned.Publish(make_value(1));
  GOAssert(versioned.RenewSnapshot(snapshot1), "the first reader must renew");
  GOAssert(
    is_value(snapshot2.Get(), 0),
    "renewing one snapshot must not change another one");
  GOAssert(versioned.RenewSnapshot(snapshot2), "the second reader must renew");

  versioned.Publish(make_value(2));
  GOAssert(versioned.RenewSnapshot(snapshot1), "the first reader must renew");
  GOAssert(
    is_value(snapshot2.Get(), 1),
    "a snapshot that was not renewed must keep its value");
}

void GOTestVersioned::TestRenewNeverSeesTornValue() {
  TestVersioned versioned;
  TestVersioned::Snapshot snapshot;
  std::atomic_bool isStopped(false);
  std::thread writer([&]() {
    for (uint64_t n = 1; !isStopped.load(); n++)
      versioned.Publish(make_value(n));
  });

  const auto deadline = std::chrono::steady_clock::now() + TIME_LIMIT;
  uint64_t lastSeen = 0;
  unsigned nRenewed = 0;
  unsigned nFailedInRow = 0;
  bool isTimeUp = false;
  bool isTorn = false;
  bool isGoneBack = false;

  // The test does not depend on the scheduler: it runs until the reader has
  // observed enough publications, and fails only when the time limit expires
  for (unsigned i = 0;
       !isTimeUp && (i < MIN_ITERATIONS || nRenewed < MIN_RENEWED);
       i++) {
    if (versioned.RenewSnapshot(snapshot)) {
      nRenewed++;
      nFailedInRow = 0;
    } else if (++nFailedInRow % YIELD_PERIOD == 0)
      // let the writer finish its publication even on a single core
      std::this_thread::yield();

    const TestValue &value = snapshot.Get();

    if (!is_consistent(value))
      isTorn = true;
    if (value.a < lastSeen)
      isGoneBack = true;
    lastSeen = value.a;
    if (i % TIME_CHECK_PERIOD == 0)
      isTimeUp = std::chrono::steady_clock::now() >= deadline;
  }
  // join before any assertion: an assertion throws, and destroying a joinable
  // std::thread would terminate the process
  isStopped.store(true);
  writer.join();
  GOAssert(!isTorn, "the snapshot must never hold a torn value");
  GOAssert(!isGoneBack, "the snapshot must never go back to an older value");
  GOAssert(
    nRenewed >= MIN_RENEWED,
    "the reader did not observe enough publications within the time limit");
}

void GOTestVersioned::run() {
  GO_RUN_TEST(TestNothingPublished())
  GO_RUN_TEST(TestRenewFollowsPublish())
  GO_RUN_TEST(TestRenewIsIdempotent())
  GO_RUN_TEST(TestSnapshotsAreIndependent())
  GO_RUN_TEST(TestRenewNeverSeesTornValue())
}
