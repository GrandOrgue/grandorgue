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
 * Basic read-only class for manipulation with a memory buffer of sound samples.
 *
 * We assume that the buffer has interleaved format: it consists of nSamples
 * consecutive multichannel sound samples. One multichannel sample consists of
 * nChannels consecutive sound units. Currently, the sound unit is float.
 *
 * The class is a non-owning wrapper around existing memory. Multiple
 * GOSoundBuffer objects can point to the same memory region (shared data).
 * Copying creates a shallow copy that shares the underlying data.
 */

class GOSoundBuffer {
public:
  // Currently we work with float units
  using SoundUnit = float;

  /**
   * Calculate total number of sound units for given number of channels and
   * samples.
   * @param nChannels Number of channels
   * @param nSamples Number of samples
   * @return Total number of sound units (nChannels * nSamples)
   */
  static inline unsigned getNUnits(unsigned nChannels, unsigned nSamples) {
    return nChannels * nSamples;
  }

private:
  friend class GOSoundBufferMutable;

  // Points to the memory buffer of nChannels * nSamples sound units
  const SoundUnit *p_data;
  // Number of sound units in one sample
  unsigned m_NChannels;
  // Number of samples in the buffer
  unsigned m_NSamples;

public:
  // Constructor for read-only buffer
  inline GOSoundBuffer(
    const SoundUnit *pData, unsigned nChannels, unsigned nSamples)
    : p_data(pData), m_NChannels(nChannels), m_NSamples(nSamples) {}

  inline const SoundUnit *GetData() const { return p_data; }
  inline unsigned GetNChannels() const { return m_NChannels; }
  inline unsigned GetNSamples() const { return m_NSamples; }

  inline bool isValid() const {
    return m_NChannels > 0 && m_NSamples > 0 && p_data;
  }

  /**
   * Calculate the offset of a specific unit in the buffer.
   * @param sampleOffset Offset in samples from the beginning of the buffer
   * @param channelN Channel number within the sample (0-based)
   * @return Offset in sound units from the beginning of the buffer
   */
  inline unsigned GetUnitOffset(
    unsigned sampleOffset, unsigned channelN = 0) const {
    // For safety, we could add assertions here:
    // assert(sampleOffset < m_NSamples);
    // assert(channelN < m_NChannels);
    return getNUnits(m_NChannels, sampleOffset) + channelN;
  }

  /**
   * Get total number of sound units in this buffer.
   * @return Total number of sound units (GetNChannels() * GetNSamples())
   */
  inline unsigned GetNUnits() const {
    return getNUnits(m_NChannels, m_NSamples);
  }

  /**
   * Get size of buffer in bytes.
   * @return Size in bytes (sizeof(SoundUnit) * GetNUnits())
   */
  inline unsigned GetNBytes() const { return sizeof(SoundUnit) * GetNUnits(); }

  /**
   * Create a sub-buffer that shares the same memory region starting from
   * offset.
   * @param offset Offset in samples from the beginning of this buffer
   * @param nSamples Number of samples in the sub-buffer
   * @return A new buffer object pointing to a subset of the original data
   */
  inline GOSoundBuffer GetSubBuffer(unsigned offset, unsigned nSamples) const {
    assert(offset + nSamples <= m_NSamples);
    return GOSoundBuffer(p_data + GetUnitOffset(offset), m_NChannels, nSamples);
  }

protected:
  // Subclasses may create an invalid instance and then they may call Assign
  inline GOSoundBuffer() : p_data(nullptr), m_NChannels(0), m_NSamples(0) {}

  // Some subclasses may switch to another memory buffer
  inline void Assign(
    const SoundUnit *pData, unsigned nChannels, unsigned nSamples) {
    p_data = pData;
    m_NChannels = nChannels;
    m_NSamples = nSamples;
  }
};

#endif /* GOSOUNDBUFFER_H */
