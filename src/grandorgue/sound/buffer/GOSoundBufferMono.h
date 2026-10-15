/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERMONO_H
#define GOSOUNDBUFFERMONO_H

#include "GOSoundBuffer.h"

/**
 * Read-only, single-channel view of sound data - the read-only counterpart
 * of GOSoundBufferMutableMono. Used to expose a "this is exactly one
 * curve/channel" contract in public signatures (e.g.
 * GOSoundBufferPlanar::GetChannelBuffer()), making a channel index
 * structurally unrepresentable at the call site rather than merely
 * unnecessary by convention.
 *
 * Design note: GOSoundBufferMutableMono derives from GOSoundBufferMutable,
 * not from this class - a shared parent would need virtual inheritance to
 * avoid a diamond over GOSoundBuffer, which is the wrong tool for a
 * value-like, non-virtual wrapper used in audio inner loops. The two mono
 * types are siblings instead; GOSoundBufferMono(const GOSoundBuffer &)
 * converts any single-channel buffer (including a sliced
 * GOSoundBufferMutableMono, which is-a GOSoundBuffer) in one step.
 *
 * A buffer that is only ever read from, even if the memory it wraps is
 * written elsewhere (e.g. by an external library through its own pointer,
 * as with a convolver's output block), should be typed as
 * GOSoundBufferMono, not GOSoundBufferMutableMono.
 */
class GOSoundBufferMono : public GOSoundBuffer {
public:
  // Constructor for a mono read-only buffer
  inline GOSoundBufferMono(const Item *pData, unsigned nFrames)
    : GOSoundBuffer(pData, 1, nFrames) {}

  inline GOSoundBufferMono(const GOSoundBufferMono &srcBuffer) = default;

  // Converts any single-channel GOSoundBuffer (including a sliced
  // GOSoundBufferMutableMono) into a read-only mono view.
  inline explicit GOSoundBufferMono(const GOSoundBuffer &srcBuffer)
    : GOSoundBuffer(srcBuffer) {
    assert(srcBuffer.GetNChannels() == 1);
  }

protected:
  // Subclasses may create an invalid instance and then call Assign
  inline GOSoundBufferMono() : GOSoundBuffer() {}

public:
  // Override GetSubBuffer to return the mono type
  inline GOSoundBufferMono GetSubBuffer(
    unsigned firstFrameIndex, unsigned nFrames) const {
    return GOSoundBufferMono(
      GOSoundBuffer::GetSubBuffer(firstFrameIndex, nFrames));
  }
};

#endif /* GOSOUNDBUFFERMONO_H */
