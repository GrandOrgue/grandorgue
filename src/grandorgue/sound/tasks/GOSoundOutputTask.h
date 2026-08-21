/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDOUTPUTTASK_H
#define GOSOUNDOUTPUTTASK_H

#include <atomic>
#include <vector>

#include "sound/reverb/GOSoundReverb.h"
#include "threading/GOMutex.h"

#include "GOSoundBufferTaskBase.h"

class GOSoundOutputTask : public GOSoundBufferTaskBase {
private:
  std::vector<float> m_ScaleFactors;
  std::vector<GOSoundBufferTaskBase *> m_Outputs;
  unsigned m_OutputCount;
  std::vector<float> m_MeterInfo;
  GOSoundReverb *m_Reverb;
  GOMutex m_Mutex;
  std::atomic_bool m_Done;
  std::atomic_bool m_IsToComplete;

public:
  GOSoundOutputTask(
    unsigned channels,
    std::vector<float> scale_factors,
    unsigned samples_per_buffer);
  ~GOSoundOutputTask();

  void SetOutputs(std::vector<GOSoundBufferTaskBase *> outputs);

  unsigned GetPriority() const override { return PRIORITY_AUDIOOUTPUT; }
  unsigned GetCost() const override { return 0; }
  bool IsRepeatable() const override { return false; }
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void CompleteRound() override;
  void EnsureBufferReady(
    bool isToComplete, GOSchedulerThread *pThread = nullptr) override;

  void DiscardContent() override;
  void NewRound() override;

  void SetupReverb(
    const GOSoundReverb::ReverbConfig &config,
    unsigned nSamplesPerBuffer,
    unsigned sampleRate);

  const std::vector<float> &GetMeterInfo();
  void ResetMeterInfo();
};

#endif
