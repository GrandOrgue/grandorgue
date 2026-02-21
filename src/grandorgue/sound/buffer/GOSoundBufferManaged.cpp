/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundBufferManaged.h"

void GOSoundBufferManaged::Resize(unsigned nChannels, unsigned nFrames) {
  if (nChannels != GetNChannels() || nFrames != GetNFrames()) {
    unsigned newNItems = getNItems(nChannels, nFrames);

    // reallocate memory only if the size is changed
    if (newNItems != GetNItems()) {
      if (newNItems)
        m_OwnedData = std::make_unique<Item[]>(newNItems);
      else
        m_OwnedData.reset();
    }
    Assign(m_OwnedData.get(), nChannels, nFrames);
  }
}

void GOSoundBufferManaged::CopyDataFrom(const GOSoundBuffer &srcBuffer) {
  Resize(srcBuffer.GetNChannels(), srcBuffer.GetNFrames());
  // After Resize, both buffers have same validity: either both valid or both
  // invalid
  if (isValid())
    CopyFrom(srcBuffer);
}

void GOSoundBufferManaged::MoveDataFrom(GOSoundBufferManaged &other) noexcept {
  m_OwnedData = std::move(other.m_OwnedData);
  Assign(other.GetData(), other.GetNChannels(), other.GetNFrames());
  other.Assign(nullptr, 0, 0);
}

void GOSoundBufferManaged::Swap(GOSoundBufferManaged &other) noexcept {
  // Save current data of this buffer
  unsigned oldChannels = GetNChannels();
  unsigned oldFrames = GetNFrames();

  // Swap ownership of memory
  std::swap(m_OwnedData, other.m_OwnedData);

  // Update this buffer from new m_OwnedData
  Assign(m_OwnedData.get(), other.GetNChannels(), other.GetNFrames());

  // Update other buffer from new other.m_OwnedData
  other.Assign(other.m_OwnedData.get(), oldChannels, oldFrames);
}
