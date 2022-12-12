/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLoadThread.h"

#include "model/GOCacheObject.h"

void GOLoadThread::CheckResult() {
  Wait();
  AssertNoException();
}

void GOLoadThread::Entry() {
  GOCacheObject *obj = nullptr;

  while (!ShouldStop() && LoadNextObject(obj)) {
  }
}
