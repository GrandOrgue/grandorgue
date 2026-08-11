/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundOutputTask.h"

#include <algorithm>

#include "scheduler/GOSchedulerThread.h"
#include "sound/reverb/GOSoundReverb.h"
#include "threading/GOMutexLocker.h"

// Maximum sound items amplitude for output
static constexpr float CLAMP_MIN = -1.0f;
static constexpr float CLAMP_MAX = 1.0f;

GOSoundOutputTask::GOSoundOutputTask(
  unsigned channels, std::vector<float> scaleFactors, unsigned samplesPerBuffer)
  : GOSoundBufferTaskBase(
    PRIORITY_AUDIOOUTPUT, false, channels, samplesPerBuffer),
    m_ScaleFactors(scaleFactors),
    m_Outputs(),
    m_OutputCount(0),
    m_MeterInfo(channels),
    m_Reverb(0) {
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

bool GOSoundOutputTask::DoRun(GOSchedulerThread *pThread) {
  bool isStopped = false;

  /* initialise the output buffer */
  FillWithSilence();

  const unsigned nChannels = GetNChannels();

  for (unsigned i = 0; i < nChannels && !isStopped; i++)
    for (unsigned j = 0; j < m_OutputCount && !isStopped; j++) {
      float factor = m_ScaleFactors[i * m_OutputCount + j];

      if (factor != 0) {
        GOSoundBufferTaskBase *output = m_Outputs[j / 2];

        output->EnsureBufferReady(m_IsToComplete.load(), pThread);
        if (pThread && pThread->ShouldStop())
          isStopped = true;
        else
          AddChannelFrom(*output, j % 2, i, factor);
      }
    }

  if (!isStopped) {
    m_Reverb->Process(GetData(), GetNFrames());

    /* Clamp the output and put the maximum amplitude to m_MeterInfo */
    float *pData = GetData();
    unsigned nChannelsRest = nChannels;
    float *pMeterInfo = m_MeterInfo.data();

    for (unsigned nItemsRest = GetNItems(); nItemsRest; nItemsRest--, pData++) {
      float f = std::clamp(*pData, CLAMP_MIN, CLAMP_MAX);
      float absF = std::abs(f);

      if (f != *pData)
        *pData = f;
      if (absF > *pMeterInfo)
        *pMeterInfo = absF;

      // Move to next channel (circular: after last channel, wrap to first)
      pMeterInfo++;
      if (!--nChannelsRest) {
        nChannelsRest = nChannels;
        pMeterInfo = m_MeterInfo.data();
      }
    }
  }

  return !isStopped;
}

void GOSoundOutputTask::EnsureBufferReady(
  bool isToComplete, GOSchedulerThread *pThread) {
  if (isToComplete)
    m_IsToComplete.store(true);
  if (!IsDone())
    Run(pThread);
}

// Read without m_mutex, like every other IsEmpty(): called only while the
// task is quiescent (deregistered from the scheduler), never concurrently
// with DoRun()
bool GOSoundOutputTask::IsEmpty() const {
  bool isEmpty = true;

  for (unsigned i = 0; i < m_MeterInfo.size() && isEmpty; i++)
    isEmpty = m_MeterInfo[i] == 0;
  return isEmpty;
}

void GOSoundOutputTask::DiscardContent() { ResetMeterInfo(); }

void GOSoundOutputTask::ResetMeterInfo() {
  GOMutexLocker locker(m_mutex);

  for (unsigned i = 0; i < m_MeterInfo.size(); i++)
    m_MeterInfo[i] = 0;
}

void GOSoundOutputTask::SetupReverb(
  const GOSoundReverb::ReverbConfig &config,
  unsigned nSamplesPerBuffer,
  unsigned sampleRate) {
  m_Reverb->Setup(config, nSamplesPerBuffer, sampleRate);
  // a freshly configured reverb engine already starts silent, but reset
  // explicitly so DiscardContent()'s old guarantee still holds if Setup()
  // ever stops implying it
  m_Reverb->Reset();
}

const std::vector<float> &GOSoundOutputTask::GetMeterInfo() {
  return m_MeterInfo;
}
