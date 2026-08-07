/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERMANAGEDBASE_H
#define GOSOUNDBUFFERMANAGEDBASE_H

#include <memory>

#include "GOSoundBuffer.h"

/**
 * Shared implementation of an owning (heap-allocated) sound buffer. Used as
 * the base of both GOSoundBufferManaged (interleaved,
 * MutableBaseType=GOSoundBufferMutable, ViewType=GOSoundBuffer) and
 * GOSoundBufferPlanarManaged (planar,
 * MutableBaseType=GOSoundBufferPlanarMutable, ViewType=GOSoundBufferPlanar), so
 * that Resize()/CopyDataFrom()/ MoveDataFrom()/Swap() are written once for both
 * memory layouts.
 *
 * Not meant to be used directly - GOSoundBufferManaged and
 * GOSoundBufferPlanarManaged are the public names of its two
 * instantiations, and add the Rule-of-5 constructors/assignment operators
 * with layout-specific names in their own signatures.
 *
 * @warning In real-time audio callbacks, heap allocation can cause latency
 * spikes. Use a stack-allocated buffer (GO_DECLARE_LOCAL_SOUND_BUFFER or
 * GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR) in such contexts when the buffer
 * size is bounded and small.
 */
template <typename MutableBaseType, typename ViewType>
class GOSoundBufferManagedBase : public MutableBaseType {
public:
  using Item = typename MutableBaseType::Item;

private:
  // The memory buffer that contains sound items. It is deleted automatically
  // when the instance is destroyed
  std::unique_ptr<Item[]> m_OwnedData;

protected:
  inline GOSoundBufferManagedBase() : MutableBaseType() {}

  /**
   * Copy data from source buffer (resizes this buffer to match source).
   * @param srcBuffer Source buffer to copy from
   */
  void CopyDataFrom(const ViewType &srcBuffer) {
    Resize(srcBuffer.GetNChannels(), srcBuffer.GetNFrames());
    // After Resize, both buffers have same validity: either both valid or
    // both invalid
    if (this->isValid())
      this->CopyFrom(srcBuffer);
  }

  /**
   * Move data from another GOSoundBufferManagedBase.
   * Helper for move constructor and move assignment operator.
   * @param other Buffer to move from (will be left in empty state)
   */
  void MoveDataFrom(GOSoundBufferManagedBase &other) noexcept {
    m_OwnedData = std::move(other.m_OwnedData);
    this->Assign(other.GetData(), other.GetNChannels(), other.GetNFrames());
    other.Assign(nullptr, 0, 0);
  }

public:
  /**
   * Resize the buffer to new dimensions.
   * If dimensions differ, memory is reallocated and all existing data in
   * the buffer is lost.
   *
   * @param nChannels New number of channels
   * @param nFrames New number of frames per channel
   */
  void Resize(unsigned nChannels, unsigned nFrames) {
    if (nChannels != this->GetNChannels() || nFrames != this->GetNFrames()) {
      unsigned newNItems = GOSoundBuffer::getNItems(nChannels, nFrames);

      // reallocate memory only if the size is changed
      if (newNItems != this->GetNItems()) {
        if (newNItems)
          m_OwnedData = std::make_unique<Item[]>(newNItems);
        else
          m_OwnedData.reset();
      }
      this->Assign(m_OwnedData.get(), nChannels, nFrames);
    }
  }

  /**
   * Swap contents with another GOSoundBufferManagedBase.
   * @param other Buffer to swap with
   */
  void Swap(GOSoundBufferManagedBase &other) noexcept {
    // Save current data of this buffer
    unsigned oldChannels = this->GetNChannels();
    unsigned oldFrames = this->GetNFrames();

    // Swap ownership of memory
    std::swap(m_OwnedData, other.m_OwnedData);

    // Update this buffer from new m_OwnedData
    this->Assign(m_OwnedData.get(), other.GetNChannels(), other.GetNFrames());

    // Update other buffer from new other.m_OwnedData
    other.Assign(other.m_OwnedData.get(), oldChannels, oldFrames);
  }
};

#endif /* GOSOUNDBUFFERMANAGEDBASE_H */
