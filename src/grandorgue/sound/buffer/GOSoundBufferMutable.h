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
#include <initializer_list>

/**
 * Mutable version of GOSoundBuffer that allows modification of the buffer.
 *
 * It also has some methods for manipulating sound in the buffer
 */
class GOSoundBufferMutable : public GOSoundBuffer {
public:
  /**
   * Asserts that two buffers (interleaved or planar, any mix) are valid and
   * have the same number of channels and frames. Shared by
   * GOSoundBufferMutable and GOSoundBufferPlanarMutable so the whole-buffer
   * compatibility check is not duplicated between the interleaved and
   * planar hierarchies.
   * @param bufferA First buffer to check
   * @param bufferB Second buffer to check
   */
  template <typename BufferAType, typename BufferBType>
  static inline void assertBuffersCompatible(
    const BufferAType &bufferA, const BufferBType &bufferB) {
    assert(bufferA.isValid());
    assert(bufferB.isValid());
    assert(bufferA.GetNChannels() == bufferB.GetNChannels());
    assert(bufferA.GetNFrames() == bufferB.GetNFrames());
  }

  /**
   * Asserts that a per-channel copy/add between two buffers (interleaved or
   * planar, any mix) is valid: both buffers are valid, they have the same
   * number of frames, and the channel indices are in range. Shared by
   * GOSoundBufferMutable and GOSoundBufferPlanarMutable.
   * @param dstBuffer Destination buffer
   * @param srcBuffer Source buffer
   * @param srcChannelI Source channel index (0-based)
   * @param dstChannelI Destination channel index (0-based)
   */
  template <typename BufferAType, typename BufferBType>
  static inline void assertChannelsCompatible(
    const BufferAType &dstBuffer,
    const BufferBType &srcBuffer,
    unsigned srcChannelI,
    unsigned dstChannelI) {
    assert(dstBuffer.isValid());
    assert(srcBuffer.isValid());
    assert(srcBuffer.GetNFrames() == dstBuffer.GetNFrames());
    assert(srcChannelI < srcBuffer.GetNChannels());
    assert(dstChannelI < dstBuffer.GetNChannels());
  }

  /**
   * Fills the entire buffer (all channels) with zeros (silence). Shared by
   * GOSoundBufferMutable and GOSoundBufferPlanarMutable.
   * @param buffer Buffer to fill with silence
   */
  template <typename BufferType>
  static inline void fillWithSilence(BufferType &buffer) {
    assert(buffer.isValid());
    std::memset(buffer.GetData(), 0, buffer.GetNBytes());
  }

  /**
   * Copies audio data from one buffer into another of the same shape (both
   * interleaved or both planar). Shared by GOSoundBufferMutable and
   * GOSoundBufferPlanarMutable.
   * @param dstBuffer Destination buffer
   * @param srcBuffer Source buffer, same shape as dstBuffer
   */
  template <typename BufferAType, typename BufferBType>
  static inline void copyFrom(
    BufferAType &dstBuffer, const BufferBType &srcBuffer) {
    assertBuffersCompatible(dstBuffer, srcBuffer);
    std::memcpy(
      dstBuffer.GetData(), srcBuffer.GetData(), dstBuffer.GetNBytes());
  }

  /**
   * Adds audio data from one buffer into another of the same shape,
   * scaled by coeff. Shared by GOSoundBufferMutable and
   * GOSoundBufferPlanarMutable.
   * @param dstBuffer Destination buffer
   * @param srcBuffer Source buffer, same shape as dstBuffer
   * @param coeff Multiply source items by this coefficient before adding
   */
  template <typename BufferAType, typename BufferBType>
  static inline void addFrom(
    BufferAType &dstBuffer, const BufferBType &srcBuffer, float coeff) {
    assertBuffersCompatible(dstBuffer, srcBuffer);

    // Take the pointers into a register cache for better performance
    const typename BufferAType::Item *__restrict pSrc = srcBuffer.GetData();
    typename BufferAType::Item *__restrict pDst = dstBuffer.GetData();

    // The compiler should auto-vectorize this loop
    for (unsigned i = dstBuffer.GetNItems(); i; i--)
      *pDst++ += *pSrc++ * coeff;
  }

  /**
   * Adds audio data from one buffer into another of the same shape. Shared
   * by GOSoundBufferMutable and GOSoundBufferPlanarMutable.
   * @param dstBuffer Destination buffer
   * @param srcBuffer Source buffer, same shape as dstBuffer
   */
  template <typename BufferAType, typename BufferBType>
  static inline void addFrom(
    BufferAType &dstBuffer, const BufferBType &srcBuffer) {
    assertBuffersCompatible(dstBuffer, srcBuffer);

    // Take the pointers into a register cache for better performance
    const typename BufferAType::Item *__restrict pSrc = srcBuffer.GetData();
    typename BufferAType::Item *__restrict pDst = dstBuffer.GetData();

    // The compiler should auto-vectorize this loop
    for (unsigned i = dstBuffer.GetNItems(); i; i--)
      *pDst++ += *pSrc++;
  }

private:
  inline void AssertChannelCompatibilityWith(
    const GOSoundBuffer &srcBuffer,
    unsigned srcChannel,
    unsigned dstChannel) const {
    assertChannelsCompatible(*this, srcBuffer, srcChannel, dstChannel);
  }

public:
  // Constructor for mutable buffer
  inline GOSoundBufferMutable(Item *pData, unsigned nChannels, unsigned nFrames)
    : GOSoundBuffer(pData, nChannels, nFrames) {}

  inline GOSoundBufferMutable(const GOSoundBufferMutable &srcBuffer) = default;

protected:
  // Subclasses may create an invalid instance and then they may call Assign
  inline GOSoundBufferMutable() : GOSoundBuffer() {}

public:
  // Un-hide the const overload of GetData() inherited from GOSoundBuffer,
  // so calling GetData() on a const GOSoundBufferMutable (or subclass)
  // reference resolves to it instead of failing to find an accessible
  // overload.
  using GOSoundBuffer::GetData;

  // Get mutable pointer to data
  inline Item *GetData() { return const_cast<Item *>(p_data); }

  // Override SubBuffer to return mutable version
  inline GOSoundBufferMutable GetSubBuffer(
    unsigned firstFrameIndex, unsigned nFrames) {
    assert(firstFrameIndex + nFrames <= m_NFrames);
    return GOSoundBufferMutable(
      const_cast<Item *>(p_data + GetItemIndex(firstFrameIndex)),
      m_NChannels,
      nFrames);
  }

  /**
   * Fill the entire buffer with zeros (silence).
   */
  inline void FillWithSilence() { fillWithSilence(*this); }

  /**
   * Copy audio data from another buffer.
   * Both buffers must have the same number of channels and frames.
   * @param srcBuffer Source buffer to copy from
   */
  inline void CopyFrom(const GOSoundBuffer &srcBuffer) {
    copyFrom(*this, srcBuffer);
  }

  /**
   * Fill the buffer with values from an initializer list.
   * The number of values in the list must match the buffer's total items.
   *
   * This method is usually used in tests for more compaction.
   *
   * @param values Initializer list of float values
   */
  inline void FillWith(std::initializer_list<float> values) {
    assert(values.size() == GetNItems());
    CopyFrom(GOSoundBuffer(values.begin(), m_NChannels, m_NFrames));
  }

  /**
   * Add audio data from another buffer with coefficient.
   * Both buffers must have the same number of channels and frames.
   *
   * @param srcBuffer Source buffer to add from
   * @param coeff Multiply source frames by this coefficient before adding
   */
  inline void AddFrom(const GOSoundBuffer &srcBuffer, float coeff) {
    addFrom(*this, srcBuffer, coeff);
  }

  /**
   * Add audio data from another buffer.
   * Both buffers must have the same number of channels and frames.
   *
   * @param srcBuffer Source buffer to add from
   */
  inline void AddFrom(const GOSoundBuffer &srcBuffer) {
    addFrom(*this, srcBuffer);
  }

  /**
   * Copy audio data from one channel of another buffer to one channel of this
   * buffer. Both buffers must have the same number of frames.
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
    const Item *__restrict pSrc = srcBuffer.p_data + srcChannel;
    Item *__restrict pDst = const_cast<Item *>(p_data) + dstChannel;
    const unsigned srcNChannels = srcBuffer.m_NChannels;
    const unsigned dstNChannels = m_NChannels;

    for (unsigned i = m_NFrames; i; i--) {
      *pDst = *pSrc;
      pDst += dstNChannels;
      pSrc += srcNChannels;
    }
  }

  /**
   * Add audio data from one channel of another buffer to one channel of this
   * buffer with coefficient. Both buffers must have the same number of frames.
   *
   * @param srcBuffer Source buffer to add from
   * @param srcChannel Source channel index (0-based)
   * @param dstChannel Destination channel index (0-based)
   * @param coeff Multiply source frames by this coefficient before adding
   */
  inline void AddChannelFrom(
    const GOSoundBuffer &srcBuffer,
    unsigned srcChannel,
    unsigned dstChannel,
    float coeff) {
    AssertChannelCompatibilityWith(srcBuffer, srcChannel, dstChannel);

    // Take the number and the pointers into a register cache for better
    // performance
    const Item *__restrict pSrc = srcBuffer.p_data + srcChannel;
    Item *__restrict pDst = const_cast<Item *>(p_data) + dstChannel;
    const unsigned srcNChannels = srcBuffer.m_NChannels;
    const unsigned dstNChannels = m_NChannels;

    for (unsigned i = m_NFrames; i; --i) {
      *pDst += *pSrc * coeff;
      pSrc += srcNChannels;
      pDst += dstNChannels;
    }
  }

  /**
   * Add audio data from one channel of another buffer to one channel of this
   * buffer. Both buffers must have the same number of frames.
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
    const Item *__restrict pSrc = srcBuffer.p_data + srcChannel;
    Item *__restrict pDst = const_cast<Item *>(p_data) + dstChannel;
    const unsigned srcNChannels = srcBuffer.m_NChannels;
    const unsigned dstNChannels = m_NChannels;

    for (unsigned i = m_NFrames; i; i--) {
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
 * nFrames sound items, and a GOSoundBufferMutable object named <varName> that
 * wraps this array.
 *
 * ATTENTION: Don't create too large buffers on the stack to avoid stack
 * overflows
 *
 * @param varName The name of the GOSoundBufferMutable variable to declare
 * @param nChannels Number of audio channels
 * @param nFrames Number of frames per channel
 *
 * Example:
 *   GO_DECLARE_LOCAL_SOUND_BUFFER(myBuffer, 2, 480);
 *   // Creates:
 *   // - GOSoundBuffer::Item myBufferMemory[960];
 *   // - GOSoundBufferMutable myBuffer(myBufferMemory, 2, 480);
 */
#define GO_DECLARE_LOCAL_SOUND_BUFFER(varName, nChannels, nFrames)             \
  GOSoundBuffer::Item                                                          \
    varName##Memory[GOSoundBuffer::getNItems(nChannels, nFrames)];             \
  GOSoundBufferMutable varName(varName##Memory, nChannels, nFrames);

#endif /* GOSOUNDBUFFERMUTABLE_H */