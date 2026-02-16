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

#include "sound/scheduler/GOSoundTask.h"
#include "threading/GOMutex.h"

#include "GOSoundBufferTaskBase.h"

class GOSoundReverb;
class GOConfig;

class GOSoundOutputTask : public GOSoundTask, public GOSoundBufferTaskBase {
private:
  std::vector<float> m_ScaleFactors;
  std::vector<GOSoundBufferTaskBase *> m_Outputs;
  unsigned m_OutputCount;
  std::vector<float> m_MeterInfo;
  GOSoundReverb *m_Reverb;
  GOMutex m_Mutex;
  std::atomic_bool m_Done;
  std::atomic_bool m_Stop;

public:
  GOSoundOutputTask(
    unsigned channels,
    std::vector<float> scale_factors,
    unsigned samples_per_buffer);
  ~GOSoundOutputTask();

  void SetOutputs(std::vector<GOSoundBufferTaskBase *> outputs);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *pThread = nullptr);
  void Exec();
  void Finish(bool stop, GOSoundThread *pThread = nullptr);

  void Clear();
  void Reset();

  void SetupReverb(GOConfig &settings);

  const std::vector<float> &GetMeterInfo();
  void ResetMeterInfo();
};

#endif
