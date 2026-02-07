/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDORGANINTERFACEPROXY_H
#define GOSOUNDORGANINTERFACEPROXY_H

#include "GOInterfaceProxy.h"
#include "GOSoundOrganInterface.h"

/**
 * Proxy implementation for GOSoundOrganInterface.
 * Provides safe default behavior when not connected to a real sound engine.
 */
class GOSoundOrganInterfaceProxy
  : public GOInterfaceProxy<GOSoundOrganInterface> {
public:
  /**
   * Gets the sample rate. Returns 48000 if not connected.
   */
  unsigned GetSampleRate() const override {
    return ForwardCall(&GOSoundOrganInterface::GetSampleRate, 48000u);
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
      &GOSoundOrganInterface::StartPipeSample,
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
      &GOSoundOrganInterface::StartTremulantSample,
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

    ForwardCall(&GOSoundOrganInterface::UpdateVelocity, pipe, handle, velocity);
  }

  /**
   * Switches sample. Does nothing if not connected.
   */
  void SwitchSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override {

    ForwardCall(&GOSoundOrganInterface::SwitchSample, pipe, handle);
  }

  /**
   * Stops a sample. Returns 0 if not connected.
   */
  uint64_t StopSample(
    const GOSoundProvider *pipe, GOSoundSampler *handle) override {

    return ForwardCall(
      &GOSoundOrganInterface::StopSample,
      static_cast<uint64_t>(0),
      pipe,
      handle);
  }
};

#endif /* GOSOUNDORGANINTERFACEPROXY_H */
