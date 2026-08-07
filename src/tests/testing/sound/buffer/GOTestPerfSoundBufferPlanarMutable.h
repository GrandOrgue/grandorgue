/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTPERFSOUNDBUFFERPLANARMUTABLE_H
#define GOTESTPERFSOUNDBUFFERPLANARMUTABLE_H

#include "GOTestPerfSoundBufferBase.h"

#include <string>

class GOTestPerfSoundBufferPlanarMutable : public GOTestPerfSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestPerfFillWithSilence();
  void TestPerfCopyFrom();
  void TestPerfAddFrom();
  void TestPerfAddFromWithCoefficient();
  void TestPerfCopyChannelFrom();
  void TestPerfAddChannelFrom();
  void TestPerfAddChannelFromWithCoefficient();
  void TestPerfDeinterleaveFrom();
  void TestPerfAddDeinterleaveFrom();
  void TestPerfInterleaveTo();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTPERFSOUNDBUFFERPLANARMUTABLE_H */
