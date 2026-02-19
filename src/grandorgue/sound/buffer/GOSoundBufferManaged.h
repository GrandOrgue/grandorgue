/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERMANAGED_H
#define GOSOUNDBUFFERMANAGED_H

#include "GOSoundBufferMutable.h"

#include <memory>

/**
 * Sound buffer that owns its memory (heap-allocated via unique_ptr).
 * Inherits from GOSoundBufferMutable and manages its own memory allocation.
 *
 * Use this class when:
 * - The buffer must persist beyond the current scope (e.g. as a class member)
 * - The buffer is large (to avoid stack overflow)
 *
 * @warning In real-time audio callbacks, heap allocation can cause latency
 * spikes. Use GO_DECLARE_LOCAL_SOUND_BUFFER for stack-allocated buffers in
 * such contexts when the buffer size is bounded and small.
 */
class GOSoundBufferManaged : public GOSoundBufferMutable {
private:
  // The memory buffer that contains sound items. It is deleted automatically
  // when the instance is destroyed
  std::unique_ptr<Item[]> m_OwnedData;

  /**
   * Copy data from source buffer (resizes this buffer to match source).
   * @param srcBuffer Source buffer to copy from
   */
  void CopyDataFrom(const GOSoundBuffer &srcBuffer);

  /**
   * Move data from another GOSoundBufferManaged.
   * Helper for move constructor and move assignment operator.
   * @param other Buffer to move from (will be left in empty state)
   */
  void MoveDataFrom(GOSoundBufferManaged &other) noexcept;

public:
  /**
   * Resize the buffer to new dimensions.
   * If dimensions differ, memory is reallocated and all existing data in the
   * buffer is lost.
   *
   * @param nChannels New number of channels
   * @param nFrames New number of frames
   */
  void Resize(unsigned nChannels, unsigned nFrames);

  /**
   * Default constructor - creates empty invalid buffer.
   */
  inline GOSoundBufferManaged() : GOSoundBufferMutable() {}

  /**
   * Constructor that allocates memory for the buffer.
   * @param nChannels Number of channels
   * @param nFrames Number of frames
   */
  inline GOSoundBufferManaged(unsigned nChannels, unsigned nFrames)
    : GOSoundBufferMutable() {
    Resize(nChannels, nFrames);
  }

  /**
   * Copy constructor from GOSoundBuffer.
   * @param srcBuffer Source buffer to copy from
   */
  inline GOSoundBufferManaged(const GOSoundBuffer &srcBuffer)
    : GOSoundBufferMutable() {
    CopyDataFrom(srcBuffer);
  }

  /**
   * Copy constructor from another GOSoundBufferManaged.
   * This constructor is required because when a move constructor is declared,
   * the compiler does not automatically generate a copy constructor (Rule of
   * 5). Delegates to constructor from GOSoundBuffer.
   * @param other Buffer to copy from
   */
  inline GOSoundBufferManaged(const GOSoundBufferManaged &other)
    : GOSoundBufferManaged(static_cast<const GOSoundBuffer &>(other)) {}

  /**
   * Move constructor.
   * @param other Buffer to move from
   */
  inline GOSoundBufferManaged(GOSoundBufferManaged &&other) noexcept
    : GOSoundBufferMutable() {
    MoveDataFrom(other);
  }

  /**
   * Copy assignment operator from GOSoundBuffer.
   * @param srcBuffer Source buffer to copy from
   * @return Reference to this buffer
   */
  inline GOSoundBufferManaged &operator=(const GOSoundBuffer &srcBuffer) {
    if (this != &srcBuffer)
      CopyDataFrom(srcBuffer);
    return *this;
  }

  /**
   * Copy assignment operator from another GOSoundBufferManaged.
   * This operator is required because when a move assignment operator is
   * declared, the compiler does not automatically generate a copy assignment
   * operator (Rule of 5). Delegates to assignment operator from GOSoundBuffer.
   * @param other Buffer to copy from
   * @return Reference to this buffer
   */
  inline GOSoundBufferManaged &operator=(const GOSoundBufferManaged &other) {
    // Delegate to base class assignment operator
    return this->operator=(static_cast<const GOSoundBuffer &>(other));
  }

  /**
   * Move assignment operator.
   * @param other Buffer to move from
   * @return Reference to this buffer
   */
  inline GOSoundBufferManaged &operator=(
    GOSoundBufferManaged &&other) noexcept {
    if (this != &other)
      MoveDataFrom(other);
    return *this;
  }

  /**
   * Swap contents with another GOSoundBufferManaged.
   * @param other Buffer to swap with
   */
  void Swap(GOSoundBufferManaged &other) noexcept;
};

/**
 * Non-member swap function for compatibility with standard algorithms.
 */
inline void swap(GOSoundBufferManaged &a, GOSoundBufferManaged &b) noexcept {
  a.Swap(b);
}

#endif /* GOSOUNDBUFFERMANAGED_H */