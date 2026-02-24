/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundOutputTask.h"

#include <algorithm>

#include "sound/reverb/GOSoundReverb.h"
#include "sound/scheduler/GOSoundThread.h"
#include "threading/GOMutexLocker.h"

// Maximum sound items amplitude for output
static constexpr float CLAMP_MIN = -1.0f;
static constexpr float CLAMP_MAX = 1.0f;

GOSoundOutputTask::GOSoundOutputTask(
  unsigned channels,
  std::vector<float> scale_factors,
  unsigned samples_per_buffer)
  : GOSoundBufferTaskBase(channels, samples_per_buffer),
    m_ScaleFactors(scale_factors),
    m_Outputs(),
    m_OutputCount(0),
    m_MeterInfo(channels),
    m_Reverb(0),
    m_Done(false) {
  m_Reverb = new GOSoundReverb(channels);
}

GOSoundOutputTask::~GOSoundOutputTask() {
  if (m_Reverb)
    delete m_Reverb;
}

void GOSoundOutputTask::SetOutputs(
  std::vector<GOSoundBufferTaskBase *> outputs) {
  m_Outputs = outputs;
  m_OutputCount = m_Outputs.size() * 2;
}

void GOSoundOutputTask::Run(GOSoundThread *pThread) {
  if (m_Done.load())
    return;
  GOMutexLocker locker(m_Mutex, false, "GOSoundOutputTask::Run", pThread);

  if (m_Done.load() || !locker.IsLocked())
    return;

  /* initialise the output buffer */
  FillWithSilence();

  const unsigned nChannels = GetNChannels();

  for (unsigned i = 0; i < nChannels; i++) {
    for (unsigned j = 0; j < m_OutputCount; j++) {
      float factor = m_ScaleFactors[i * m_OutputCount + j];

      if (factor == 0)
        continue;

      GOSoundBufferTaskBase *output = m_Outputs[j / 2];

      output->Finish(m_Stop.load(), pThread);
      if (pThread && pThread->ShouldStop())
        return;

      AddChannelFrom(*output, j % 2, i, factor);
    }
  }

  m_Reverb->Process(GetData(), GetNFrames());

  /* Clamp the output and put the maximum amplitude to m_MeterInfo */
  float *pData = GetData();
  float *pMeterInfo = m_MeterInfo.data();

  for (unsigned nItems = GetNItems(), itemI = 0, channelI = 0; itemI < nItems;
       itemI++, pData++, pMeterInfo++) {
    float f = std::clamp(*pData, CLAMP_MIN, CLAMP_MAX);
    float absF = std::abs(f);

    if (f != *pData)
      *pData = f;
    if (absF > *pMeterInfo)
      *pMeterInfo = absF;

    // Move to next channel (circular: after last channel, wrap to first)
    channelI++;
    if (channelI >= nChannels) {
      channelI = 0;
      pMeterInfo = m_MeterInfo.data();
    }
  }

  m_Done.store(true);
}

void GOSoundOutputTask::Exec() { Run(); }

void GOSoundOutputTask::Finish(bool stop, GOSoundThread *pThread) {
  if (stop)
    m_Stop.store(true);
  if (!m_Done.load())
    Run(pThread);
}

void GOSoundOutputTask::Clear() {
  m_Reverb->Reset();
  ResetMeterInfo();
}

void GOSoundOutputTask::ResetMeterInfo() {
  GOMutexLocker locker(m_Mutex);
  for (unsigned i = 0; i < m_MeterInfo.size(); i++)
    m_MeterInfo[i] = 0;
}

void GOSoundOutputTask::Reset() {
  GOMutexLocker locker(m_Mutex);
  m_Done.store(false);
  m_Stop.store(false);
}

unsigned GOSoundOutputTask::GetGroup() { return AUDIOOUTPUT; }

unsigned GOSoundOutputTask::GetCost() { return 0; }

bool GOSoundOutputTask::GetRepeat() { return false; }

void GOSoundOutputTask::SetupReverb(
  const GOSoundReverb::ReverbConfig &config,
  unsigned nSamplesPerBuffer,
  unsigned sampleRate) {
  m_Reverb->Setup(config, nSamplesPerBuffer, sampleRate);
}

const std::vector<float> &GOSoundOutputTask::GetMeterInfo() {
  return m_MeterInfo;
}
