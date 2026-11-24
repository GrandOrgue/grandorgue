/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundVibratoProcessorState.h"

#include <algorithm>

#include "sound/buffer/GOSoundBuffer.h"

GOSoundVibratoProcessorState::GOSoundVibratoProcessorState(
  unsigned nChannels, unsigned nRingFrames, unsigned nDelayBufferTailFrames)
  : m_NDelayBufferRingFrames(nRingFrames),
    m_NDelayBufferTailFrames(nDelayBufferTailFrames),
    m_DelayBufferRingWithTail(nChannels, nRingFrames + nDelayBufferTailFrames) {
  assert(nDelayBufferTailFrames > 0);
  assert(nRingFrames > nDelayBufferTailFrames);

  // Qualified: a virtual call in a constructor resolves to this class anyway
  GOSoundVibratoProcessorState::Reset();
}

unsigned GOSoundVibratoProcessorState::GetNDelayBufferTailFrames() const {
  return m_NDelayBufferTailFrames;
}

void GOSoundVibratoProcessorState::WriteToDelayBuffer(
  const GOSoundBufferPlanar &srcBuffer) {
  const unsigned nFrames = srcBuffer.GetNFrames();
  const unsigned nChannels = srcBuffer.GetNChannels();

  assert(GetNFramesAvailableToWrite() >= nFrames);

  unsigned nFramesLeft = nFrames;

  while (nFramesLeft > 0) {
    const unsigned writePos = m_DelayBufferWritePos;

    // Else m_NDelayBufferRingFrames - writePos below would underflow
    assert(writePos < m_NDelayBufferRingFrames);

    const unsigned nChunkFrames
      = std::min(nFramesLeft, m_NDelayBufferRingFrames - writePos);
    // Where this chunk starts in the source
    const unsigned srcPos = nFrames - nFramesLeft;
    // The part of this chunk that lies in [0, m_NDelayBufferTailFrames)
    const unsigned nFramesToMirror = writePos < m_NDelayBufferTailFrames
      ? std::min(nChunkFrames, m_NDelayBufferTailFrames - writePos)
      : 0;

    for (unsigned channelI = 0; channelI < nChannels; channelI++) {
      GOSoundBuffer srcChannel = srcBuffer.GetChannelBuffer(channelI);

      GetDelayMonoSubBuffer(channelI, writePos, nChunkFrames)
        .CopyFrom(srcChannel.GetSubBuffer(srcPos, nChunkFrames));
      if (nFramesToMirror > 0)
        GetDelayMonoSubBuffer(
          channelI, m_NDelayBufferRingFrames + writePos, nFramesToMirror)
          .CopyFrom(GetDelayMonoSubBuffer(channelI, writePos, nFramesToMirror));
    }
    // The chunk does not cross the end of the ring, so at most it reaches it
    m_DelayBufferWritePos += nChunkFrames;
    if (m_DelayBufferWritePos >= m_NDelayBufferRingFrames)
      m_DelayBufferWritePos = 0;
    nFramesLeft -= nChunkFrames;
  }
}

void GOSoundVibratoProcessorState::Reset() {
  const unsigned nChannels = m_DelayBufferRingWithTail.GetNChannels();

  for (unsigned channelI = 0; channelI < nChannels; ++channelI)
    GetDelayMonoSubBuffer(channelI, 0, m_NDelayBufferTailFrames)
      .FillWithSilence();
  m_DelayBufferWritePos = m_NDelayBufferTailFrames;
  m_DelayBufferReadResamplingPos.Init(0);
  m_ActivatingPhase = INACTIVE;
  m_CrossfadePos = 0;
  m_LastCrossfadeNFrames = 0;
}
