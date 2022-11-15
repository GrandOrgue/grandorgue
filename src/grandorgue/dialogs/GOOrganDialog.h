/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANDIALOG_H
#define GOORGANDIALOG_H

#include <vector>
#include <wx/dialog.h>

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

class GOOrganDialog : public wxDialog, public GOView {
private:
  GOOrganController *m_OrganController;
  wxTreeCtrl *m_Tree;
  wxTextCtrl *m_Amplitude;
  wxSpinButton *m_AmplitudeSpin;
  wxTextCtrl *m_Gain;
  wxSpinButton *m_GainSpin;
  wxTextCtrl *m_Tuning;
  wxSpinButton *m_TuningSpin;
  wxTextCtrl *m_Delay;
  wxSpinButton *m_DelaySpin;
  wxComboBox *m_AudioGroup;
  wxString m_LastAudioGroup;
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
  wxButton *m_Reset;
  wxButton *m_Default;
  wxButton *m_DefaultAll;
  wxButton *m_AudioGroupAssistant;
  wxButton *m_Collapse;
  wxCheckBox *m_IgnorePitch;
  OrganTreeItemData *m_Last;
  unsigned m_LoadChangeCnt;
  wxDialog *m_ModalDialog;
  bool m_Destroying;
  bool m_DestroyPending;

  bool CloseModal();
  void FillTree();
  void Load();
  bool Changed();
  void Modified();
  void SetEmpty(wxChoice *choice);
  void RemoveEmpty(wxChoice *choice);
  void UpdateAudioGroup(
    std::vector<wxString> audio_group, unsigned &pos, wxTreeItemId item);
  void FillTree(wxTreeItemId parent, GOPipeConfigNode &config);
  void CloseTree(wxTreeItemId parent);
  void ResetSelectedToDefault(bool isForChildren);

  void OnTreeChanging(wxTreeEvent &e);
  void OnTreeChanged(wxTreeEvent &e);
  void OnTreeUpdated(wxCommandEvent &e);
  void OnAmplitudeSpinChanged(wxSpinEvent &e);
  void OnAmplitudeChanged(wxCommandEvent &e);
  void OnGainSpinChanged(wxSpinEvent &e);
  void OnGainChanged(wxCommandEvent &e);
  void OnTuningSpinChanged(wxSpinEvent &e);
  void OnTuningChanged(wxCommandEvent &e);
  void OnDelaySpinChanged(wxSpinEvent &e);
  void OnDelayChanged(wxCommandEvent &e);
  void OnAudioGroupChanged(wxCommandEvent &e);
  void OnBitsPerSampleChanged(wxCommandEvent &e);
  void OnCompressChanged(wxCommandEvent &e);
  void OnChannelsChanged(wxCommandEvent &e);
  void OnLoopLoadChanged(wxCommandEvent &e);
  void OnAttackLoadChanged(wxCommandEvent &e);
  void OnReleaseLoadChanged(wxCommandEvent &e);
  void OnEventApply(wxCommandEvent &e);
  void OnEventReset(wxCommandEvent &e);
  void OnEventDefault(wxCommandEvent &e);
  void OnEventDefaultAll(wxCommandEvent &e);
  void OnOK(wxCommandEvent &event);
  void OnCancel(wxCommandEvent &event);
  void OnAudioGroupAssitant(wxCommandEvent &e);
  void OnCollapse(wxCommandEvent &e);

protected:
  enum {
    ID_EVENT_TREE = 200,
    ID_EVENT_APPLY,
    ID_EVENT_RESET,
    ID_EVENT_AUDIO_GROUP_ASSISTANT,
    ID_EVENT_COLLAPSE,
    ID_EVENT_DEFAULT,
    ID_EVENT_DEFAULT_ALL,
    ID_EVENT_AMPLITUDE,
    ID_EVENT_AMPLITUDE_SPIN,
    ID_EVENT_GAIN,
    ID_EVENT_GAIN_SPIN,
    ID_EVENT_TUNING,
    ID_EVENT_TUNING_SPIN,
    ID_EVENT_DELAY,
    ID_EVENT_DELAY_SPIN,
    ID_EVENT_AUDIO_GROUP,
    ID_EVENT_IGNORE_PITCH,
    ID_EVENT_LOOP_LOAD,
    ID_EVENT_ATTACK_LOAD,
    ID_EVENT_RELEASE_LOAD,
    ID_EVENT_BITS_PER_SAMPLE,
    ID_EVENT_CHANNELS,
    ID_EVENT_COMPRESS
  };

public:
  GOOrganDialog(
    GODocumentBase *doc, wxWindow *parent, GOOrganController *organController);
  ~GOOrganDialog();

  bool Destroy();

  DECLARE_EVENT_TABLE()
};

#endif
