/*
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTVERSIONED_H
#define GOTESTVERSIONED_H

#include <string>

#include "GOTest.h"

/**
 * Coverage for GOVersioned<T>: a snapshot follows the published values, is
 * renewed only when something new has been published, snapshots of different
 * readers are independent, and a reader running concurrently with the writer
 * never gets a torn or older value.
 */
class GOTestVersioned : public GOTest {
private:
  static const std::string TEST_NAME;

  /** Nothing published: RenewSnapshot() returns false and the snapshot holds
   * a default-constructed value. */
  void TestNothingPublished();

  /** The snapshot follows every Publish(). */
  void TestRenewFollowsPublish();

  /** RenewSnapshot() returns true only the first time after a Publish();
   * publishing an equal value is a new publication. */
  void TestRenewIsIdempotent();

  /** Renewing one snapshot does not change another one. */
  void TestSnapshotsAreIndependent();

  /** A reader that renews in a loop while another thread publishes never
   * sees a torn value (fields of different publications mixed) and never goes
   * back to an older value. */
  void TestRenewNeverSeesTornValue();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTVERSIONED_H */
