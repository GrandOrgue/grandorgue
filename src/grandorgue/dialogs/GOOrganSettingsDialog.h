/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINGSDIALOG_H
#define GOORGANSETTINGSDIALOG_H

#include <vector>

#include "common/GOSimpleDialog.h"
#include "document-base/GOView.h"

class GOPipeConfigNode;
class GOOrganController;
class OrganTreeItemData;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxComboBox;
class wxSpinButton;
class wxSpinEvent;
class wxStaticText;
class wxTextCtrl;
class wxTreeCtrl;
class wxTreeEvent;
class wxTreeItemId;

class GOOrganSettingsDialog : public GOSimpleDialog, public GOView {
private:
  GOOrganController *m_OrganController;
  wxTreeCtrl *m_Tree;
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
  wxComboBox *m_AudioGroup;
  wxString m_LastAudioGroup;
  wxCheckBox *m_IgnorePitch;
  bool m_LastIgnorePitch;
  wxChoice *m_BitsPerSample;
  int m_LastBitsPerSample;
  wxChoice *m_Channels;
  int m_LastChannels;
  wxChoice *m_Compress;
  int m_LastCompress;
  wxChoice *m_LoopLoad;
  int m_LastLoopLoad;
  wxChoice *m_AttackLoad;
  int m_LastAttackLoad;
  wxChoice *m_ReleaseLoad;
  int m_LastReleaseLoad;
  wxStaticText *m_MemoryDisplay;
  wxStaticText *m_BitDisplay;
  wxButton *m_Apply;
  wxButton *m_Discard;
  wxButton *m_Default;
  wxButton *m_AudioGroupAssistant;
  OrganTreeItemData *m_Last;
  unsigned m_LoadChangeCnt;

  void FillTree();
  void Load(bool isForce);
  bool Changed();
  void Modified();
  void SetEmpty(wxChoice *choice);
  void RemoveEmpty(wxChoice *choice);
  void UpdateAudioGroup(
    std::vector<wxString> audio_group, unsigned &pos, wxTreeItemId item);
  wxTreeItemId FillTree(wxTreeItemId parent, GOPipeConfigNode &config);
  void CloseTree(wxTreeItemId parent);
  /**
   * Checks if all changes have been applied. If some unapplied changes are
   * present, then display an error message.
   * Returns if there are unapplied changes
   */
  bool CheckForUnapplied();

  void OnTreeChanging(wxTreeEvent &e);
  void OnTreeChanged(wxTreeEvent &e);
  void OnTreeUpdated(wxCommandEvent &e);
  void OnAmplitudeSpinChanged(wxSpinEvent &e);
  void OnAmplitudeChanged(wxCommandEvent &e);
  void OnGainSpinChanged(wxSpinEvent &e);
  void OnGainChanged(wxCommandEvent &e);
  void OnManualTuningSpinChanged(wxSpinEvent &e);
  void OnManualTuningChanged(wxCommandEvent &e);
  void OnAutoTuningCorrectionSpinChanged(wxSpinEvent &e);
  void OnAutoTuningCorrectionChanged(wxCommandEvent &e);
  void OnDelaySpinChanged(wxSpinEvent &e);
  void OnDelayChanged(wxCommandEvent &e);
  void OnReleaseLengthSpinChanged(wxSpinEvent &e);
  void OnReleaseLengthChanged(wxCommandEvent &e);
  void OnAudioGroupChanged(wxCommandEvent &e);
  void OnIgnorePitchChanged(wxCommandEvent &e);
  void OnBitsPerSampleChanged(wxCommandEvent &e);
  void OnCompressChanged(wxCommandEvent &e);
  void OnChannelsChanged(wxCommandEvent &e);
  void OnLoopLoadChanged(wxCommandEvent &e);
  void OnAttackLoadChanged(wxCommandEvent &e);
  void OnReleaseLoadChanged(wxCommandEvent &e);
  void OnEventApply(wxCommandEvent &e);
  void OnEventDiscard(wxCommandEvent &e);
  void OnEventDefault(wxCommandEvent &e);
  void OnAudioGroupAssitant(wxCommandEvent &e);

protected:
  bool TransferDataToWindow() override;
  bool Validate() override { return !CheckForUnapplied(); }

public:
  GOOrganSettingsDialog(
    GODocumentBase *doc, wxWindow *parent, GOOrganController *organController);

  DECLARE_EVENT_TABLE()
};

#endif
