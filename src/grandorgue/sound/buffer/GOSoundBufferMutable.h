/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERMUTABLE_H
#define GOSOUNDBUFFERMUTABLE_H

#include "GOSoundBuffer.h"

#include <cstring> // For std::memset, std::memcpy

/**
 * Mutable version of GOSoundBuffer that allows modification of the buffer.
 *
 * It also has some methods for manipulating sound in the buffer
 */
class GOSoundBufferMutable : public GOSoundBuffer {
private:
  inline void AssertCompatibilityWith(const GOSoundBuffer &srcBuffer) const {
    assert(isValid());
    assert(srcBuffer.isValid());
    assert(srcBuffer.m_NChannels == m_NChannels);
    assert(srcBuffer.m_NSamples == m_NSamples);
  }

  inline void AssertChannelCompatibilityWith(
    const GOSoundBuffer &srcBuffer,
    unsigned srcChannel,
    unsigned dstChannel) const {
    assert(isValid());
    assert(srcBuffer.isValid());
    assert(srcBuffer.m_NSamples == m_NSamples);
    assert(srcChannel < srcBuffer.m_NChannels);
    assert(dstChannel < m_NChannels);
  }

public:
  // Constructor for mutable buffer
  inline GOSoundBufferMutable(
    SoundUnit *pData, unsigned nChannels, unsigned nSamples)
    : GOSoundBuffer(pData, nChannels, nSamples) {}

protected:
  // Subclasses may create an invalid instance and then they may call Assign
  inline GOSoundBufferMutable() : GOSoundBuffer() {}

public:
  // Get mutable pointer to data
  inline SoundUnit *GetData() { return const_cast<SoundUnit *>(p_data); }

  // Override SubBuffer to return mutable version
  inline GOSoundBufferMutable GetSubBuffer(unsigned offset, unsigned nSamples) {
    assert(offset + nSamples <= m_NSamples);
    return GOSoundBufferMutable(
      const_cast<SoundUnit *>(p_data + GetUnitOffset(offset)),
      m_NChannels,
      nSamples);
  }

  /**
   * Fill the entire buffer with zeros (silence).
   */
  inline void FillWithSilence() {
    assert(isValid());
    std::memset(const_cast<SoundUnit *>(p_data), 0, GetNBytes());
  }

  /**
   * Copy audio data from another buffer.
   * Both buffers must have the same number of channels and samples.
   * @param srcBuffer Source buffer to copy from
   */
  inline void CopyFrom(const GOSoundBuffer &srcBuffer) {
    AssertCompatibilityWith(srcBuffer);
    std::memcpy(const_cast<SoundUnit *>(p_data), srcBuffer.p_data, GetNBytes());
  }

  /**
   * Add audio data from another buffer with coefficient.
   * Both buffers must have the same number of channels and samples.
   *
   * @param srcBuffer Source buffer to add from
   * @param coeff Multiply source samples by this coefficient before adding
   */
  inline void AddFrom(const GOSoundBuffer &srcBuffer, float coeff) {
    AssertCompatibilityWith(srcBuffer);

    // Take the pointers into a register cache for better performance
    const SoundUnit *__restrict pSrc = srcBuffer.p_data;
    SoundUnit *__restrict pDst = const_cast<SoundUnit *>(p_data);

    // The compiler should auto-vectorize this loop
    for (unsigned i = GetNUnits(); i; i--)
      *pDst++ += *pSrc++ * coeff;
  }

  /**
   * Add audio data from another buffer.
   * Both buffers must have the same number of channels and samples.
   *
   * @param srcBuffer Source buffer to add from
   */
  inline void AddFrom(const GOSoundBuffer &srcBuffer) {
    AssertCompatibilityWith(srcBuffer);

    // Take the pointers into a register cache for better performance
    const SoundUnit *__restrict pSrc = srcBuffer.p_data;
    SoundUnit *__restrict pDst = const_cast<SoundUnit *>(p_data);

    // The compiler should auto-vectorize this loop
    for (unsigned i = GetNUnits(); i; i--)
      *pDst++ += *pSrc++;
  }

  /**
   * Copy audio data from one channel of another buffer to one channel of this
   * buffer. Both buffers must have the same number of samples.
   *
   * It is not implemented as a wrapper around the similar function with coeff=1
   * for better optimisation.
   *
   * @param srcBuffer Source buffer to copy from
   * @param srcChannel Source channel index (0-based)
   * @param dstChannel Destination channel index (0-based)
   */
  inline void CopyChannelFrom(
    const GOSoundBuffer &srcBuffer, unsigned srcChannel, unsigned dstChannel) {
    AssertChannelCompatibilityWith(srcBuffer, srcChannel, dstChannel);

    // Take the number and the pointers into a register cache for better
    // performance
    const SoundUnit *__restrict pSrc = srcBuffer.p_data + srcChannel;
    SoundUnit *__restrict pDst = const_cast<SoundUnit *>(p_data) + dstChannel;
    const unsigned srcNChannels = srcBuffer.m_NChannels;
    const unsigned dstNChannels = m_NChannels;

    for (unsigned i = m_NSamples; i; i--) {
      *pDst = *pSrc;
      pDst += dstNChannels;
      pSrc += srcNChannels;
    }
  }

  /**
   * Add audio data from one channel of another buffer to one channel of this
   * buffer with coefficient. Both buffers must have the same number of samples.
   *
   * @param srcBuffer Source buffer to add from
   * @param srcChannel Source channel index (0-based)
   * @param dstChannel Destination channel index (0-based)
   * @param coeff Multiply source samples by this coefficient before adding
   */
  inline void AddChannelFrom(
    const GOSoundBuffer &srcBuffer,
    unsigned srcChannel,
    unsigned dstChannel,
    float coeff) {
    AssertChannelCompatibilityWith(srcBuffer, srcChannel, dstChannel);

    // Take the number and the pointers into a register cache for better
    // performance
    const SoundUnit *__restrict pSrc = srcBuffer.p_data + srcChannel;
    SoundUnit *__restrict pDst = const_cast<SoundUnit *>(p_data) + dstChannel;
    const unsigned srcNChannels = srcBuffer.m_NChannels;
    const unsigned dstNChannels = m_NChannels;

    for (unsigned i = m_NSamples; i; --i) {
      *pDst += *pSrc * coeff;
      pSrc += srcNChannels;
      pDst += dstNChannels;
    }
  }

  /**
   * Add audio data from one channel of another buffer to one channel of this
   * buffer. Both buffers must have the same number of samples.
   *
   * It is not implemented as a wrapper around the similar function with coeff=1
   * for better optimisation.
   *
   * @param srcBuffer Source buffer to add from
   * @param srcChannel Source channel index (0-based)
   * @param dstChannel Destination channel index (0-based)
   */
  inline void AddChannelFrom(
    const GOSoundBuffer &srcBuffer, unsigned srcChannel, unsigned dstChannel) {
    AssertChannelCompatibilityWith(srcBuffer, srcChannel, dstChannel);

    // Take the number and the pointers into a register cache for better
    // performance
    const SoundUnit *__restrict pSrc = srcBuffer.p_data + srcChannel;
    SoundUnit *__restrict pDst = const_cast<SoundUnit *>(p_data) + dstChannel;
    const unsigned srcNChannels = srcBuffer.m_NChannels;
    const unsigned dstNChannels = m_NChannels;

    for (unsigned i = m_NSamples; i; i--) {
      *pDst += *pSrc;
      pDst += dstNChannels;
      pSrc += srcNChannels;
    }
  }
};

/**
 * Declares a local sound buffer on the stack.
 *
 * Creates an array named <varName>Memory with enough space for nChannels *
 * nSamples sound units, and a GOSoundBufferMutable object named <varName> that
 * wraps this array.
 *
 * ATTENTION: Don't create too large buffers on the stack to avoid stack
 * overflows
 *
 * @param varName The name of the GOSoundBufferMutable variable to declare
 * @param nChannels Number of audio channels
 * @param nSamples Number of samples per channel
 *
 * Example:
 *   GO_DECLARE_LOCAL_SOUND_BUFFER(myBuffer, 2, 480);
 *   // Creates:
 *   // - GOSoundBuffer::SoundUnit myBufferMemory[960];
 *   // - GOSoundBufferMutable myBuffer(myBufferMemory, 2, 480);
 */
#define GO_DECLARE_LOCAL_SOUND_BUFFER(varName, nChannels, nSamples)            \
  GOSoundBuffer::SoundUnit                                                     \
    varName##Memory[GOSoundBuffer::getNUnits(nChannels, nSamples)];            \
  GOSoundBufferMutable varName(varName##Memory, nChannels, nSamples)

#endif /* GOSOUNDBUFFERMUTABLE_H */