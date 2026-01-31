/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDORGANINTERFACE_H
#define GOSOUNDORGANINTERFACE_H

#include <cstdint>

/**
 * This class represents entry points of GOSoundEngine to the organ model.
 * It is intended that organ model objects would not call GOSoundEngine and
 * GOSoundSystem directly but would use this interface.
 */

class GOSoundProvider;
class GOSoundSampler;

class GOSoundOrganInterface {
public:
  virtual ~GOSoundOrganInterface() = default;

  // ProxyDefault: 48000
  virtual unsigned GetSampleRate() const = 0;

  // ProxyDefault: nullptr
  virtual GOSoundSampler *StartPipeSample(
    const GOSoundProvider *pipeProvider,
    unsigned windchestN,
    unsigned audioGroup,
    unsigned velocity,
    unsigned delay,
    uint64_t prevEventTime,
    bool isRelease = false,
    uint64_t *pStartTimeSamples = nullptr)
    = 0;

  // ProxyDefault: nullptr
  virtual GOSoundSampler *StartTremulantSample(
    const GOSoundProvider *tremProvider,
    unsigned tremulantN,
    uint64_t prevEventTime)
    = 0;

  // ProxyDefault: nothing
  virtual void UpdateVelocity(
    const GOSoundProvider *pipe, GOSoundSampler *handle, unsigned velocity)
    = 0;

  // ProxyDefault: nothing
  virtual void SwitchSample(const GOSoundProvider *pipe, GOSoundSampler *handle)
    = 0;

  // ProxyDefault: 0
  virtual uint64_t StopSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle)
    = 0;
};

#endif /* GOSOUNDORGANINTERFACE_H */
