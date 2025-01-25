/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINGSPIPESTAB_H
#define GOORGANSETTINGSPIPESTAB_H

#include <vector>

#include <wx/event.h>
#include <wx/panel.h>

#include "GOOrganSettingsButtonsProxy.h"

class wxCheckBox;
class wxChoice;
class wxComboBox;
class wxSpinButton;
class wxStaticText;
class wxTextCtrl;
class wxTreeCtrl;
class wxTreeEvent;
class wxTreeItemId;

class GOConfig;
class GOOrganModel;
class GOPipeConfigNode;

class GOOrganSettingsPipesTab : public wxPanel,
                                public GOOrganSettingsButtonsProxy {
private:
  class TreeItemData;

  GOConfig &r_config;
  GOPipeConfigNode &r_RootNode;

  wxTreeCtrl *m_Tree;
  wxStaticText *m_MemoryDisplay;
  wxStaticText *m_BitDisplay;
  wxTextCtrl *m_Amplitude;
  wxSpinButton *m_AmplitudeSpin;
  wxTextCtrl *m_Gain;
  wxSpinButton *m_GainSpin;
  wxTextCtrl *m_ManualTuning;
  wxSpinButton *m_ManualTuningSpin;
  wxTextCtrl *m_AutoTuningCorrection;
  wxSpinButton *m_AutoTuningCorrectionSpin;
  wxTextCtrl *m_Delay;
  wxSpinButton *m_DelaySpin;
  wxTextCtrl *m_ReleaseLength;
  wxSpinButton *m_ReleaseLengthSpin;
  wxTextCtrl *m_ToneBalance;
  wxSpinButton *m_ToneBalanceSpin;
  wxComboBox *m_AudioGroup;
  wxString m_LastAudioGroup;
  wxCheckBox *m_IgnorePitch;
  bool m_LastIgnorePitch;
  wxChoice *m_BitsPerSample;
  int m_LastBitsPerSample;
  wxChoice *m_Compress;
  int m_LastCompress;
  wxChoice *m_Channels;
  int m_LastChannels;
  wxChoice *m_LoopLoad;
  int m_LastLoopLoad;
  wxChoice *m_AttackLoad;
  int m_LastAttackLoad;
  wxChoice *m_ReleaseLoad;
  int m_LastReleaseLoad;

  TreeItemData *p_LastTreeItemData;
  unsigned m_LoadChangeCnt;

  wxTreeItemId FillTree(wxTreeItemId parent, GOPipeConfigNode &config);
  bool TransferDataToWindow() override;

  void SetEmpty(wxChoice *choice);
  void RemoveEmpty(wxChoice *choice);

  void Load(bool isForce);
  void UpdateAudioGroup(
    const std::vector<wxString> &audio_group, unsigned &pos, wxTreeItemId item);

  void OnTreeChanging(wxTreeEvent &e);
  void OnTreeChanged(wxTreeEvent &e);
  void OnTreeUpdated(wxCommandEvent &e) { Load(false); }
  void OnAmplitudeChanged(wxCommandEvent &e);
  void OnAmplitudeSpinChanged(wxSpinEvent &e);
  void OnGainChanged(wxCommandEvent &e);
  void OnGainSpinChanged(wxSpinEvent &e);
  void OnManualTuningChanged(wxCommandEvent &e);
  void OnManualTuningSpinChanged(wxSpinEvent &e);
  void OnAutoTuningCorrectionChanged(wxCommandEvent &e);
  void OnAutoTuningCorrectionSpinChanged(wxSpinEvent &e);
  void OnDelayChanged(wxCommandEvent &e);
  void OnDelaySpinChanged(wxSpinEvent &e);
  void OnReleaseLengthChanged(wxCommandEvent &e);
  void OnReleaseLengthSpinChanged(wxSpinEvent &e);
  void OnToneBalanceChanged(wxCommandEvent &e);
  void OnToneBalanceSpinChanged(wxSpinEvent &e);
  void OnAudioGroupChanged(wxCommandEvent &e) { NotifyModified(); }
  void OnIgnorePitchChanged(wxCommandEvent &e) { NotifyModified(); }
  void OnBitsPerSampleChanged(wxCommandEvent &e);
  void OnCompressChanged(wxCommandEvent &e);
  void OnChannelsChanged(wxCommandEvent &e);
  void OnLoopLoadChanged(wxCommandEvent &e);
  void OnAttackLoadChanged(wxCommandEvent &e);
  void OnReleaseLoadChanged(wxCommandEvent &e);

  bool Validate() override { return !CheckForUnapplied(); }

public:
  GOOrganSettingsPipesTab(
    GOOrganModel &organModel,
    wxWindow *parent,
    GOOrganSettingsButtonsProxy::Listener &listener);

  void DistributeAudio();
  void ResetToDefault();
  void DiscardChanges();
  void ApplyChanges();

  DECLARE_EVENT_TABLE()
};

#endif /* GOORGANSETTINGSPIPESTAB_H */
