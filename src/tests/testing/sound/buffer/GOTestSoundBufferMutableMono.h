/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDMUTABLEBUFFERMONO_H
#define GOTESTSOUNDMUTABLEBUFFERMONO_H

#include "GOTestSoundBufferBase.h"

class GOSoundBufferMutable;
class GOSoundBufferMutableMono;

class GOTestSoundBufferMutableMono : public GOTestSoundBufferBase {
private:
  static const std::string TEST_NAME;

  void TestCopyMonoFromChannel(
    GOSoundBufferMutableMono &monoBuffer,
    const GOSoundBuffer &srcBuffer,
    unsigned channelI);
  void TestCopyMonoToChannel(
    GOSoundBufferMutableMono &monoBuffer,
    GOSoundBufferMutable &dstBuffer,
    unsigned channelI);

  void TestConstructorAndBasicProperties();
  void TestGetSubBuffer();
  void TestCopyChannelFrom();
  void TestCopyChannelTo();
  void TestInvalidBuffer();
  void TestEdgeCases();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTSOUNDMUTABLEBUFFERMONO_H */
