/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIG_H
#define GOPIPECONFIG_H

#include <cstdint>

#include <wx/string.h>

#include "GOBool3.h"
#include "GOPipeConfigListener.h"
#include "GOPipeUpdateCallback.h"

class GOConfigReader;
class GOConfigWriter;
class GOPipeConfigListener;

class GOPipeConfig {
private:
  GOPipeConfigListener &r_listener;
  GOPipeUpdateCallback *m_Callback;
  wxString m_Group;
  wxString m_NamePrefix;
  wxString m_AudioGroup;
  float m_DefaultAmplitude;
  float m_Amplitude;
  float m_DefaultGain;
  float m_Gain;
  // ODF pitch tuning offset for using withot auto-tuning
  float m_PitchTuning;
  // ODF pitch tuning offset for using with auto-tuning
  float m_PitchCorrection;
  float m_ManualTuning;
  float m_AutoTuningCorrection;
  uint16_t m_DefaultDelay;
  uint16_t m_Delay;
  uint16_t m_ReleaseTail; // the max release length in ms
  int8_t m_BitsPerSample;
  int8_t m_Channels;
  int8_t m_LoopLoad;
  GOBool3 m_Percussive;
  GOBool3 m_IndependentRelease;
  GOBool3 m_Compress;
  GOBool3 m_AttackLoad;
  GOBool3 m_ReleaseLoad;
  GOBool3 m_IgnorePitch;

  // Load all customizable values from the .cmb
  void LoadFromCmb(
    GOConfigReader &cfg, const wxString &group, const wxString &prefix);

  /* two generic setters SetSmallMember and SetLargeMember differ only in
   * passing values */

#define SET_MEMBER_BODY(value, member, callbackFun)                            \
  member = value;                                                              \
  if (callbackFun)                                                             \
    (m_Callback->*callbackFun)();                                              \
  r_listener.NotifyPipeConfigModified();

  template <typename T>
  void SetSmallMember(
    const T value,
    T &member,
    void (GOPipeUpdateCallback::*callbackFun)() = nullptr) {
    SET_MEMBER_BODY(value, member, callbackFun)
  }

  template <typename T>
  void SetLargeMember(
    const T &value,
    T &member,
    void (GOPipeUpdateCallback::*callbackFun)() = nullptr) {
    SET_MEMBER_BODY(value, member, callbackFun)
  }

  void SetPitchMember(float cents, float &member);

public:
  GOPipeConfig(GOPipeConfigListener &listener, GOPipeUpdateCallback *callback);

  // Set default values and load all customizable values from the .cmb
  void Init(GOConfigReader &cfg, const wxString &group, const wxString &prefix);

  // Load default values from the ODF and and load all customizable values from
  // the .cmb
  void Load(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &prefix,
    bool isParentPercussive);

  // Save all customizable values to the .cmb
  void Save(GOConfigWriter &cfg);

  GOPipeUpdateCallback *GetCallback() const { return m_Callback; }

  const wxString &GetAudioGroup() const { return m_AudioGroup; }
  void SetAudioGroup(const wxString &str) {
    SetLargeMember(str, m_AudioGroup, &GOPipeUpdateCallback::UpdateAudioGroup);
  }

  float GetDefaultAmplitude() const { return m_DefaultAmplitude; }
  float GetAmplitude() const { return m_Amplitude; }
  void SetAmplitude(float amp) {
    SetSmallMember(amp, m_Amplitude, &GOPipeUpdateCallback::UpdateAmplitude);
  }

  float GetDefaultGain() const { return m_DefaultGain; }
  float GetGain() const { return m_Gain; }
  void SetGain(float gain) {
    SetSmallMember(gain, m_Gain, &GOPipeUpdateCallback::UpdateAmplitude);
  }

  float GetPitchTuning() const { return m_PitchTuning; }
  float GetPitchCorrection() const { return m_PitchCorrection; }

  float GetManualTuning() const { return m_ManualTuning; }
  void SetManualTuning(float cents) { SetPitchMember(cents, m_ManualTuning); }

  float GetAutoTuningCorrection() const { return m_AutoTuningCorrection; }
  void SetAutoTuningCorrection(float cents) {
    SetPitchMember(cents, m_AutoTuningCorrection);
  }

  uint16_t GetDefaultDelay() const { return m_DefaultDelay; }
  uint16_t GetDelay() const { return m_Delay; }
  void SetDelay(uint16_t delay) { SetSmallMember(delay, m_Delay); }

  uint16_t GetReleaseTail() const { return m_ReleaseTail; }
  void SetReleaseTail(uint16_t releaseTail) {
    SetSmallMember(
      releaseTail, m_ReleaseTail, &GOPipeUpdateCallback::UpdateReleaseTail);
  }

  int8_t GetBitsPerSample() const { return m_BitsPerSample; }
  void SetBitsPerSample(int8_t value) {
    SetSmallMember(value, m_BitsPerSample);
  }

  int8_t GetChannels() const { return m_Channels; }
  void SetChannels(int8_t value) { SetSmallMember(value, m_Channels); }

  int8_t GetLoopLoad() const { return m_LoopLoad; }
  void SetLoopLoad(int8_t value) { SetSmallMember(value, m_LoopLoad); }

  GOBool3 GetPercussive() const { return m_Percussive; }
  // does not send notifications
  void SetPercussiveFromInit(GOBool3 value) { m_Percussive = value; }

  GOBool3 GetIndependentRelease() const { return m_IndependentRelease; }

  GOBool3 GetCompress() const { return m_Compress; }
  void SetCompress(GOBool3 value) { SetSmallMember(value, m_Compress); }

  GOBool3 GetAttackLoad() const { return m_AttackLoad; }
  void SetAttackLoad(GOBool3 value) { SetSmallMember(value, m_AttackLoad); }

  GOBool3 GetReleaseLoad() const { return m_ReleaseLoad; }
  void SetReleaseLoad(GOBool3 value) { SetSmallMember(value, m_ReleaseLoad); }

  GOBool3 IsIgnorePitch() const { return m_IgnorePitch; }
  void SetIgnorePitch(GOBool3 value) { SetSmallMember(value, m_IgnorePitch); }
};

#endif
