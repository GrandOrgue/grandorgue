/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIG_H
#define GOPIPECONFIG_H

#include <wx/string.h>

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
  unsigned m_DefaultDelay;
  unsigned m_Delay;
  unsigned m_ReleaseTail; // the max release length in ms
  int m_BitsPerSample;
  int m_Channels;
  int m_LoopLoad;
  int m_Compress;
  int m_AttackLoad;
  int m_ReleaseLoad;
  int m_IgnorePitch;

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
  void Load(GOConfigReader &cfg, const wxString &group, const wxString &prefix);

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

  unsigned GetDefaultDelay() const { return m_DefaultDelay; }
  unsigned GetDelay() const { return m_Delay; }
  void SetDelay(unsigned delay) { SetSmallMember(delay, m_Delay); }

  unsigned GetReleaseTail() const { return m_ReleaseTail; }
  void SetReleaseTail(unsigned releaseTail) {
    SetSmallMember(
      releaseTail, m_ReleaseTail, &GOPipeUpdateCallback::UpdateReleaseTail);
  }

  int GetBitsPerSample() const { return m_BitsPerSample; }
  void SetBitsPerSample(int value) { SetSmallMember(value, m_BitsPerSample); }

  int GetChannels() const { return m_Channels; }
  void SetChannels(int value) { SetSmallMember(value, m_Channels); }

  int GetLoopLoad() const { return m_LoopLoad; }
  void SetLoopLoad(int value) { SetSmallMember(value, m_LoopLoad); }

  int GetCompress() const { return m_Compress; }
  void SetCompress(int value) { SetSmallMember(value, m_Compress); }

  int GetAttackLoad() const { return m_AttackLoad; }
  void SetAttackLoad(int value) { SetSmallMember(value, m_AttackLoad); }

  int GetReleaseLoad() const { return m_ReleaseLoad; }
  void SetReleaseLoad(int value) { SetSmallMember(value, m_ReleaseLoad); }

  int IsIgnorePitch() const { return m_IgnorePitch; }
  void SetIgnorePitch(int value) { SetSmallMember(value, m_IgnorePitch); }
};

#endif
