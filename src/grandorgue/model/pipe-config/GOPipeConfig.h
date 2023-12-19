/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIG_H
#define GOPIPECONFIG_H

#include <wx/string.h>

#include "GOPipeUpdateCallback.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganModel;

class GOPipeConfig {
private:
  GOOrganModel *m_OrganModel;
  GOPipeUpdateCallback *m_Callback;
  wxString m_Group;
  wxString m_NamePrefix;
  wxString m_AudioGroup;
  float m_Amplitude;
  float m_DefaultAmplitude;
  float m_DefaultGain;
  // ODF pitch tuning offset for using withot auto-tuning
  float m_PitchTuning;
  // ODF pitch tuning offset for using with auto-tuning
  float m_PitchCorrection;
  float m_Gain;
  float m_ManualTuning;
  float m_AutoTuningCorrection;
  unsigned m_Delay;
  unsigned m_DefaultDelay;
  int m_BitsPerSample;
  int m_Compress;
  int m_Channels;
  int m_LoopLoad;
  int m_AttackLoad;
  int m_ReleaseLoad;
  int m_IgnorePitch;
  unsigned m_ReleaseTail; // the max release length in ms

  void ReadTuning(GOConfigReader &cfg, wxString group, wxString prefix);

public:
  GOPipeConfig(GOOrganModel *organModel, GOPipeUpdateCallback *callback);

  void Init(GOConfigReader &cfg, wxString group, wxString prefix);
  void Load(GOConfigReader &cfg, wxString group, wxString prefix);
  void Save(GOConfigWriter &cfg);

  GOPipeUpdateCallback *GetCallback();

  float GetAmplitude();
  float GetDefaultAmplitude();
  void SetAmplitude(float amp);

  float GetGain();
  float GetDefaultGain();
  void SetGain(float gain);

  float GetPitchTuning() const { return m_PitchTuning; }
  float GetPitchCorrection() const { return m_PitchCorrection; }
  float GetManualTuning() const { return m_ManualTuning; }
  void SetManualTuning(float cent);
  float GetAutoTuningCorrection() const { return m_AutoTuningCorrection; }
  void SetAutoTuningCorrection(float cent);

  unsigned GetDefaultDelay() const { return m_DefaultDelay; }

  unsigned GetDelay() const { return m_Delay; }
  void SetDelay(unsigned delay);

  const wxString &GetAudioGroup() const { return m_AudioGroup; }
  void SetAudioGroup(const wxString &str);

  int GetBitsPerSample() const { return m_BitsPerSample; }
  void SetBitsPerSample(int value);

  int GetCompress() const { return m_Compress; }
  void SetCompress(int value);

  int GetChannels() const { return m_Channels; }
  void SetChannels(int value);

  int GetLoopLoad() const { return m_LoopLoad; }
  void SetLoopLoad(int value);

  int GetAttackLoad() const { return m_AttackLoad; }
  void SetAttackLoad(int value);

  int GetReleaseLoad() const { return m_ReleaseLoad; }
  void SetReleaseLoad(int value);

  int IsIgnorePitch() const { return m_IgnorePitch; }
  void SetIgnorePitch(int value);

  unsigned GetReleaseTail() const { return m_ReleaseTail; }
  void SetReleaseTail(unsigned releaseTail);
};

#endif
