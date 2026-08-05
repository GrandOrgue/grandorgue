/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDOUTPUTTASK_H
#define GOSOUNDOUTPUTTASK_H

#include <vector>

#include "sound/reverb/GOSoundReverb.h"

#include "GOSoundBufferTaskBase.h"

class GOSoundOutputTask : public GOSoundBufferTaskBase {
private:
  std::vector<float> m_ScaleFactors;
  std::vector<GOSoundBufferTaskBase *> m_Outputs;
  unsigned m_OutputCount;
  std::vector<float> m_MeterInfo;
  GOSoundReverb *m_Reverb;

  bool DoRun(GOSchedulerThread *pThread) override;

public:
  GOSoundOutputTask(
    unsigned channels,
    std::vector<float> scaleFactors,
    unsigned samplesPerBuffer);
  ~GOSoundOutputTask();

  void SetOutputs(std::vector<GOSoundBufferTaskBase *> outputs);

  void CompleteRound() override { Run(); }
  void EnsureBufferReady(
    bool isToComplete, GOSchedulerThread *pThread = nullptr) override;

  void DiscardContent() override;

  void SetupReverb(
    const GOSoundReverb::ReverbConfig &config,
    unsigned nSamplesPerBuffer,
    unsigned sampleRate);

  const std::vector<float> &GetMeterInfo();
  void ResetMeterInfo();
};

#endif
