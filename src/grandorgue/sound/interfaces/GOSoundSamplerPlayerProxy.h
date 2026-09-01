/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSAMPLERPLAYERPROXY_H
#define GOSOUNDSAMPLERPLAYERPROXY_H

#include "GOInterfaceProxy.h"
#include "GOSoundSamplerPlayerInterface.h"

/**
 * Proxy implementation for GOSoundSamplerPlayerInterface.
 * Provides safe default behavior when not connected to a real sound engine.
 */
class GOSoundSamplerPlayerProxy
  : public GOInterfaceProxy<GOSoundSamplerPlayerInterface> {
public:
  /**
   * Gets the sample rate. Returns 48000 if not connected.
   */
  unsigned GetSampleRate() const override {
    return ForwardCall(&GOSoundSamplerPlayerInterface::GetSampleRate, 48000u);
  }

  /**
   * Starts a pipe sample. Returns nullptr if not connected.
   */
  GOSoundSampler *StartPipeSample(
    const GOSoundProvider *pipeProvider,
    unsigned windchestN,
    unsigned audioGroup,
    unsigned velocity,
    unsigned delay,
    uint64_t prevEventTime,
    bool isRelease = false,
    uint64_t *pStartTimeSamples = nullptr) override {

    return ForwardCall(
      &GOSoundSamplerPlayerInterface::StartPipeSample,
      nullptr,
      pipeProvider,
      windchestN,
      audioGroup,
      velocity,
      delay,
      prevEventTime,
      isRelease,
      pStartTimeSamples);
  }

  /**
   * Starts a tremulant sample. Returns nullptr if not connected.
   */
  GOSoundSampler *StartTremulantSample(
    const GOSoundProvider *tremProvider,
    unsigned tremulantN,
    uint64_t prevEventTime) override {

    return ForwardCall(
      &GOSoundSamplerPlayerInterface::StartTremulantSample,
      nullptr,
      tremProvider,
      tremulantN,
      prevEventTime);
  }

  /**
   * Updates velocity. Does nothing if not connected.
   */
  void UpdateVelocity(
    const GOSoundProvider *pipe,
    GOSoundSampler *handle,
    unsigned velocity) override {

    ForwardCall(
      &GOSoundSamplerPlayerInterface::UpdateVelocity, pipe, handle, velocity);
  }

  /**
   * Switches sample. Does nothing if not connected.
   */
  void SwitchSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override {

    ForwardCall(&GOSoundSamplerPlayerInterface::SwitchSample, pipe, handle);
  }

  /**
   * Stops a sample. Returns 0 if not connected.
   */
  uint64_t StopSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override {

    return ForwardCall(
      &GOSoundSamplerPlayerInterface::StopSample,
      static_cast<uint64_t>(0),
      pipe,
      handle);
  }
};

#endif /* GOSOUNDSAMPLERPLAYERPROXY_H */
