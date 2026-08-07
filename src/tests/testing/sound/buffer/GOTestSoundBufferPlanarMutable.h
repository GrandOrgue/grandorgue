/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERPLANARMUTABLE_H
#define GOTESTSOUNDBUFFERPLANARMUTABLE_H

#include "GOTestSoundBufferBase.h"

class GOTestSoundBufferPlanarMutable : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestGetChannelBufferWriteThrough();
  void TestFillWithSilence();
  void TestCopyFrom();
  void TestAddFrom();
  void TestAddFromWithCoefficient();
  void TestCopyChannelFrom();
  void TestAddChannelFrom();
  void TestAddChannelFromWithCoefficient();
  void TestDeinterleaveFrom();
  void TestAddDeinterleavedFrom();
  void TestSingleChannelBuffer();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDBUFFERPLANARMUTABLE_H */
