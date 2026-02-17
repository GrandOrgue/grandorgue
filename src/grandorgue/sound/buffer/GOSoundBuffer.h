/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFER_H
#define GOSOUNDBUFFER_H

#include <cassert>

/**
 * Basic read-only class for manipulation with a memory buffer of sound data.
 *
 * An item is a single value of the data type used; ie a float for GrandOrgue
 * audio output. A frame contains an item for each channel.
 *
 * We assume that the buffer has interleaved format: it consists of nFrames
 * consecutive multichannel frames. One multichannel frame consists of
 * nChannels consecutive sound items. By default, the sound item is float.
 *
 * The class is a non-owning wrapper around existing memory. Multiple
 * GOSoundBuffer objects can point to the same memory region (shared data).
 * Copying creates a shallow copy that shares the underlying data.
 */

class GOSoundBuffer {
public:
  // Currently we work with float items
  using Item = float;

  /**
   * Calculate total number of sound items for given number of channels and
   * frames.
   * @param nChannels Number of channels
   * @param nFrames Number of frames
   * @return Total number of sound items (nChannels * nFrames)
   */
  static inline unsigned getNItems(unsigned nChannels, unsigned nFrames) {
    return nChannels * nFrames;
  }

private:
  friend class GOSoundBufferMutable;

  // Points to the memory buffer of nChannels * nFrames sound items
  const Item *p_data;
  // Number of sound items per frame
  unsigned m_NChannels;
  // Number of frames in the buffer
  unsigned m_NFrames;

public:
  // Constructor for read-only buffer
  inline GOSoundBuffer(const Item *pData, unsigned nChannels, unsigned nFrames)
    : p_data(pData), m_NChannels(nChannels), m_NFrames(nFrames) {}

  inline const Item *GetData() const { return p_data; }
  inline unsigned GetNChannels() const { return m_NChannels; }
  inline unsigned GetNFrames() const { return m_NFrames; }

  inline bool isValid() const {
    return m_NChannels > 0 && m_NFrames > 0 && p_data;
  }

  /**
   * Calculate the index of a specific item in the buffer.
   * @param frameIndex Index of the frame from the beginning of the buffer
   * @param channelN Channel number within the frame (0-based)
   * @return Index of the sound item from the beginning of the buffer
   */
  inline unsigned GetItemIndex(
    unsigned frameIndex, unsigned channelN = 0) const {
    // For safety, we could add assertions here:
    // assert(frameIndex < m_NFrames);
    // assert(channelN < m_NChannels);
    return getNItems(m_NChannels, frameIndex) + channelN;
  }

  /**
   * Get total number of sound items in this buffer.
   * @return Total number of sound items (GetNChannels() * GetNFrames())
   */
  inline unsigned GetNItems() const {
    return getNItems(m_NChannels, m_NFrames);
  }

  /**
   * Get size of the buffer in bytes.
   * @return Size in bytes (sizeof(Item) * GetNItems())
   */
  inline unsigned GetNBytes() const { return sizeof(Item) * GetNItems(); }

  /**
   * Create a sub-buffer that shares the same memory region starting from
   * frameIndex.
   * @param frameIndex Index of the first frame in this buffer
   * @param nFrames Number of frames in the sub-buffer
   * @return A new buffer object pointing to a subset of the original data
   */
  inline GOSoundBuffer GetSubBuffer(
    unsigned frameIndex, unsigned nFrames) const {
    assert(frameIndex + nFrames <= m_NFrames);
    return GOSoundBuffer(
      p_data + GetItemIndex(frameIndex), m_NChannels, nFrames);
  }

protected:
  // Subclasses may create an invalid instance and then they may call Assign
  inline GOSoundBuffer() : p_data(nullptr), m_NChannels(0), m_NFrames(0) {}

  // Some subclasses may switch to another memory buffer
  inline void Assign(const Item *pData, unsigned nChannels, unsigned nFrames) {
    p_data = pData;
    m_NChannels = nChannels;
    m_NFrames = nFrames;
  }
};

#endif /* GOSOUNDBUFFER_H */
