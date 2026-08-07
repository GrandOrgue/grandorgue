/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERPLANAR_H
#define GOSOUNDBUFFERPLANAR_H

#include <cassert>

#include "GOSoundBuffer.h"
#include "GOSoundBufferMutable.h"

/**
 * Basic read-only class for manipulation with a memory buffer of planar
 * (channel-major, non-interleaved) sound data.
 *
 * We assume that the buffer consists of nChannels consecutive contiguous
 * channels, each nFrames items long: channel c starts at pData + c *
 * nFrames. This is the counterpart of GOSoundBuffer, which assumes
 * interleaved data. The two are intentionally not related by inheritance:
 * GOSoundBuffer's GetItemIndex(), GetSubBuffer(), CopyChannelFrom() and
 * AddChannelFrom() all assume interleaved stride and would be silently
 * wrong on a planar block.
 *
 * The class is a non-owning wrapper around existing memory. Multiple
 * GOSoundBufferPlanar objects can point to the same memory region (shared
 * data). Copying creates a shallow copy that shares the underlying data.
 */
class GOSoundBufferPlanar {
public:
  // Currently we work with float items
  using Item = GOSoundBuffer::Item;

private:
  friend class GOSoundBufferPlanarMutable;

  // Points to the memory buffer of nChannels * nFrames sound items,
  // channel-major
  const Item *p_data;
  // Number of channels in the buffer
  unsigned m_NChannels;
  // Number of frames per channel in the buffer
  unsigned m_NFrames;

public:
  /**
   * Constructor for a read-only planar buffer.
   * @param pData Pointer to nChannels * nFrames sound items, channel-major
   * @param nChannels Number of channels
   * @param nFrames Number of frames per channel
   */
  inline GOSoundBufferPlanar(
    const Item *pData, unsigned nChannels, unsigned nFrames)
    : p_data(pData), m_NChannels(nChannels), m_NFrames(nFrames) {}

  inline GOSoundBufferPlanar(const GOSoundBufferPlanar &srcBuffer) = default;

  /** @return Pointer to the underlying channel-major data */
  inline const Item *GetData() const { return p_data; }

  /** @return Number of channels in this buffer */
  inline unsigned GetNChannels() const { return m_NChannels; }

  /** @return Number of frames per channel in this buffer */
  inline unsigned GetNFrames() const { return m_NFrames; }

  inline bool isValid() const {
    return m_NChannels > 0 && m_NFrames > 0 && p_data;
  }

  /**
   * Get total number of sound items in this buffer.
   * @return Total number of sound items (GetNChannels() * GetNFrames())
   */
  inline unsigned GetNItems() const {
    return GOSoundBuffer::getNItems(m_NChannels, m_NFrames);
  }

  /**
   * Get size of the buffer in bytes.
   * @return Size in bytes (sizeof(Item) * GetNItems())
   */
  inline unsigned GetNBytes() const { return sizeof(Item) * GetNItems(); }

  /**
   * A contiguous read-only view of one channel. A single channel has no
   * interleaving to speak of, so this is a plain GOSoundBuffer(pData, 1,
   * nFrames) - no new type needed.
   * @param channelI Channel index (0-based)
   * @return A read-only view of the frames of the given channel
   */
  inline GOSoundBuffer GetChannelBuffer(unsigned channelI) const {
    assert(channelI < m_NChannels);
    return GOSoundBuffer(p_data + channelI * m_NFrames, 1, m_NFrames);
  }

  /**
   * Gathers this buffer into an interleaved buffer of the same shape.
   * @param dstBuffer Interleaved destination buffer, same nChannels and
   *   nFrames as this buffer
   */
  inline void InterleaveTo(GOSoundBufferMutable &dstBuffer) const {
    GOSoundBufferMutable::assertBuffersCompatible(*this, dstBuffer);

    for (unsigned channelI = 0; channelI < m_NChannels; ++channelI)
      dstBuffer.CopyChannelFrom(GetChannelBuffer(channelI), 0, channelI);
  }

protected:
  // Subclasses may create an invalid instance and then they may call Assign
  inline GOSoundBufferPlanar()
    : p_data(nullptr), m_NChannels(0), m_NFrames(0) {}

  // Some subclasses may switch to another memory buffer
  inline void Assign(const Item *pData, unsigned nChannels, unsigned nFrames) {
    p_data = pData;
    m_NChannels = nChannels;
    m_NFrames = nFrames;
  }
};

#endif /* GOSOUNDBUFFERPLANAR_H */
