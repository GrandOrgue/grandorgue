/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganDialog.h"

#include <unordered_set>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/log.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/spinbutt.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>

#include "config/GOConfig.h"
#include "model/GORank.h"
#include "model/GOWindchest.h"

#include "GOEvent.h"
#include "GOOrganController.h"
#include "GOSampleStatistic.h"

class OrganTreeItemData : public wxTreeItemData {
public:
  OrganTreeItemData(GOPipeConfigNode &c) {
    node = &c;
    config = &node->GetPipeConfig();
  }

  GOPipeConfigNode *node;
  GOPipeConfig *config;
};

DEFINE_LOCAL_EVENT_TYPE(wxEVT_TREE_UPDATED)

BEGIN_EVENT_TABLE(GOOrganDialog, wxDialog)
EVT_BUTTON(ID_EVENT_APPLY, GOOrganDialog::OnEventApply)
EVT_BUTTON(ID_EVENT_RESET, GOOrganDialog::OnEventReset)
EVT_BUTTON(ID_EVENT_DEFAULT, GOOrganDialog::OnEventDefault)
EVT_BUTTON(ID_EVENT_DEFAULT_ALL, GOOrganDialog::OnEventDefaultAll)
EVT_BUTTON(wxID_OK, GOOrganDialog::OnOK)
EVT_BUTTON(wxID_CANCEL, GOOrganDialog::OnCancel)
EVT_TREE_SEL_CHANGING(ID_EVENT_TREE, GOOrganDialog::OnTreeChanging)
EVT_TREE_SEL_CHANGED(ID_EVENT_TREE, GOOrganDialog::OnTreeChanged)
EVT_COMMAND(ID_EVENT_TREE, wxEVT_TREE_UPDATED, GOOrganDialog::OnTreeUpdated)
EVT_TEXT(ID_EVENT_AMPLITUDE, GOOrganDialog::OnAmplitudeChanged)
EVT_SPIN(ID_EVENT_AMPLITUDE_SPIN, GOOrganDialog::OnAmplitudeSpinChanged)
EVT_TEXT(ID_EVENT_GAIN, GOOrganDialog::OnGainChanged)
EVT_SPIN(ID_EVENT_GAIN_SPIN, GOOrganDialog::OnGainSpinChanged)
EVT_TEXT(ID_EVENT_MANUAL_TUNING, GOOrganDialog::OnManualTuningChanged)
EVT_SPIN(ID_EVENT_MANUAL_TUNING_SPIN, GOOrganDialog::OnManualTuningSpinChanged)
EVT_TEXT(
  ID_EVENT_AUTO_TUNING_CORRECTION, GOOrganDialog::OnAutoTuningCorrectionChanged)
EVT_SPIN(
  ID_EVENT_AUTO_TUNING_CORRECTION_SPIN,
  GOOrganDialog::OnAutoTuningCorrectionSpinChanged)
EVT_TEXT(ID_EVENT_DELAY, GOOrganDialog::OnDelayChanged)
EVT_SPIN(ID_EVENT_DELAY_SPIN, GOOrganDialog::OnDelaySpinChanged)
EVT_TEXT(ID_EVENT_AUDIO_GROUP, GOOrganDialog::OnAudioGroupChanged)
EVT_BUTTON(ID_EVENT_AUDIO_GROUP_ASSISTANT, GOOrganDialog::OnAudioGroupAssitant)
EVT_CHOICE(ID_EVENT_BITS_PER_SAMPLE, GOOrganDialog::OnBitsPerSampleChanged)
EVT_CHOICE(ID_EVENT_COMPRESS, GOOrganDialog::OnCompressChanged)
EVT_CHOICE(ID_EVENT_CHANNELS, GOOrganDialog::OnChannelsChanged)
EVT_CHOICE(ID_EVENT_LOOP_LOAD, GOOrganDialog::OnLoopLoadChanged)
EVT_CHOICE(ID_EVENT_ATTACK_LOAD, GOOrganDialog::OnAttackLoadChanged)
EVT_CHOICE(ID_EVENT_RELEASE_LOAD, GOOrganDialog::OnReleaseLoadChanged)
EVT_BUTTON(ID_EVENT_COLLAPSE, GOOrganDialog::OnCollapse)
END_EVENT_TABLE()

GOOrganDialog::GOOrganDialog(
  GODocumentBase *doc, wxWindow *parent, GOOrganController *organController)
  : wxDialog(
    parent,
    wxID_ANY,
    _("Organ settings"),
    wxDefaultPosition,
    wxDefaultSize,
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    GOView(doc, this),
    m_OrganController(organController),
    m_Apply(NULL),
    m_Reset(NULL),
    m_Last(NULL),
    m_LoadChangeCnt(0),
    m_ModalDialog(NULL),
    m_Destroying(false),
    m_DestroyPending(false) {
  wxArrayString choices;

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);

  topSizer->Add(mainSizer, 1, wxALL | wxEXPAND, 6);

  m_Tree = new wxTreeCtrl(
    this,
    ID_EVENT_TREE,
    wxDefaultPosition,
    wxDefaultSize,
    wxTR_HAS_BUTTONS | wxTR_MULTIPLE);
  wxBoxSizer *Sizer1 = new wxBoxSizer(wxVERTICAL);
  Sizer1->Add(m_Tree, 1, wxALIGN_TOP | wxEXPAND);

  m_Collapse = new wxButton(this, ID_EVENT_COLLAPSE, _("Collapse tree"));
  Sizer1->Add(m_Collapse);
  m_AudioGroupAssistant = new wxButton(
    this, ID_EVENT_AUDIO_GROUP_ASSISTANT, _("Distribute audio groups"));
  Sizer1->Add(m_AudioGroupAssistant);

  mainSizer->Add(Sizer1, 1, wxALIGN_LEFT | wxEXPAND);
  mainSizer->AddSpacer(5);

  wxScrolledWindow *scroll = new wxScrolledWindow(
    this,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize,
    wxVSCROLL,
    wxT("scrolledWindow"));

  wxBoxSizer *settingSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *box1 = new wxStaticBoxSizer(wxVERTICAL, scroll, _("Settings"));
  wxGridBagSizer *gb = new wxGridBagSizer(5, 5);

  gb->Add(
    new wxStaticText(scroll, wxID_ANY, _("Amplitude:")),
    wxGBPosition(0, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Amplitude = new wxTextCtrl(scroll, ID_EVENT_AMPLITUDE, wxEmptyString);
  gb->Add(m_Amplitude, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
  m_AmplitudeSpin = new wxSpinButton(scroll, ID_EVENT_AMPLITUDE_SPIN);
  m_AmplitudeSpin->SetRange(0, 1000);
  gb->Add(m_AmplitudeSpin, wxGBPosition(0, 2), wxDefaultSpan);

  gb->Add(
    new wxStaticText(scroll, wxID_ANY, _("Gain (dB):")),
    wxGBPosition(1, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Gain = new wxTextCtrl(scroll, ID_EVENT_GAIN, wxEmptyString);
  gb->Add(m_Gain, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
  m_GainSpin = new wxSpinButton(scroll, ID_EVENT_GAIN_SPIN);
  m_GainSpin->SetRange(-120, 40);
  gb->Add(m_GainSpin, wxGBPosition(1, 2), wxDefaultSpan);

  gb->Add(
    new wxStaticText(scroll, wxID_ANY, _("Manual Tuning (Cent):")),
    wxGBPosition(2, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_ManualTuning
    = new wxTextCtrl(scroll, ID_EVENT_MANUAL_TUNING, wxEmptyString);
  gb->Add(m_ManualTuning, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);
  m_ManualTuningSpin = new wxSpinButton(scroll, ID_EVENT_MANUAL_TUNING_SPIN);
  m_ManualTuningSpin->SetRange(-1800, 1800);
  gb->Add(m_ManualTuningSpin, wxGBPosition(2, 2));
  gb->Add(
    new wxStaticText(scroll, wxID_ANY, _("AutoTuning Correction (Cent):")),
    wxGBPosition(3, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_AutoTuningCorrection
    = new wxTextCtrl(scroll, ID_EVENT_AUTO_TUNING_CORRECTION, wxEmptyString);
  gb->Add(m_AutoTuningCorrection, wxGBPosition(3, 1), wxDefaultSpan, wxEXPAND);
  m_AutoTuningCorrectionSpin
    = new wxSpinButton(scroll, ID_EVENT_AUTO_TUNING_CORRECTION_SPIN);
  m_AutoTuningCorrectionSpin->SetRange(-1800, 1800);
  gb->Add(m_AutoTuningCorrectionSpin, wxGBPosition(3, 2));

  gb->Add(
    new wxStaticText(scroll, wxID_ANY, _("Tracker (ms):")),
    wxGBPosition(4, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Delay = new wxTextCtrl(scroll, ID_EVENT_DELAY, wxEmptyString);
  gb->Add(m_Delay, wxGBPosition(4, 1), wxDefaultSpan, wxEXPAND);
  m_DelaySpin = new wxSpinButton(scroll, ID_EVENT_DELAY_SPIN);
  m_DelaySpin->SetRange(0, 10000);
  gb->Add(m_DelaySpin, wxGBPosition(4, 2), wxDefaultSpan);

  gb->Add(
    new wxStaticText(scroll, wxID_ANY, _("Release Length (ms):")),
    wxGBPosition(5, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_ReleaseLength
    = new wxTextCtrl(scroll, ID_EVENT_RELEASE_LENGTH, wxEmptyString);
  gb->Add(m_ReleaseLength, wxGBPosition(5, 1), wxDefaultSpan, wxEXPAND);
  m_ReleaseLengthSpin = new wxSpinButton(scroll, ID_EVENT_RELEASE_LENGTH_SPIN);
  m_ReleaseLengthSpin->SetRange(0, 3000);
  gb->Add(m_ReleaseLengthSpin, wxGBPosition(5, 2), wxDefaultSpan);

  gb->Add(
    new wxStaticText(scroll, wxID_ANY, _("Audio group:")),
    wxGBPosition(6, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_AudioGroup = new wxComboBox(scroll, ID_EVENT_AUDIO_GROUP, wxEmptyString);
  m_AudioGroup->Append(wxEmptyString);
  std::vector<wxString> audio_groups
    = m_OrganController->GetSettings().GetAudioGroups();
  for (unsigned i = 0; i < audio_groups.size(); i++)
    m_AudioGroup->Append(audio_groups[i]);
  m_AudioGroup->SetValue(wxT(" "));
  m_LastAudioGroup = m_AudioGroup->GetValue();
  gb->Add(m_AudioGroup, wxGBPosition(6, 1), wxGBSpan(1, 2), wxEXPAND);

  gb->Add(
    m_IgnorePitch = new wxCheckBox(
      scroll,
      ID_EVENT_IGNORE_PITCH,
      _("Ignore pitch info in organ samples wav files")),
    wxGBPosition(7, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
    5);
  if (m_OrganController->GetRootPipeConfigNode().GetEffectiveIgnorePitch())
    m_IgnorePitch->SetValue(true);

  gb->AddGrowableCol(1, 1);

  box1->Add(gb, 0, wxEXPAND | wxALL, 5);
  settingSizer->Add(box1, 0, wxEXPAND | wxALL, 5);

  box1 = new wxStaticBoxSizer(wxVERTICAL, scroll, _("Sample Loading"));
  wxFlexGridSizer *grid = new wxFlexGridSizer(2, 5, 5);

  grid->AddGrowableCol(1, 1);

  choices.clear();
  choices.push_back(_("Parent default"));
  for (unsigned i = 8; i <= 24; i++)
    choices.push_back(wxString::Format(_("%d bits"), i));
  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Sample Size:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_BitsPerSample = new wxChoice(
    scroll,
    ID_EVENT_BITS_PER_SAMPLE,
    wxDefaultPosition,
    wxDefaultSize,
    choices);
  grid->Add(m_BitsPerSample, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Disabled"));
  choices.push_back(_("Enabled"));
  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Lossless compression:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Compress = new wxChoice(
    scroll, ID_EVENT_COMPRESS, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_Compress, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Don't load"));
  choices.push_back(_("Mono"));
  choices.push_back(_("Stereo"));
  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Sample channels:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Channels = new wxChoice(
    scroll, ID_EVENT_CHANNELS, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_Channels, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("First loop"));
  choices.push_back(_("Longest loop"));
  choices.push_back(_("All loops"));
  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Loop loading:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_LoopLoad = new wxChoice(
    scroll, ID_EVENT_LOOP_LOAD, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_LoopLoad, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Single attack"));
  choices.push_back(_("All"));
  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Attack loading:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_AttackLoad = new wxChoice(
    scroll, ID_EVENT_ATTACK_LOAD, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_AttackLoad, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Single release"));
  choices.push_back(_("All"));
  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Release loading:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_ReleaseLoad = new wxChoice(
    scroll, ID_EVENT_RELEASE_LOAD, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_ReleaseLoad, 1, wxEXPAND);

  m_BitsPerSample->SetSelection(wxNOT_FOUND);
  m_LastBitsPerSample = m_BitsPerSample->GetSelection();
  m_Compress->SetSelection(wxNOT_FOUND);
  m_LastCompress = m_Compress->GetSelection();
  m_Channels->SetSelection(wxNOT_FOUND);
  m_LastChannels = m_Channels->GetSelection();
  m_LoopLoad->SetSelection(wxNOT_FOUND);
  m_LastLoopLoad = m_LoopLoad->GetSelection();
  m_AttackLoad->SetSelection(wxNOT_FOUND);
  m_LastAttackLoad = m_AttackLoad->GetSelection();
  m_ReleaseLoad->SetSelection(wxNOT_FOUND);
  m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
  box1->Add(grid, 0, wxEXPAND | wxALL, 5);
  settingSizer->Add(box1, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
  m_Apply = new wxButton(scroll, ID_EVENT_APPLY, _("Apply"));
  m_Reset = new wxButton(scroll, ID_EVENT_RESET, _("Reset"));
  m_Default = new wxButton(scroll, ID_EVENT_DEFAULT, _("Default"));
  m_DefaultAll
    = new wxButton(scroll, ID_EVENT_DEFAULT_ALL, _("Default for All"));
  buttons->Add(m_Default);
  buttons->Add(m_DefaultAll);
  buttons->Add(m_Reset);
  buttons->Add(m_Apply);
  settingSizer->Add(buttons);

  box1 = new wxStaticBoxSizer(wxVERTICAL, scroll, _("Sample information"));
  grid = new wxFlexGridSizer(2, 5, 5);
  box1->Add(grid, 0, wxEXPAND | wxALL, 5);

  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Memory usage:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_MemoryDisplay = new wxStaticText(scroll, wxID_ANY, wxEmptyString);
  grid->Add(m_MemoryDisplay);

  grid->Add(
    new wxStaticText(scroll, wxID_ANY, _("Bits per sample:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_BitDisplay = new wxStaticText(scroll, wxID_ANY, wxEmptyString);
  grid->Add(m_BitDisplay);

  settingSizer->Add(box1, 0, wxEXPAND | wxALL, 5);

  scroll->SetSizer(settingSizer);
  scroll->SetScrollbars(0, 5, 0, 15);
  mainSizer->Add(scroll, 1, wxALIGN_RIGHT | wxEXPAND);

  topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 5);
  topSizer->Add(
    CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 5);
  topSizer->AddSpacer(5);
  FillTree();
  Load();
  SetSizerAndFit(topSizer);
}

GOOrganDialog::~GOOrganDialog() {}

bool GOOrganDialog::CloseModal() {
  if (m_DestroyPending) {
    wxDialog::Destroy();
    return true;
  }
  if (m_ModalDialog) {
    wxDialog *dlg = m_ModalDialog;
    m_ModalDialog = NULL;
    dlg->EndModal(wxID_CANCEL);
    if (m_Destroying)
      m_DestroyPending = true;
    return true;
  }
  return false;
}

bool GOOrganDialog::Destroy() {
  Hide();
  m_Destroying = true;
  if (CloseModal())
    return true;
  return wxDialog::Destroy();
}

void GOOrganDialog::SetEmpty(wxChoice *choice) {
  int index = choice->FindString(wxEmptyString);
  if (index == wxNOT_FOUND)
    index = choice->Append(wxEmptyString);
  choice->SetSelection(index);
}

void GOOrganDialog::RemoveEmpty(wxChoice *choice) {
  int sel = choice->GetSelection();
  int index = choice->FindString(wxEmptyString);
  if (index != wxNOT_FOUND)
    choice->Delete(index);
  choice->SetSelection(sel);
}

void GOOrganDialog::Load() {
  wxArrayTreeItemIds entries;

  m_Tree->GetSelections(entries);
  for (unsigned i = 0; i < entries.size(); i++) {
    if (!m_Tree->GetItemData(entries[i])) {
      wxLogError(
        _("Invalid item selected: %s"),
        m_Tree->GetItemText(entries[i]).c_str());
      entries.RemoveAt(i, 1);
      i--;
    }
  }

  GOSampleStatistic stat;
  for (unsigned i = 0; i < entries.size(); i++)
    if (m_Tree->GetItemData(entries[i]))
      stat.Cumulate(((OrganTreeItemData *)m_Tree->GetItemData(entries[i]))
                      ->node->GetStatistic());

  if (!stat.IsValid()) {
    m_MemoryDisplay->SetLabel(_("--- MB (--- MB end)"));
    m_BitDisplay->SetLabel(_("-- bits (- used)"));
  } else {
    m_MemoryDisplay->SetLabel(wxString::Format(
      _("%.3f MB  (%.3f MB end)"),
      stat.GetMemorySize() / (1024.0 * 1024.0),
      stat.GetEndSegmentSize() / (1024.0 * 1024.0)));
    wxString buf;
    if (stat.GetMinBitPerSample() == stat.GetMaxBitPerSample())
      buf = wxString::Format(_("%d bits"), stat.GetMinBitPerSample());
    else
      buf = wxString::Format(
        _("%d - %d bits"),
        stat.GetMinBitPerSample(),
        stat.GetMaxBitPerSample());
    m_BitDisplay->SetLabel(
      buf + wxString::Format(_(" (%.3f used)"), stat.GetUsedBits()));
  }

  if (entries.size() == 0) {
    m_Last = NULL;
    m_Amplitude->ChangeValue(wxEmptyString);
    m_Amplitude->Disable();
    m_AmplitudeSpin->Disable();
    m_Gain->ChangeValue(wxEmptyString);
    m_Gain->Disable();
    m_GainSpin->Disable();
    m_ManualTuning->ChangeValue(wxEmptyString);
    m_ManualTuning->Disable();
    m_ManualTuningSpin->Disable();
    m_AutoTuningCorrection->ChangeValue(wxEmptyString);
    m_AutoTuningCorrection->Disable();
    m_AutoTuningCorrectionSpin->Disable();
    m_Delay->ChangeValue(wxEmptyString);
    m_Delay->Disable();
    m_DelaySpin->Disable();
    m_ReleaseLength->ChangeValue(wxEmptyString);
    m_ReleaseLength->Disable();
    m_ReleaseLengthSpin->Disable();
    m_AudioGroup->Disable();
    m_IgnorePitch->Disable();
    m_BitsPerSample->Disable();
    m_Compress->Disable();
    m_Channels->Disable();
    m_LoopLoad->Disable();
    m_AttackLoad->Disable();
    m_ReleaseLoad->Disable();
    m_Apply->Disable();
    m_Reset->Disable();
    m_Default->Disable();
    m_AudioGroupAssistant->Disable();
    return;
  }

  m_AudioGroupAssistant->Enable();

  if (entries.size() > 1) {
    if (!m_Amplitude->IsModified())
      m_Amplitude->ChangeValue(wxEmptyString);
    if (!m_Gain->IsModified())
      m_Gain->ChangeValue(wxEmptyString);
    if (!m_ManualTuning->IsModified())
      m_ManualTuning->ChangeValue(wxEmptyString);
    if (!m_AutoTuningCorrection->IsModified())
      m_AutoTuningCorrection->ChangeValue(wxEmptyString);
    if (!m_Delay->IsModified())
      m_Delay->ChangeValue(wxEmptyString);
    if (!m_ReleaseLength->IsModified())
      m_ReleaseLength->ChangeValue(wxEmptyString);
    if (m_AudioGroup->GetValue() == m_LastAudioGroup) {
      m_AudioGroup->SetValue(wxT(" "));
      m_LastAudioGroup = m_AudioGroup->GetValue();
    }
    if (m_BitsPerSample->GetSelection() == m_LastBitsPerSample) {
      SetEmpty(m_BitsPerSample);
      m_LastBitsPerSample = m_BitsPerSample->GetSelection();
    }
    if (m_Compress->GetSelection() == m_LastCompress) {
      SetEmpty(m_Compress);
      m_LastCompress = m_Compress->GetSelection();
    }
    if (m_Channels->GetSelection() == m_LastChannels) {
      SetEmpty(m_Channels);
      m_LastChannels = m_Channels->GetSelection();
    }
    if (m_LoopLoad->GetSelection() == m_LastLoopLoad) {
      SetEmpty(m_LoopLoad);
      m_LastLoopLoad = m_LoopLoad->GetSelection();
    }
    if (m_AttackLoad->GetSelection() == m_LastAttackLoad) {
      SetEmpty(m_AttackLoad);
      m_LastAttackLoad = m_AttackLoad->GetSelection();
    }
    if (m_ReleaseLoad->GetSelection() == m_LastReleaseLoad) {
      SetEmpty(m_ReleaseLoad);
      m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
    }
  } else
    m_Apply->Disable();

  for (unsigned i = 0; i < entries.size(); i++)
    if (m_Last && m_Tree->GetItemData(entries[i]) == m_Last)
      return;

  m_Last = 0;
  for (unsigned i = 0; i < entries.size() && !m_Last; i++)
    m_Last = (OrganTreeItemData *)m_Tree->GetItemData(entries[i]);

  m_Amplitude->Enable();
  m_AmplitudeSpin->Enable();
  m_Gain->Enable();
  m_GainSpin->Enable();
  m_ManualTuning->Enable();
  m_ManualTuningSpin->Enable();
  m_AutoTuningCorrection->Enable();
  m_AutoTuningCorrectionSpin->Enable();
  m_Delay->Enable();
  m_DelaySpin->Enable();
  m_ReleaseLength->Enable();
  m_ReleaseLengthSpin->Enable();
  m_AudioGroup->Enable();
  m_IgnorePitch->Enable();
  m_BitsPerSample->Enable();
  m_Compress->Enable();
  m_Channels->Enable();
  m_LoopLoad->Enable();
  m_AttackLoad->Enable();
  m_ReleaseLoad->Enable();
  m_Default->Enable();
  m_Reset->Disable();

  float amplitude = m_Last->config->GetAmplitude();
  float gain = m_Last->config->GetGain();
  float manualTuning = m_Last->config->GetManualTuning();
  float autoTuningCorrection = m_Last->config->GetAutoTuningCorrection();
  unsigned delay = m_Last->config->GetDelay();
  unsigned releaseLength = m_Last->config->GetReleaseTail();
  int ignorePitch = m_Last->config->IsIgnorePitch();

  if (entries.size() == 1)
    m_Amplitude->ChangeValue(wxString::Format(wxT("%f"), amplitude));
  m_AmplitudeSpin->SetValue(amplitude);
  if (entries.size() == 1)
    m_Gain->ChangeValue(wxString::Format(wxT("%f"), gain));
  m_GainSpin->SetValue(gain);
  if (entries.size() == 1)
    m_ManualTuning->ChangeValue(wxString::Format(wxT("%f"), manualTuning));
  m_ManualTuningSpin->SetValue(manualTuning);
  if (entries.size() == 1)
    m_AutoTuningCorrection->ChangeValue(
      wxString::Format(wxT("%f"), autoTuningCorrection));
  m_AutoTuningCorrectionSpin->SetValue(autoTuningCorrection);
  if (entries.size() == 1)
    m_Delay->ChangeValue(wxString::Format(wxT("%u"), delay));
  if (entries.size() == 1)
    m_ReleaseLength->ChangeValue(wxString::Format(wxT("%u"), releaseLength));
  m_DelaySpin->SetValue(delay);
  if (entries.size() == 1) {
    m_AudioGroup->SetValue(m_Last->config->GetAudioGroup());

    int bits_per_sample = m_Last->config->GetBitsPerSample();
    if (bits_per_sample == -1)
      bits_per_sample = 0;
    else
      bits_per_sample = bits_per_sample - 7;

    RemoveEmpty(m_BitsPerSample);
    RemoveEmpty(m_Compress);
    RemoveEmpty(m_Channels);
    RemoveEmpty(m_LoopLoad);
    RemoveEmpty(m_AttackLoad);
    RemoveEmpty(m_ReleaseLoad);

    m_BitsPerSample->SetSelection(bits_per_sample);
    m_Compress->SetSelection(m_Last->config->GetCompress() + 1);
    m_Channels->SetSelection(m_Last->config->GetChannels() + 1);
    m_LoopLoad->SetSelection(m_Last->config->GetLoopLoad() + 1);
    m_AttackLoad->SetSelection(m_Last->config->GetAttackLoad() + 1);
    m_ReleaseLoad->SetSelection(m_Last->config->GetReleaseLoad() + 1);

    m_LastAudioGroup = m_AudioGroup->GetValue();
    m_LastBitsPerSample = m_BitsPerSample->GetSelection();
    m_LastCompress = m_Compress->GetSelection();
    m_LastChannels = m_Channels->GetSelection();
    m_LastLoopLoad = m_LoopLoad->GetSelection();
    m_LastAttackLoad = m_AttackLoad->GetSelection();
    m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
  }
}

void GOOrganDialog::OnAmplitudeSpinChanged(wxSpinEvent &e) {
  m_Amplitude->ChangeValue(
    wxString::Format(wxT("%f"), (float)m_AmplitudeSpin->GetValue()));
  m_Amplitude->MarkDirty();
  Modified();
}

void GOOrganDialog::OnAmplitudeChanged(wxCommandEvent &e) {
  double amp;
  if (m_Amplitude->GetValue().ToDouble(&amp))
    m_AmplitudeSpin->SetValue(amp);
  Modified();
}

void GOOrganDialog::OnGainSpinChanged(wxSpinEvent &e) {
  m_Gain->ChangeValue(
    wxString::Format(wxT("%f"), (float)m_GainSpin->GetValue()));
  m_Gain->MarkDirty();
  Modified();
}

void GOOrganDialog::OnGainChanged(wxCommandEvent &e) {
  double gain;
  if (m_Gain->GetValue().ToDouble(&gain))
    m_GainSpin->SetValue(gain);
  Modified();
}

void GOOrganDialog::OnManualTuningSpinChanged(wxSpinEvent &e) {
  m_ManualTuning->ChangeValue(
    wxString::Format(wxT("%f"), (float)m_ManualTuningSpin->GetValue()));
  m_ManualTuning->MarkDirty();
  Modified();
}

void GOOrganDialog::OnManualTuningChanged(wxCommandEvent &e) {
  double tuning;

  if (m_ManualTuning->GetValue().ToDouble(&tuning))
    m_ManualTuningSpin->SetValue(tuning);
  Modified();
}

void GOOrganDialog::OnAutoTuningCorrectionSpinChanged(wxSpinEvent &e) {
  m_AutoTuningCorrection->ChangeValue(
    wxString::Format(wxT("%f"), (float)m_AutoTuningCorrectionSpin->GetValue()));
  m_AutoTuningCorrection->MarkDirty();
  Modified();
}

void GOOrganDialog::OnAutoTuningCorrectionChanged(wxCommandEvent &e) {
  double tuning;

  if (m_AutoTuningCorrection->GetValue().ToDouble(&tuning))
    m_AutoTuningCorrectionSpin->SetValue(tuning);
  Modified();
}

void GOOrganDialog::OnDelaySpinChanged(wxSpinEvent &e) {
  m_Delay->ChangeValue(
    wxString::Format(wxT("%u"), (unsigned)m_DelaySpin->GetValue()));
  m_Delay->MarkDirty();
  Modified();
}

void GOOrganDialog::OnDelayChanged(wxCommandEvent &e) {
  long delay;
  if (m_Delay->GetValue().ToLong(&delay))
    m_DelaySpin->SetValue(delay);
  Modified();
}

void GOOrganDialog::OnAudioGroupChanged(wxCommandEvent &e) { Modified(); }

void GOOrganDialog::OnBitsPerSampleChanged(wxCommandEvent &e) {
  RemoveEmpty(m_BitsPerSample);
  Modified();
}

void GOOrganDialog::OnCompressChanged(wxCommandEvent &e) {
  RemoveEmpty(m_Compress);
  Modified();
}

void GOOrganDialog::OnChannelsChanged(wxCommandEvent &e) {
  RemoveEmpty(m_Channels);
  Modified();
}

void GOOrganDialog::OnLoopLoadChanged(wxCommandEvent &e) {
  RemoveEmpty(m_LoopLoad);
  Modified();
}

void GOOrganDialog::OnAttackLoadChanged(wxCommandEvent &e) {
  RemoveEmpty(m_AttackLoad);
  Modified();
}

void GOOrganDialog::OnReleaseLoadChanged(wxCommandEvent &e) {
  RemoveEmpty(m_ReleaseLoad);
  Modified();
}

bool GOOrganDialog::Changed() {
  bool changed = false;
  if (m_Amplitude->IsModified())
    changed = true;
  if (m_Gain->IsModified())
    changed = true;
  if (m_ManualTuning->IsModified())
    changed = true;
  if (m_AutoTuningCorrection->IsModified())
    changed = true;
  if (m_Delay->IsModified())
    changed = true;
  if (m_AudioGroup->GetValue() != m_LastAudioGroup)
    changed = true;
  if (m_BitsPerSample->GetSelection() != m_LastBitsPerSample)
    changed = true;
  if (m_Compress->GetSelection() != m_LastCompress)
    changed = true;
  if (m_Channels->GetSelection() != m_LastChannels)
    changed = true;
  if (m_LoopLoad->GetSelection() != m_LastLoopLoad)
    changed = true;
  if (m_AttackLoad->GetSelection() != m_LastAttackLoad)
    changed = true;
  if (m_ReleaseLoad->GetSelection() != m_LastReleaseLoad)
    changed = true;

  return changed;
}

void GOOrganDialog::Modified() {
  if (m_Reset)
    m_Reset->Enable();
  if (m_Apply)
    m_Apply->Enable();
}

void GOOrganDialog::FillTree(wxTreeItemId parent, GOPipeConfigNode &config) {
  wxTreeItemData *data = new OrganTreeItemData(config);
  wxTreeItemId e;
  if (!parent.IsOk())
    e = m_Tree->AddRoot(config.GetName(), -1, -1, data);
  else
    e = m_Tree->AppendItem(parent, config.GetName(), -1, -1, data);
  for (unsigned i = 0; i < config.GetChildCount(); i++)
    FillTree(e, *config.GetChild(i));
  m_Tree->Expand(e);
}

void GOOrganDialog::FillTree() {
  wxTreeItemId id_root;
  FillTree(id_root, m_OrganController->GetRootPipeConfigNode());
}

void GOOrganDialog::OnEventApply(wxCommandEvent &e) {
  double amp, gain, manualTuning, autoTuningCorrection;
  long delay;

  wxArrayTreeItemIds entries;
  m_Tree->GetSelections(entries);

  if (
    !m_Amplitude->GetValue().ToDouble(&amp)
    && (m_Amplitude->IsModified() && (amp < 0 || amp > 1000))) {
    GOMessageBox(
      _("Amplitude is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  if (
    !m_Gain->GetValue().ToDouble(&gain)
    && (m_Gain->IsModified() && (gain < -120 || gain > 40))) {
    GOMessageBox(_("Gain is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  if (
    !m_ManualTuning->GetValue().ToDouble(&manualTuning)
    && (m_ManualTuning->IsModified() && (manualTuning < -1800 || manualTuning > 1800))) {
    GOMessageBox(_("Tuning is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  if (
    !m_AutoTuningCorrection->GetValue().ToDouble(&autoTuningCorrection)
    && (m_AutoTuningCorrection->IsModified() && (autoTuningCorrection < -1800 || autoTuningCorrection > 1800))) {
    GOMessageBox(_("Tuning is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  if (
    !m_Delay->GetValue().ToLong(&delay)
    && (m_Delay->IsModified() && (delay < 0 || delay > 10000))) {
    GOMessageBox(
      _("Tracker delay is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  for (unsigned i = 0; i < entries.size(); i++) {
    OrganTreeItemData *e = (OrganTreeItemData *)m_Tree->GetItemData(entries[i]);
    if (!e)
      continue;
    if (m_Amplitude->IsModified())
      e->config->SetAmplitude(amp);
    if (m_Gain->IsModified())
      e->config->SetGain(gain);
    if (m_ManualTuning->IsModified())
      e->config->SetManualTuning(manualTuning);
    if (m_AutoTuningCorrection->IsModified())
      e->config->SetAutoTuningCorrection(autoTuningCorrection);
    if (m_Delay->IsModified())
      e->config->SetDelay(delay);
    if (m_AudioGroup->GetValue() != m_LastAudioGroup)
      e->config->SetAudioGroup(m_AudioGroup->GetValue().Trim());
    if (m_BitsPerSample->GetSelection() != m_LastBitsPerSample)
      e->config->SetBitsPerSample(
        m_BitsPerSample->GetSelection() == 0
          ? -1
          : m_BitsPerSample->GetSelection() + 7);
    if (m_Compress->GetSelection() != m_LastCompress)
      e->config->SetCompress(m_Compress->GetSelection() - 1);
    if (m_Channels->GetSelection() != m_LastChannels)
      e->config->SetChannels(m_Channels->GetSelection() - 1);
    if (m_LoopLoad->GetSelection() != m_LastLoopLoad)
      e->config->SetLoopLoad(m_LoopLoad->GetSelection() - 1);
    if (m_AttackLoad->GetSelection() != m_LastAttackLoad)
      e->config->SetAttackLoad(m_AttackLoad->GetSelection() - 1);
    if (m_ReleaseLoad->GetSelection() != m_LastReleaseLoad)
      e->config->SetReleaseLoad(m_ReleaseLoad->GetSelection() - 1);
  }

  m_Reset->Disable();
  m_Apply->Disable();
  if (m_Amplitude->IsModified()) {
    m_Amplitude->ChangeValue(wxString::Format(wxT("%f"), amp));
    m_Amplitude
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_Gain->IsModified()) {
    m_Gain->ChangeValue(wxString::Format(wxT("%f"), gain));
    m_Gain->DiscardEdits(); // workaround of osx implementation bug
                            // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_ManualTuning->IsModified()) {
    m_ManualTuning->ChangeValue(wxString::Format(wxT("%f"), manualTuning));
    m_ManualTuning
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_AutoTuningCorrection->IsModified()) {
    m_AutoTuningCorrection->ChangeValue(
      wxString::Format(wxT("%f"), autoTuningCorrection));
    m_AutoTuningCorrection
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_Delay->IsModified()) {
    m_Delay->ChangeValue(wxString::Format(wxT("%lu"), delay));
    m_Delay->DiscardEdits(); // workaround of osx implementation bug
                             // https://github.com/oleg68/GrandOrgue/issues/87
  }
  m_LastAudioGroup = m_AudioGroup->GetValue();
  m_LastBitsPerSample = m_BitsPerSample->GetSelection();
  m_LastCompress = m_Compress->GetSelection();
  m_LastChannels = m_Channels->GetSelection();
  m_LastLoopLoad = m_LoopLoad->GetSelection();
  m_LastAttackLoad = m_AttackLoad->GetSelection();
  m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
}

void GOOrganDialog::OnEventReset(wxCommandEvent &e) {
  m_Last = NULL;
  Load();
}

void GOOrganDialog::ResetSelectedToDefault(bool isForChildren) {
  wxArrayTreeItemIds entries;
  m_Tree->GetSelections(entries);

  std::unordered_set<OrganTreeItemData *> oiSet;

  // Fill idSet with entries
  for (const wxTreeItemId id : entries) {
    OrganTreeItemData *p = (OrganTreeItemData *)m_Tree->GetItemData(id);

    if (p)
      oiSet.insert(p);
  }

  // add the children
  if (isForChildren) {
    // entries.size() it may not be precalculated because it is increased
    for (unsigned i = 0; i < entries.size(); i++) {
      const wxTreeItemId id = entries[i];
      wxTreeItemIdValue cookie;

      for (wxTreeItemId c = m_Tree->GetFirstChild(id, cookie); c.IsOk();
           c = m_Tree->GetNextChild(id, cookie)) {
        OrganTreeItemData *p = (OrganTreeItemData *)m_Tree->GetItemData(c);

        if (p && oiSet.find(p) == oiSet.end()) {
          oiSet.insert(p);
          entries.push_back(c);
          // c's children will be scanned later in the loop
        }
      }
    }
  }

  for (OrganTreeItemData *e : oiSet) {
    e->config->SetAmplitude(e->config->GetDefaultAmplitude());
    e->config->SetGain(e->config->GetDefaultGain());
    e->config->SetManualTuning(0);
    e->config->SetAutoTuningCorrection(0);
    e->config->SetDelay(e->config->GetDefaultDelay());
    e->config->SetAudioGroup(wxEmptyString);
    e->config->SetBitsPerSample(-1);
    e->config->SetCompress(-1);
    e->config->SetChannels(-1);
    e->config->SetLoopLoad(-1);
    e->config->SetAttackLoad(-1);
    e->config->SetReleaseLoad(-1);
  }

  m_Last = NULL;
  Load();
}

void GOOrganDialog::OnEventDefault(wxCommandEvent &e) {
  ResetSelectedToDefault(false);
}

void GOOrganDialog::OnEventDefaultAll(wxCommandEvent &e) {
  ResetSelectedToDefault(true);
}

void GOOrganDialog::OnTreeChanging(wxTreeEvent &e) {
  if (Changed()) {
    GOMessageBox(
      _("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, this);
    e.Veto();
  }
}

void GOOrganDialog::OnTreeChanged(wxTreeEvent &e) {
  wxArrayTreeItemIds entries;
  if (m_LoadChangeCnt)
    return;
  m_LoadChangeCnt++;
  do {
    bool rescan = false;
    m_Tree->GetSelections(entries);
    for (unsigned i = 0; i < entries.size(); i++) {
      OrganTreeItemData *e
        = (OrganTreeItemData *)m_Tree->GetItemData(entries[i]);
      if (!e) {
        wxTreeItemIdValue it;
        wxTreeItemId child = m_Tree->GetFirstChild(entries[i], it);
        while (child.IsOk()) {
          bool found = false;
          for (unsigned j = 0; j < entries.size(); j++)
            if (entries[j] == child)
              found = true;
          if (!found)
            m_Tree->SelectItem(child, true);
          child = m_Tree->GetNextChild(entries[i], it);
        }
        rescan = true;
        m_Tree->SelectItem(entries[i], false);
      }
    }
    if (rescan)
      continue;
  } while (false);
  m_LoadChangeCnt--;

  wxCommandEvent event(wxEVT_TREE_UPDATED, ID_EVENT_TREE);
  GetEventHandler()->AddPendingEvent(event);
}

void GOOrganDialog::OnTreeUpdated(wxCommandEvent &e) { Load(); }

void GOOrganDialog::OnOK(wxCommandEvent &event) {
  if (Changed()) {
    GOMessageBox(
      _("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }
  GOPipeConfig &rootPipeConfig(
    m_OrganController->GetRootPipeConfigNode().GetPipeConfig());
  bool newIgnorePitch = m_IgnorePitch->GetValue();

  // for avoiding modification of rootPipeConfig when it is non necessary
  if (newIgnorePitch != (rootPipeConfig.IsIgnorePitch() > 0))
    m_OrganController->GetRootPipeConfigNode().GetPipeConfig().SetIgnorePitch(
      newIgnorePitch);
  m_OrganController->SetTemperament(m_OrganController->GetTemperament());
  m_OrganController->SetOrganModified();
  Destroy();
}

void GOOrganDialog::OnCancel(wxCommandEvent &event) { Destroy(); }

void GOOrganDialog::UpdateAudioGroup(
  std::vector<wxString> audio_group, unsigned &pos, wxTreeItemId item) {
  OrganTreeItemData *e = (OrganTreeItemData *)m_Tree->GetItemData(item);
  if (e) {
    e->config->SetAudioGroup(audio_group[pos]);
    pos++;
    if (pos >= audio_group.size())
      pos = 0;
  }

  wxTreeItemIdValue it;
  wxTreeItemId child = m_Tree->GetFirstChild(item, it);
  while (child.IsOk()) {
    UpdateAudioGroup(audio_group, pos, child);
    child = m_Tree->GetNextChild(item, it);
  }
}

void GOOrganDialog::OnAudioGroupAssitant(wxCommandEvent &e) {
  if (Changed()) {
    GOMessageBox(
      _("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }
  wxArrayString strs;
  std::vector<wxString> group_list
    = m_OrganController->GetSettings().GetAudioGroups();
  for (unsigned i = 0; i < group_list.size(); i++)
    strs.Add(group_list[i]);

  wxMultiChoiceDialog dlg(
    this, _("Select audio groups to distribute:"), _("Organ dialog"), strs);
  m_ModalDialog = &dlg;
  if (dlg.ShowModal() != wxID_OK) {
    CloseModal();
    return;
  }
  wxArrayInt sel = dlg.GetSelections();
  CloseModal();
  if (sel.Count() == 0) {
    GOMessageBox(
      _("No audio group selected"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }
  group_list.clear();
  for (unsigned i = 0; i < sel.Count(); i++)
    group_list.push_back(strs[sel[i]]);

  wxArrayTreeItemIds entries;
  m_Tree->GetSelections(entries);
  unsigned pos = 0;
  for (unsigned i = 0; i < entries.size(); i++)
    UpdateAudioGroup(group_list, pos, entries[i]);
  m_Last = NULL;
  Load();
}

void GOOrganDialog::CloseTree(wxTreeItemId parent) {
  m_Tree->Collapse(parent);
  wxTreeItemIdValue it;
  for (wxTreeItemId child = m_Tree->GetFirstChild(parent, it); child.IsOk();
       child = m_Tree->GetNextChild(parent, it))
    CloseTree(child);
}

void GOOrganDialog::OnCollapse(wxCommandEvent &e) {
  if (Changed()) {
    GOMessageBox(
      _("Please apply changes first"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }
  CloseTree(m_Tree->GetRootItem());
}
