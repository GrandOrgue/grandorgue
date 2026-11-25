/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDITEMACCESSOR_H
#define GOSOUNDITEMACCESSOR_H

#include <stdint.h>
#include <type_traits>

#include "go_compiler.h"

#include "GOFloat32.h"
#include "GOInt.h"
#include "GOWave.h"

/**
 * Reads and writes single items (the value of one channel of one frame) of
 * interleaved sample data of a given format. The data may be unaligned.
 *
 * The format is described by bitsPerSample and isFloat:
 * - up to 8 bits: GOInt8; up to 16: GOInt16; up to 24: GOInt24;
 * - above 24 bits: GOFloat32 if isFloat, otherwise GOInt32.
 *
 * Two kinds of units are supported:
 * - int units: an integer of the section's own bit depth; for a float format
 *   it is Q23 (see GOWave::floatToIntQ23), rounded and clamped to full scale;
 * - float units: for a float format the item itself; for an integer format
 *   the int item converted to float (NOT scaled to +-1.0).
 *
 * The object describes only the format, not the data: the same object is used
 * with any data of that format. It is intended for the loading and preparation
 * of sections. It must not be used in the audio callback or in the hot paths
 * with a compile-time known channel count (the GOSoundStream decoders).
 */
class GOSoundItemAccessor {
private:
  const uint8_t m_BitsPerSample;
  const bool m_IsFloat;
  const uint8_t m_NChannels;

  /**
   * @tparam ItemT the item type: GOInt8, GOInt16, GOInt24, GOInt32 or GOFloat32
   * @param pData the data
   * @param frameI the frame index
   * @param nChannels the number of channels
   * @param channelI the channel index
   * @return the item at the position
   */
  template <typename ItemT>
  static ItemT getItem(
    const void *pData, unsigned frameI, uint8_t nChannels, uint8_t channelI) {
    return static_cast<const ItemT *>(pData)[frameI * nChannels + channelI];
  }

  /**
   * Stores the value into the item at the position.
   * @param value the new item value
   * other parameters as for getItem
   */
  template <typename ItemT>
  static void setItem(
    void *pData,
    unsigned frameI,
    uint8_t nChannels,
    uint8_t channelI,
    ItemT value) {
    static_cast<ItemT *>(pData)[frameI * nChannels + channelI] = value;
  }

public:
  /**
   * Reads an item of a known type in int units. Intended for loops that
   * dispatched the item type once with DispatchItemType().
   * @tparam ItemT the item type: GOInt8, GOInt16, GOInt24, GOInt32 or
   *   GOFloat32
   * @param pData the data
   * @param frameI the frame index
   * @param nChannels the number of channels
   * @param channelI the channel index
   * @return the item in int units (a float is rounded to Q23)
   */
  template <typename ItemT>
  GO_FORCE_INLINE static int getIntItem(
    const void *pData, unsigned frameI, uint8_t nChannels, uint8_t channelI) {
    const ItemT item = getItem<ItemT>(pData, frameI, nChannels, channelI);

    if constexpr (std::is_same_v<ItemT, GOFloat32>)
      return GOWave::floatToIntQ23(item);
    else
      return item;
  }

  /**
   * Calls fCall.template operator()<ItemT>() with the item type of the format.
   * @tparam FuncT a callable with a template call operator
   * @param fCall the callable; it returns its result through captured
   *   variables
   */
  template <typename FuncT> void DispatchItemType(FuncT &&fCall) const {
    if (m_BitsPerSample <= 8)
      fCall.template operator()<GOInt8>();
    else if (m_BitsPerSample <= 16)
      fCall.template operator()<GOInt16>();
    else if (m_BitsPerSample <= 24)
      fCall.template operator()<GOInt24>();
    else if (m_IsFloat)
      fCall.template operator()<GOFloat32>();
    else
      fCall.template operator()<GOInt32>();
  }

  /**
   * @param bitsPerSample the bits per sample of the data
   * @param isFloat whether an item above 24 bits is a float
   * @param nChannels the number of channels
   */
  GOSoundItemAccessor(uint8_t bitsPerSample, bool isFloat, uint8_t nChannels);

  /**
   * @param pData the data
   * @param frameI the frame index
   * @param channelI the channel index
   * @return the item in int units
   */
  int GetIntItem(const void *pData, unsigned frameI, uint8_t channelI) const;

  /**
   * Writes the item in int units. A float format stores the value as Q23.
   * @param pData the data
   * @param frameI the frame index
   * @param channelI the channel index
   * @param value the new value in int units
   */
  void SetIntItem(
    void *pData, unsigned frameI, uint8_t channelI, int value) const;

  /**
   * @param pData the data
   * @param frameI the frame index
   * @param channelI the channel index
   * @return the item in float units
   */
  float GetFloatItem(
    const void *pData, unsigned frameI, uint8_t channelI) const;

  /**
   * Writes the item in float units. An integer format stores the value
   * truncated to an integer, a float format stores it as is.
   * @param pData the data
   * @param frameI the frame index
   * @param channelI the channel index
   * @param value the new value in float units
   */
  void SetFloatItem(
    void *pData, unsigned frameI, uint8_t channelI, float value) const;
};

#endif /* GOSOUNDITEMACCESSOR_H */
