/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERMUTABLEMONO_H
#define GOSOUNDBUFFERMUTABLEMONO_H

#include "GOSoundBuffer.h"
#include "GOSoundBufferMutable.h"

/**
 * Specialized mutable buffer for mono operations.
 * Provides methods for working with single channel data.
 */
class GOSoundBufferMutableMono : public GOSoundBufferMutable {
private:
  /**
   * Check if this buffer is compatible with another buffer.
   * For mono operations, we require same number of samples.
   */
  inline void AssertCompatibleWith(const GOSoundBuffer &otherBuffer) const {
    assert(isValid());
    assert(otherBuffer.isValid());
    assert(otherBuffer.GetNSamples() == GetNSamples());
    // Mono buffer should have exactly 1 channel
    assert(GetNChannels() == 1);
  }

public:
  // Constructor for mono mutable buffer
  inline GOSoundBufferMutableMono(SoundUnit *pData, unsigned nSamples)
    : GOSoundBufferMutable(pData, 1, nSamples) {}

protected:
  // Subclasses may create an invalid instance and then they may call Assign
  inline GOSoundBufferMutableMono() : GOSoundBufferMutable() {}

public:
  // Override GetSubBuffer to return mono mutable version
  inline GOSoundBufferMutableMono GetSubBuffer(
    unsigned offset, unsigned nSamples) {
    assert(offset + nSamples <= GetNSamples());
    return GOSoundBufferMutableMono(
      GetData() + offset, // Mono buffer: offset in samples = offset in units
      nSamples);
  }

  /**
   * Copy data from a specific channel of a multi-channel source buffer.
   * This mono buffer must have the same number of samples as the source buffer.
   * @param srcBuffer Source buffer to copy from
   * @param srcChannel Channel number in source buffer to copy from (0-based)
   */
  inline void CopyChannelFrom(
    const GOSoundBuffer &srcBuffer, unsigned srcChannel) {
    AssertCompatibleWith(srcBuffer);
    assert(srcChannel < srcBuffer.GetNChannels());

    GOSoundBufferMutable::CopyChannelFrom(srcBuffer, srcChannel, 0);
  }

  /**
   * Copy mono data to a specific channel of a multi-channel destination buffer.
   * This mono buffer must have the same number of samples as the destination
   * buffer.
   * @param dstBuffer Destination buffer to copy to
   * @param dstChannel Channel number in destination buffer to copy to (0-based)
   */
  inline void CopyChannelTo(
    GOSoundBufferMutable &dstBuffer, unsigned dstChannel) {
    AssertCompatibleWith(dstBuffer);
    assert(dstChannel < dstBuffer.GetNChannels());

    dstBuffer.CopyChannelFrom(*this, 0, dstChannel);
  }
};

#endif /* GOSOUNDBUFFERMUTABLEMONO_H */
