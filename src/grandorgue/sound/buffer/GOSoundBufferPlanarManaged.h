/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERPLANARMANAGED_H
#define GOSOUNDBUFFERPLANARMANAGED_H

#include "GOSoundBufferManagedBase.h"
#include "GOSoundBufferPlanarMutable.h"

/**
 * Planar sound buffer that owns its memory (heap-allocated via unique_ptr).
 * Inherits from GOSoundBufferPlanarMutable and manages its own memory
 * allocation. Resize()/Swap() and the copy/move plumbing are implemented
 * once in GOSoundBufferManagedBase and shared with GOSoundBufferManaged;
 * this class only adds the Rule-of-5 constructors/assignment operators with
 * GOSoundBufferPlanar-specific signatures.
 *
 * Use this class when:
 * - The buffer must persist beyond the current scope (e.g. as a class member)
 * - The buffer is large (to avoid stack overflow)
 *
 * @warning In real-time audio callbacks, heap allocation can cause latency
 * spikes. Use GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR for stack-allocated
 * buffers in such contexts when the buffer size is bounded and small.
 */
class GOSoundBufferPlanarManaged : public GOSoundBufferManagedBase<
                                     GOSoundBufferPlanarMutable,
                                     GOSoundBufferPlanar> {
  using Base
    = GOSoundBufferManagedBase<GOSoundBufferPlanarMutable, GOSoundBufferPlanar>;

public:
  /**
   * Default constructor - creates empty invalid buffer.
   */
  inline GOSoundBufferPlanarManaged() : Base() {}

  /**
   * Constructor that allocates memory for the buffer.
   * @param nChannels Number of channels
   * @param nFrames Number of frames per channel
   */
  inline GOSoundBufferPlanarManaged(unsigned nChannels, unsigned nFrames)
    : Base() {
    Resize(nChannels, nFrames);
  }

  /**
   * Copy constructor from GOSoundBufferPlanar.
   * @param srcBuffer Source buffer to copy from
   */
  inline GOSoundBufferPlanarManaged(const GOSoundBufferPlanar &srcBuffer)
    : Base() {
    CopyDataFrom(srcBuffer);
  }

  /**
   * Copy constructor from another GOSoundBufferPlanarManaged.
   * This constructor is required because when a move constructor is
   * declared, the compiler does not automatically generate a copy
   * constructor (Rule of 5). Delegates to constructor from
   * GOSoundBufferPlanar.
   * @param other Buffer to copy from
   */
  inline GOSoundBufferPlanarManaged(const GOSoundBufferPlanarManaged &other)
    : GOSoundBufferPlanarManaged(
      static_cast<const GOSoundBufferPlanar &>(other)) {}

  /**
   * Move constructor.
   * @param other Buffer to move from
   */
  inline GOSoundBufferPlanarManaged(GOSoundBufferPlanarManaged &&other) noexcept
    : Base() {
    MoveDataFrom(other);
  }

  /**
   * Copy assignment operator from GOSoundBufferPlanar.
   * @param srcBuffer Source buffer to copy from
   * @return Reference to this buffer
   */
  inline GOSoundBufferPlanarManaged &operator=(
    const GOSoundBufferPlanar &srcBuffer) {
    if (this != &srcBuffer)
      CopyDataFrom(srcBuffer);
    return *this;
  }

  /**
   * Copy assignment operator from another GOSoundBufferPlanarManaged.
   * This operator is required because when a move assignment operator is
   * declared, the compiler does not automatically generate a copy
   * assignment operator (Rule of 5). Delegates to assignment operator from
   * GOSoundBufferPlanar.
   * @param other Buffer to copy from
   * @return Reference to this buffer
   */
  inline GOSoundBufferPlanarManaged &operator=(
    const GOSoundBufferPlanarManaged &other) {
    // Delegate to base class assignment operator
    return this->operator=(static_cast<const GOSoundBufferPlanar &>(other));
  }

  /**
   * Move assignment operator.
   * @param other Buffer to move from
   * @return Reference to this buffer
   */
  inline GOSoundBufferPlanarManaged &operator=(
    GOSoundBufferPlanarManaged &&other) noexcept {
    if (this != &other)
      MoveDataFrom(other);
    return *this;
  }
};

/**
 * Non-member swap function for compatibility with standard algorithms.
 */
inline void swap(
  GOSoundBufferPlanarManaged &a, GOSoundBufferPlanarManaged &b) noexcept {
  a.Swap(b);
}

#endif /* GOSOUNDBUFFERPLANARMANAGED_H */
