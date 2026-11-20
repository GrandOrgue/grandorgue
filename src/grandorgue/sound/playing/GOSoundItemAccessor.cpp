/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundItemAccessor.h"

#include <assert.h>
#include <type_traits>

#include "GOFloat32.h"
#include "GOInt.h"
#include "GOWave.h"

template <typename FuncT>
void GOSoundItemAccessor::dispatchItemType(FuncT &&fCall) const {
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

GOSoundItemAccessor::GOSoundItemAccessor(
  uint8_t bitsPerSample, bool isFloat, uint8_t nChannels)
  : m_BitsPerSample(bitsPerSample), m_IsFloat(isFloat), m_NChannels(nChannels) {
  assert(bitsPerSample >= 1 && bitsPerSample <= 32);
  assert(nChannels >= 1);
}

int GOSoundItemAccessor::GetIntItem(
  const void *pData, unsigned frameI, uint8_t channelI) const {
  int res;

  assert(channelI < m_NChannels);
  dispatchItemType([&]<typename ItemT>() {
    const ItemT item = getItem<ItemT>(pData, frameI, m_NChannels, channelI);

    if constexpr (std::is_same_v<ItemT, GOFloat32>)
      res = GOWave::floatToIntQ23(item);
    else
      res = item;
  });
  return res;
}

void GOSoundItemAccessor::SetIntItem(
  void *pData, unsigned frameI, uint8_t channelI, int value) const {
  assert(channelI < m_NChannels);
  dispatchItemType([&]<typename ItemT>() {
    if constexpr (std::is_same_v<ItemT, GOFloat32>)
      setItem<ItemT>(
        pData, frameI, m_NChannels, channelI, GOWave::intQ23ToFloat(value));
    else
      setItem<ItemT>(pData, frameI, m_NChannels, channelI, value);
  });
}

float GOSoundItemAccessor::GetFloatItem(
  const void *pData, unsigned frameI, uint8_t channelI) const {
  float res;

  assert(channelI < m_NChannels);
  dispatchItemType([&]<typename ItemT>() {
    res = (float)getItem<ItemT>(pData, frameI, m_NChannels, channelI);
  });
  return res;
}

void GOSoundItemAccessor::SetFloatItem(
  void *pData, unsigned frameI, uint8_t channelI, float value) const {
  assert(channelI < m_NChannels);
  dispatchItemType([&]<typename ItemT>() {
    if constexpr (std::is_same_v<ItemT, GOFloat32>)
      setItem<ItemT>(pData, frameI, m_NChannels, channelI, value);
    else
      setItem<ItemT>(pData, frameI, m_NChannels, channelI, (int)value);
  });
}
