/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERPLANARMUTABLE_H
#define GOSOUNDBUFFERPLANARMUTABLE_H

#include <cassert>

#include "GOSoundBufferMutableMono.h"
#include "GOSoundBufferPlanar.h"

/**
 * Mutable version of GOSoundBufferPlanar that allows modification of the
 * buffer, mirroring the relationship between GOSoundBufferMutable and
 * GOSoundBuffer.
 *
 * It also has methods for manipulating sound in the buffer: filling with
 * silence, copying/adding from another planar buffer or one of its
 * channels, and converting to/from an interleaved buffer. The whole-buffer
 * operations reuse GOSoundBufferMutable's shared static templates
 * (fillWithSilence(), copyFrom(), addFrom()); the per-channel operations
 * reuse them too, indirectly, by delegating to the mono view returned by
 * GetChannelBuffer(), whose CopyFrom()/AddFrom() are the very same
 * GOSoundBufferMutable methods (GOSoundBufferMutableMono adds nothing but
 * the interleaved-conversion helpers).
 */
class GOSoundBufferPlanarMutable : public GOSoundBufferPlanar {
public:
  /**
   * Constructor for a mutable planar buffer.
   * @param pData Pointer to nChannels * nFrames sound items, channel-major
   * @param nChannels Number of channels
   * @param nFrames Number of frames per channel
   */
  inline GOSoundBufferPlanarMutable(
    Item *pData, unsigned nChannels, unsigned nFrames)
    : GOSoundBufferPlanar(pData, nChannels, nFrames) {}

  inline GOSoundBufferPlanarMutable(const GOSoundBufferPlanarMutable &srcBuffer)
    = default;

protected:
  // Subclasses may create an invalid instance and then they may call Assign
  inline GOSoundBufferPlanarMutable() : GOSoundBufferPlanar() {}

public:
  // Un-hide the const overload of GetData() inherited from
  // GOSoundBufferPlanar, so calling GetData() on a const
  // GOSoundBufferPlanarMutable (or subclass) reference resolves to it
  // instead of failing to find an accessible overload.
  using GOSoundBufferPlanar::GetData;

  /** @return Mutable pointer to the underlying channel-major data */
  inline Item *GetData() { return const_cast<Item *>(p_data); }

  /**
   * A contiguous mutable view of one channel; hides the read-only
   * GOSoundBufferPlanar::GetChannelBuffer() the same way GetSubBuffer() is
   * hidden today.
   * @param channelI Channel index (0-based)
   * @return A mutable view of the frames of the given channel
   */
  inline GOSoundBufferMutableMono GetChannelBuffer(unsigned channelI) {
    assert(channelI < GetNChannels());
    return GOSoundBufferMutableMono(
      GetData() + channelI * GetNFrames(), GetNFrames());
  }

  /** Fill the entire buffer (all channels) with zeros (silence). */
  inline void FillWithSilence() {
    GOSoundBufferMutable::fillWithSilence(*this);
  }

  /**
   * Scatters an interleaved buffer of the same shape into this one.
   * @param srcBuffer Interleaved source buffer, same nChannels and nFrames
   *   as this buffer
   */
  inline void DeinterleaveFrom(const GOSoundBuffer &srcBuffer) {
    GOSoundBufferMutable::assertBuffersCompatible(*this, srcBuffer);

    for (unsigned channelI = 0, n = GetNChannels(); channelI < n; ++channelI)
      GetChannelBuffer(channelI).CopyChannelFrom(srcBuffer, channelI);
  }

  /**
   * Scatters an interleaved buffer of the same shape into this one, adding
   * to what is already there.
   * @param srcBuffer Interleaved source buffer, same nChannels and nFrames
   *   as this buffer
   */
  inline void AddDeinterleavedFrom(const GOSoundBuffer &srcBuffer) {
    GOSoundBufferMutable::assertBuffersCompatible(*this, srcBuffer);

    for (unsigned channelI = 0, n = GetNChannels(); channelI < n; ++channelI)
      GetChannelBuffer(channelI).AddChannelFrom(srcBuffer, channelI);
  }

  /**
   * Copy audio data from another planar buffer.
   * Both buffers must have the same number of channels and frames.
   * @param srcBuffer Source buffer to copy from
   */
  inline void CopyFrom(const GOSoundBufferPlanar &srcBuffer) {
    GOSoundBufferMutable::copyFrom(*this, srcBuffer);
  }

  /**
   * Copy audio data from one channel of another planar buffer to one
   * channel of this buffer. Both buffers must have the same number of
   * frames.
   * @param srcBuffer Source buffer to copy from
   * @param srcChannelI Source channel index (0-based)
   * @param dstChannelI Destination channel index (0-based) in this buffer
   */
  inline void CopyChannelFrom(
    const GOSoundBufferPlanar &srcBuffer,
    unsigned srcChannelI,
    unsigned dstChannelI) {
    GetChannelBuffer(dstChannelI)
      .CopyFrom(srcBuffer.GetChannelBuffer(srcChannelI));
  }

  /**
   * Add audio data from another planar buffer.
   * Both buffers must have the same number of channels and frames.
   * @param srcBuffer Source buffer to add from
   */
  inline void AddFrom(const GOSoundBufferPlanar &srcBuffer) {
    GOSoundBufferMutable::addFrom(*this, srcBuffer);
  }

  /**
   * Add audio data from another planar buffer with coefficient.
   * Both buffers must have the same number of channels and frames.
   * @param srcBuffer Source buffer to add from
   * @param coeff Multiply source frames by this coefficient before adding
   */
  inline void AddFrom(const GOSoundBufferPlanar &srcBuffer, float coeff) {
    GOSoundBufferMutable::addFrom(*this, srcBuffer, coeff);
  }

  /**
   * Add audio data from one channel of another planar buffer to one channel
   * of this buffer. Both buffers must have the same number of frames.
   * @param srcBuffer Source buffer to add from
   * @param srcChannelI Source channel index (0-based)
   * @param dstChannelI Destination channel index (0-based) in this buffer
   */
  inline void AddChannelFrom(
    const GOSoundBufferPlanar &srcBuffer,
    unsigned srcChannelI,
    unsigned dstChannelI) {
    GetChannelBuffer(dstChannelI)
      .AddFrom(srcBuffer.GetChannelBuffer(srcChannelI));
  }

  /**
   * Add audio data from one channel of another planar buffer to one channel
   * of this buffer with coefficient. Both buffers must have the same number
   * of frames.
   * @param srcBuffer Source buffer to add from
   * @param srcChannelI Source channel index (0-based)
   * @param dstChannelI Destination channel index (0-based) in this buffer
   * @param coeff Multiply source frames by this coefficient before adding
   */
  inline void AddChannelFrom(
    const GOSoundBufferPlanar &srcBuffer,
    unsigned srcChannelI,
    unsigned dstChannelI,
    float coeff) {
    GetChannelBuffer(dstChannelI)
      .AddFrom(srcBuffer.GetChannelBuffer(srcChannelI), coeff);
  }
};

/**
 * Declares a local planar sound buffer on the stack.
 *
 * Creates an array named <varName>Memory with enough space for nChannels *
 * nFrames sound items, and a GOSoundBufferPlanarMutable object named
 * <varName> that wraps this array, channel-major.
 *
 * ATTENTION: Don't create too large buffers on the stack to avoid stack
 * overflows
 *
 * @param varName The name of the GOSoundBufferPlanarMutable variable to
 *   declare
 * @param nChannels Number of audio channels
 * @param nFrames Number of frames per channel
 *
 * Example:
 *   GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(myBuffer, 2, 480);
 *   // Creates:
 *   // - GOSoundBufferPlanar::Item myBufferMemory[960];
 *   // - GOSoundBufferPlanarMutable myBuffer(myBufferMemory, 2, 480);
 */
#define GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(varName, nChannels, nFrames)      \
  GOSoundBufferPlanar::Item                                                    \
    varName##Memory[GOSoundBuffer::getNItems(nChannels, nFrames)];             \
  GOSoundBufferPlanarMutable varName(varName##Memory, nChannels, nFrames);

#endif /* GOSOUNDBUFFERPLANARMUTABLE_H */
