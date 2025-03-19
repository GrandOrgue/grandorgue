/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOOrganSettingsPipesTab.h"

#include <wx/checkbox.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/spinbutt.h>
#include <wx/stattext.h>
#include <wx/treebase.h>
#include <wx/treectrl.h>

#include "model/GOOrganModel.h"
#include "model/pipe-config/GOPipeConfigNode.h"

#include "GOEvent.h"
#include "GOSampleStatistic.h"

static constexpr unsigned RELEASE_LENGTH_MAX = 3000;
static constexpr unsigned RELEASE_LENGTH_STEP = 50;
static constexpr unsigned RELEASE_LENGTH_MAX_INDEX
  = RELEASE_LENGTH_MAX / RELEASE_LENGTH_STEP;
static const wxSize EDIT_SIZE = wxSize(60, -1);
static const wxString WX_TAB_CODE = wxT("Pipes");
static const wxString WX_TAB_TITLE = _("Pipes");

class GOOrganSettingsPipesTab::TreeItemData : public wxTreeItemData {
public:
  GOPipeConfigNode &r_node;
  GOPipeConfig &r_config;

  TreeItemData(GOPipeConfigNode &node)
    : r_node(node), r_config(node.GetPipeConfig()) {}
};

enum {
  ID_EVENT_TREE = 200,
  ID_EVENT_AMPLITUDE,
  ID_EVENT_AMPLITUDE_SPIN,
  ID_EVENT_GAIN,
  ID_EVENT_GAIN_SPIN,
  ID_EVENT_MANUAL_TUNING,
  ID_EVENT_MANUAL_TUNING_SPIN,
  ID_EVENT_AUTO_TUNING_CORRECTION,
  ID_EVENT_AUTO_TUNING_CORRECTION_SPIN,
  ID_EVENT_DELAY,
  ID_EVENT_DELAY_SPIN,
  ID_EVENT_RELEASE_LENGTH,
  ID_EVENT_RELEASE_LENGTH_SPIN,
  ID_EVENT_TONE_BALANCE,
  ID_EVENT_TONE_BALANCE_SPIN,
  ID_EVENT_AUDIO_GROUP,
  ID_EVENT_IGNORE_PITCH,
  ID_EVENT_BITS_PER_SAMPLE,
  ID_EVENT_COMPRESS,
  ID_EVENT_CHANNELS,
  ID_EVENT_LOOP_LOAD,
  ID_EVENT_ATTACK_LOAD,
  ID_EVENT_RELEASE_LOAD
};

DEFINE_LOCAL_EVENT_TYPE(wxEVT_TREE_UPDATED)

BEGIN_EVENT_TABLE(GOOrganSettingsPipesTab, wxPanel)
EVT_TREE_SEL_CHANGING(ID_EVENT_TREE, GOOrganSettingsPipesTab::OnTreeChanging)
EVT_TREE_SEL_CHANGED(ID_EVENT_TREE, GOOrganSettingsPipesTab::OnTreeChanged)
EVT_COMMAND(
  ID_EVENT_TREE, wxEVT_TREE_UPDATED, GOOrganSettingsPipesTab::OnTreeUpdated)
EVT_TEXT(ID_EVENT_AMPLITUDE, GOOrganSettingsPipesTab::OnAmplitudeChanged)
EVT_SPIN(
  ID_EVENT_AMPLITUDE_SPIN, GOOrganSettingsPipesTab::OnAmplitudeSpinChanged)
EVT_TEXT(ID_EVENT_GAIN, GOOrganSettingsPipesTab::OnGainChanged)
EVT_SPIN(ID_EVENT_GAIN_SPIN, GOOrganSettingsPipesTab::OnGainSpinChanged)
EVT_TEXT(ID_EVENT_MANUAL_TUNING, GOOrganSettingsPipesTab::OnManualTuningChanged)
EVT_SPIN(
  ID_EVENT_MANUAL_TUNING_SPIN,
  GOOrganSettingsPipesTab::OnManualTuningSpinChanged)
EVT_TEXT(
  ID_EVENT_AUTO_TUNING_CORRECTION,
  GOOrganSettingsPipesTab::OnAutoTuningCorrectionChanged)
EVT_SPIN(
  ID_EVENT_AUTO_TUNING_CORRECTION_SPIN,
  GOOrganSettingsPipesTab::OnAutoTuningCorrectionSpinChanged)
EVT_TEXT(ID_EVENT_DELAY, GOOrganSettingsPipesTab::OnDelayChanged)
EVT_SPIN(ID_EVENT_DELAY_SPIN, GOOrganSettingsPipesTab::OnDelaySpinChanged)
EVT_TEXT(
  ID_EVENT_RELEASE_LENGTH, GOOrganSettingsPipesTab::OnReleaseLengthChanged)
EVT_SPIN(
  ID_EVENT_RELEASE_LENGTH_SPIN,
  GOOrganSettingsPipesTab::OnReleaseLengthSpinChanged)
EVT_TEXT(ID_EVENT_TONE_BALANCE, GOOrganSettingsPipesTab::OnToneBalanceChanged)
EVT_SPIN(
  ID_EVENT_TONE_BALANCE_SPIN, GOOrganSettingsPipesTab::OnToneBalanceSpinChanged)
EVT_TEXT(ID_EVENT_AUDIO_GROUP, GOOrganSettingsPipesTab::OnAudioGroupChanged)
EVT_CHECKBOX(
  ID_EVENT_IGNORE_PITCH, GOOrganSettingsPipesTab::OnIgnorePitchChanged)
EVT_CHOICE(
  ID_EVENT_BITS_PER_SAMPLE, GOOrganSettingsPipesTab::OnBitsPerSampleChanged)
EVT_CHOICE(ID_EVENT_COMPRESS, GOOrganSettingsPipesTab::OnCompressChanged)
EVT_CHOICE(ID_EVENT_CHANNELS, GOOrganSettingsPipesTab::OnChannelsChanged)
EVT_CHOICE(ID_EVENT_LOOP_LOAD, GOOrganSettingsPipesTab::OnLoopLoadChanged)
EVT_CHOICE(ID_EVENT_ATTACK_LOAD, GOOrganSettingsPipesTab::OnAttackLoadChanged)
EVT_CHOICE(ID_EVENT_RELEASE_LOAD, GOOrganSettingsPipesTab::OnReleaseLoadChanged)
END_EVENT_TABLE()

GOOrganSettingsPipesTab::GOOrganSettingsPipesTab(
  GOOrganModel &organModel, GOOrganSettingsDialogBase *pDlg)
  : GOOrganSettingsTab(pDlg, WX_TAB_CODE, WX_TAB_TITLE),
    r_config(organModel.GetConfig()),
    r_RootNode(organModel.GetRootPipeConfigNode()),
    p_LastTreeItemData(nullptr),
    m_LoadChangeCnt(0) {
  wxGridBagSizer *const mainSizer = new wxGridBagSizer(5, 5);

  m_Tree = new wxTreeCtrl(
    this,
    ID_EVENT_TREE,
    wxDefaultPosition,
    wxDefaultSize,
    wxTR_HAS_BUTTONS | wxTR_MULTIPLE);
  mainSizer->Add(
    m_Tree,
    wxGBPosition(0, 0),
    wxGBSpan(4, 1),
    wxEXPAND | wxTOP | wxLEFT | wxBOTTOM,
    5);
  mainSizer->AddGrowableCol(0, 1);
  mainSizer->AddGrowableRow(3, 1);

  wxBoxSizer *box1
    = new wxStaticBoxSizer(wxVERTICAL, this, _("Sample information"));

  wxFlexGridSizer *grid = new wxFlexGridSizer(2, 5, 5);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Memory usage:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_MemoryDisplay = new wxStaticText(this, wxID_ANY, wxEmptyString);
  grid->Add(m_MemoryDisplay);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Bits per sample:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_BitDisplay = new wxStaticText(this, wxID_ANY, wxEmptyString);
  grid->Add(m_BitDisplay);
  box1->Add(grid, 0, wxEXPAND | wxALL, 5);
  mainSizer->Add(
    box1, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND | wxRIGHT, 5);

  box1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Settings"));

  wxGridBagSizer *gb = new wxGridBagSizer(5, 5);

  gb->Add(
    new wxStaticText(this, wxID_ANY, _("Amplitude:")),
    wxGBPosition(0, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Amplitude = new wxTextCtrl(
    this, ID_EVENT_AMPLITUDE, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  gb->Add(m_Amplitude, wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND);
  m_AmplitudeSpin = new wxSpinButton(this, ID_EVENT_AMPLITUDE_SPIN);
  m_AmplitudeSpin->SetRange(0, 1000);
  gb->Add(m_AmplitudeSpin, wxGBPosition(0, 3), wxDefaultSpan);

  gb->Add(
    new wxStaticText(this, wxID_ANY, _("Gain (dB):")),
    wxGBPosition(1, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Gain = new wxTextCtrl(
    this, ID_EVENT_GAIN, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  gb->Add(m_Gain, wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);
  m_GainSpin = new wxSpinButton(this, ID_EVENT_GAIN_SPIN);
  m_GainSpin->SetRange(-120, 40);
  gb->Add(m_GainSpin, wxGBPosition(1, 3), wxDefaultSpan);

  gb->Add(
    new wxStaticText(this, wxID_ANY, _("Manual Tuning (Cent):")),
    wxGBPosition(2, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_ManualTuning = new wxTextCtrl(
    this, ID_EVENT_MANUAL_TUNING, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  gb->Add(m_ManualTuning, wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND);
  m_ManualTuningSpin = new wxSpinButton(this, ID_EVENT_MANUAL_TUNING_SPIN);
  m_ManualTuningSpin->SetRange(-1800, 1800);
  gb->Add(m_ManualTuningSpin, wxGBPosition(2, 3));
  gb->Add(
    new wxStaticText(this, wxID_ANY, _("AutoTuning Correction (Cent):")),
    wxGBPosition(3, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_AutoTuningCorrection = new wxTextCtrl(
    this,
    ID_EVENT_AUTO_TUNING_CORRECTION,
    wxEmptyString,
    wxDefaultPosition,
    EDIT_SIZE);
  gb->Add(m_AutoTuningCorrection, wxGBPosition(3, 2), wxDefaultSpan, wxEXPAND);
  m_AutoTuningCorrectionSpin
    = new wxSpinButton(this, ID_EVENT_AUTO_TUNING_CORRECTION_SPIN);
  m_AutoTuningCorrectionSpin->SetRange(-1800, 1800);
  gb->Add(m_AutoTuningCorrectionSpin, wxGBPosition(3, 3));

  gb->Add(
    new wxStaticText(this, wxID_ANY, _("Tracker (ms):")),
    wxGBPosition(4, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Delay = new wxTextCtrl(
    this, ID_EVENT_DELAY, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  gb->Add(m_Delay, wxGBPosition(4, 2), wxDefaultSpan, wxEXPAND);
  m_DelaySpin = new wxSpinButton(this, ID_EVENT_DELAY_SPIN);
  m_DelaySpin->SetRange(0, 10000);
  gb->Add(m_DelaySpin, wxGBPosition(4, 3), wxDefaultSpan);

  gb->Add(
    new wxStaticText(this, wxID_ANY, _("Release Tail Length (ms):")),
    wxGBPosition(5, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_ReleaseLength = new wxTextCtrl(
    this, ID_EVENT_RELEASE_LENGTH, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  gb->Add(m_ReleaseLength, wxGBPosition(5, 2), wxDefaultSpan, wxEXPAND);
  m_ReleaseLengthSpin = new wxSpinButton(this, ID_EVENT_RELEASE_LENGTH_SPIN);
  m_ReleaseLengthSpin->SetRange(0, RELEASE_LENGTH_MAX_INDEX);
  gb->Add(m_ReleaseLengthSpin, wxGBPosition(5, 3), wxDefaultSpan);

  gb->Add(
    new wxStaticText(this, wxID_ANY, _("Tone balance:")),
    wxGBPosition(6, 0),
    wxGBSpan(1, 2),
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_ToneBalance = new wxTextCtrl(
    this, ID_EVENT_TONE_BALANCE, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  gb->Add(m_ToneBalance, wxGBPosition(6, 2), wxDefaultSpan, wxEXPAND);
  m_ToneBalanceSpin = new wxSpinButton(this, ID_EVENT_TONE_BALANCE_SPIN);
  m_ToneBalanceSpin->SetRange(-100, 100);
  gb->Add(m_ToneBalanceSpin, wxGBPosition(6, 3), wxDefaultSpan);

  gb->Add(
    new wxStaticText(this, wxID_ANY, _("Audio group:")),
    wxGBPosition(7, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_AudioGroup = new wxComboBox(
    this, ID_EVENT_AUDIO_GROUP, wxEmptyString, wxDefaultPosition, EDIT_SIZE);
  m_AudioGroup->Append(wxEmptyString);
  gb->Add(m_AudioGroup, wxGBPosition(7, 1), wxGBSpan(1, 3), wxEXPAND);

  gb->Add(
    m_IgnorePitch = new wxCheckBox(
      this,
      ID_EVENT_IGNORE_PITCH,
      _("Ignore pitch info in organ samples wav files")),
    wxGBPosition(8, 0),
    wxGBSpan(1, 4),
    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
    5);

  gb->AddGrowableCol(2, 1);
  box1->Add(gb, 0, wxEXPAND | wxALL, 5);
  mainSizer->Add(
    box1, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND | wxRIGHT, 5);

  box1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Sample Loading"));
  grid = new wxFlexGridSizer(2, 5, 5);

  grid->AddGrowableCol(1, 1);

  wxArrayString choices;

  choices.push_back(_("Parent default"));
  for (unsigned i = 8; i <= 24; i++)
    choices.push_back(wxString::Format(_("%d bits"), i));
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Sample Size:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_BitsPerSample = new wxChoice(
    this, ID_EVENT_BITS_PER_SAMPLE, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_BitsPerSample, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Disabled"));
  choices.push_back(_("Enabled"));
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Lossless compression:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Compress = new wxChoice(
    this, ID_EVENT_COMPRESS, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_Compress, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Don't load"));
  choices.push_back(_("Mono"));
  choices.push_back(_("Stereo"));
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Sample channels:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_Channels = new wxChoice(
    this, ID_EVENT_CHANNELS, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_Channels, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("First loop"));
  choices.push_back(_("Longest loop"));
  choices.push_back(_("All loops"));
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Loop loading:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_LoopLoad = new wxChoice(
    this, ID_EVENT_LOOP_LOAD, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_LoopLoad, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Single attack"));
  choices.push_back(_("All"));
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Attack loading:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_AttackLoad = new wxChoice(
    this, ID_EVENT_ATTACK_LOAD, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_AttackLoad, 1, wxEXPAND);

  choices.clear();
  choices.push_back(_("Parent default"));
  choices.push_back(_("Single release"));
  choices.push_back(_("All"));
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Release loading:")),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxBOTTOM,
    5);
  m_ReleaseLoad = new wxChoice(
    this, ID_EVENT_RELEASE_LOAD, wxDefaultPosition, wxDefaultSize, choices);
  grid->Add(m_ReleaseLoad, 1, wxEXPAND);

  m_LastIgnorePitch = false;
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
  mainSizer->Add(
    box1, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND | wxRIGHT, 5);

  SetSizerAndFit(mainSizer);
}

static const wxString WX_SPACE = wxT(" ");

wxTreeItemId GOOrganSettingsPipesTab::FillTree(
  wxTreeItemId parent, GOPipeConfigNode &config) {
  wxTreeItemData *data = new TreeItemData(config);
  wxTreeItemId e;
  if (!parent.IsOk())
    e = m_Tree->AddRoot(config.GetName(), -1, -1, data);
  else
    e = m_Tree->AppendItem(parent, config.GetName(), -1, -1, data);
  for (unsigned i = 0; i < config.GetChildCount(); i++)
    FillTree(e, *config.GetChild(i));
  return e;
}

bool GOOrganSettingsPipesTab::TransferDataToWindow() {
  m_AudioGroup->Clear();
  for (const auto &group : r_config.GetAudioGroups())
    m_AudioGroup->Append(group);
  m_AudioGroup->ChangeValue(WX_SPACE);
  m_LastAudioGroup = WX_SPACE;

  wxTreeItemId idRoot = FillTree(wxTreeItemId(), r_RootNode);

  NotifyModified(false);

  m_Tree->Expand(idRoot);
  m_Tree->SelectItem(idRoot, true);
  return true;
}

void GOOrganSettingsPipesTab::SetEmpty(wxChoice *choice) {
  int index = choice->FindString(wxEmptyString);
  if (index == wxNOT_FOUND)
    index = choice->Append(wxEmptyString);
  choice->SetSelection(index);
}

void GOOrganSettingsPipesTab::RemoveEmpty(wxChoice *choice) {
  int sel = choice->GetSelection();
  int index = choice->FindString(wxEmptyString);
  if (index != wxNOT_FOUND)
    choice->Delete(index);
  choice->SetSelection(sel);
}

static const wxString WX__U = wxT("%u");
static const wxString WX_MAX = wxT("Max");

/**
 * Parses releaseLengthStr. Max is converted to 0
 * @param releaseLengthStr a string to parse
 * @param releaseLength receives the new value if the string value is valid
 * @return is the string value valid
 */
static bool str_to_release_length(
  const wxString &releaseLengthStr, unsigned &releaseLength) {
  bool isValid = true;
  long releaseLengthLong = 0;

  if (releaseLengthStr == WX_MAX)
    releaseLength = 0;
  else if ((isValid = releaseLengthStr.ToLong(&releaseLengthLong)
              && releaseLengthLong >= 0
              && releaseLengthLong <= RELEASE_LENGTH_MAX))
    releaseLength = (unsigned)releaseLengthLong;
  return isValid;
}

/**
 * Converts the supplied value to string. 0 is converted to Max
 * @param releaseLength the new value
 * @return the string value
 */
static wxString release_length_to_str(unsigned releaseLength) {
  return releaseLength ? wxString::Format(WX__U, releaseLength) : WX_MAX;
}

/**
 * Converts ReleaseLengthSpin index to ReleaseLength. 0 -> 50, 59 -> 3000, 60 ->
 * 0
 * @return the ReleaseLength corresponding the current spin index
 */
static unsigned spin_index_to_release_length(int index) {
  unsigned releaseLength = ((unsigned)index + 1) * RELEASE_LENGTH_STEP;

  return releaseLength <= RELEASE_LENGTH_MAX ? releaseLength : 0;
}

/**
 * Converts ReleaseLengthSpin index to the specified ReleaseLength
 * 0 -> 60, 50 -> 0, 59 -> 3000
 * @return the ReleaseLength corresponding the current spin index
 */
static int release_length_to_spin_index(unsigned releaseLength) {
  unsigned index = releaseLength / RELEASE_LENGTH_STEP;

  return index ? index - 1 : RELEASE_LENGTH_MAX_INDEX;
}

void GOOrganSettingsPipesTab::Load(bool isForce) {
  wxArrayTreeItemIds selectedItemIds;

  m_Tree->GetSelections(selectedItemIds);
  for (unsigned i = 0; i < selectedItemIds.size(); i++) {
    if (!m_Tree->GetItemData(selectedItemIds[i])) {
      wxLogError(
        _("Invalid item selected: %s"),
        m_Tree->GetItemText(selectedItemIds[i]).c_str());
      selectedItemIds.RemoveAt(i, 1);
      i--;
    }
  }

  GOSampleStatistic stat;

  for (unsigned l = selectedItemIds.size(), i = 0; i < l; i++)
    if (m_Tree->GetItemData(selectedItemIds[i]))
      stat.Cumulate(((TreeItemData *)m_Tree->GetItemData(selectedItemIds[i]))
                      ->r_node.GetStatistic());

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

  if (selectedItemIds.size() == 0) {
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
    m_ToneBalance->ChangeValue(wxEmptyString);
    m_ToneBalance->Disable();
    m_ToneBalanceSpin->SetValue(0);
    m_ToneBalanceSpin->Disable();
    m_AudioGroup->Disable();
    m_IgnorePitch->Disable();
    m_BitsPerSample->Disable();
    m_Compress->Disable();
    m_Channels->Disable();
    m_LoopLoad->Disable();
    m_AttackLoad->Disable();
    m_ReleaseLoad->Disable();
    m_IsDefaultEnabled = false;
    SetModified(false);
  }
  m_IsDistributeAudioEnabled = true;

  const bool isSingleSelection = selectedItemIds.size() == 1;

  if (isSingleSelection || isForce) {
    // all values will be rendered, so mark they as not modified
    SetModified(false);
  }

  if (!isSingleSelection) {
    if (!m_Amplitude->IsModified() || isForce) {
      m_Amplitude->ChangeValue(wxEmptyString);
      m_Amplitude->DiscardEdits();
    }
    if (!m_Gain->IsModified() || isForce) {
      m_Gain->ChangeValue(wxEmptyString);
      m_Gain->DiscardEdits();
    }
    if (!m_ManualTuning->IsModified() || isForce) {
      m_ManualTuning->ChangeValue(wxEmptyString);
      m_ManualTuning->DiscardEdits();
    }
    if (!m_AutoTuningCorrection->IsModified() || isForce) {
      m_AutoTuningCorrection->ChangeValue(wxEmptyString);
      m_AutoTuningCorrection->DiscardEdits();
    }
    if (!m_Delay->IsModified() || isForce) {
      m_Delay->ChangeValue(wxEmptyString);
      m_Delay->DiscardEdits();
    }
    if (!m_ReleaseLength->IsModified() || isForce) {
      m_ReleaseLength->ChangeValue(wxEmptyString);
      m_ReleaseLength->DiscardEdits();
    }
    if (!m_ToneBalance->IsModified())
      m_ToneBalance->ChangeValue(wxEmptyString);
    if (m_AudioGroup->GetValue() == m_LastAudioGroup || isForce) {
      m_AudioGroup->ChangeValue(WX_SPACE);
      m_LastAudioGroup = WX_SPACE;
    }
    if (m_IgnorePitch->IsChecked() == m_LastIgnorePitch || isForce) {
      m_IgnorePitch->SetValue(false);
      m_LastIgnorePitch = false;
    }
    if (m_BitsPerSample->GetSelection() == m_LastBitsPerSample || isForce) {
      SetEmpty(m_BitsPerSample);
      m_LastBitsPerSample = m_BitsPerSample->GetSelection();
    }
    if (m_Compress->GetSelection() == m_LastCompress || isForce) {
      SetEmpty(m_Compress);
      m_LastCompress = m_Compress->GetSelection();
    }
    if (m_Channels->GetSelection() == m_LastChannels || isForce) {
      SetEmpty(m_Channels);
      m_LastChannels = m_Channels->GetSelection();
    }
    if (m_LoopLoad->GetSelection() == m_LastLoopLoad || isForce) {
      SetEmpty(m_LoopLoad);
      m_LastLoopLoad = m_LoopLoad->GetSelection();
    }
    if (m_AttackLoad->GetSelection() == m_LastAttackLoad || isForce) {
      SetEmpty(m_AttackLoad);
      m_LastAttackLoad = m_AttackLoad->GetSelection();
    }
    if (m_ReleaseLoad->GetSelection() == m_LastReleaseLoad || isForce) {
      SetEmpty(m_ReleaseLoad);
      m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
    }
  }

  bool isLastSelected = false;

  if (p_LastTreeItemData)
    for (unsigned i = 0; i < selectedItemIds.size(); i++)
      if (
        p_LastTreeItemData
        && m_Tree->GetItemData(selectedItemIds[i]) == p_LastTreeItemData) {
        isLastSelected = true;
        break;
      }

  if (!isLastSelected) {
    p_LastTreeItemData = nullptr;
    for (unsigned i = 0; i < selectedItemIds.size() && !p_LastTreeItemData; i++)
      p_LastTreeItemData
        = (TreeItemData *)m_Tree->GetItemData(selectedItemIds[i]);

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
    m_ToneBalance->Enable();
    m_ToneBalanceSpin->Enable();
    m_AudioGroup->Enable();
    m_IgnorePitch->Enable();
    m_BitsPerSample->Enable();
    m_Compress->Enable();
    m_Channels->Enable();
    m_LoopLoad->Enable();
    m_AttackLoad->Enable();
    m_ReleaseLoad->Enable();
    m_IsDefaultEnabled = true;

    if (isSingleSelection) {
      // fill the fields with values from the selected object
      float amplitude = p_LastTreeItemData->r_config.GetAmplitude();
      float gain = p_LastTreeItemData->r_config.GetGain();
      float manualTuning = p_LastTreeItemData->r_config.GetManualTuning();
      float autoTuningCorrection
        = p_LastTreeItemData->r_config.GetAutoTuningCorrection();
      unsigned delay = p_LastTreeItemData->r_config.GetDelay();
      unsigned releaseLength
        = p_LastTreeItemData->r_node.GetEffectiveReleaseTail();
      int toneBalance = p_LastTreeItemData->r_config.GetToneBalanceValue();

      m_Amplitude->ChangeValue(wxString::Format(wxT("%.1f"), amplitude));
      m_Amplitude->DiscardEdits();
      m_AmplitudeSpin->SetValue(amplitude);
      m_Gain->ChangeValue(wxString::Format(wxT("%.1f"), gain));
      m_Gain->DiscardEdits();
      m_GainSpin->SetValue(gain);
      m_ManualTuning->ChangeValue(wxString::Format(wxT("%.1f"), manualTuning));
      m_ManualTuning->DiscardEdits();
      m_ManualTuningSpin->SetValue(manualTuning);
      m_AutoTuningCorrection->ChangeValue(
        wxString::Format(wxT("%.1f"), autoTuningCorrection));
      m_AutoTuningCorrection->DiscardEdits();
      m_AutoTuningCorrectionSpin->SetValue(autoTuningCorrection);
      m_Delay->ChangeValue(wxString::Format(wxT("%u"), delay));
      m_Delay->DiscardEdits();
      m_DelaySpin->SetValue(delay);
      m_ReleaseLength->ChangeValue(release_length_to_str(releaseLength));
      m_ReleaseLength->DiscardEdits();
      m_ReleaseLengthSpin->SetValue(
        release_length_to_spin_index(releaseLength));
      m_ToneBalance->ChangeValue(wxString::Format(wxT("%i"), toneBalance));
      m_ToneBalanceSpin->SetValue(toneBalance);
      m_AudioGroup->ChangeValue(p_LastTreeItemData->r_config.GetAudioGroup());
      m_IgnorePitch->SetValue(
        p_LastTreeItemData->r_node.GetEffectiveIgnorePitch());

      int bits_per_sample = p_LastTreeItemData->r_config.GetBitsPerSample();
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
      m_Compress->SetSelection(p_LastTreeItemData->r_config.GetCompress() + 1);
      m_Channels->SetSelection(p_LastTreeItemData->r_config.GetChannels() + 1);
      m_LoopLoad->SetSelection(p_LastTreeItemData->r_config.GetLoopLoad() + 1);
      m_AttackLoad->SetSelection(
        p_LastTreeItemData->r_config.GetAttackLoad() + 1);
      m_ReleaseLoad->SetSelection(
        p_LastTreeItemData->r_config.GetReleaseLoad() + 1);

      m_LastAudioGroup = m_AudioGroup->GetValue();
      m_LastIgnorePitch = m_IgnorePitch->IsChecked();
      m_LastBitsPerSample = m_BitsPerSample->GetSelection();
      m_LastCompress = m_Compress->GetSelection();
      m_LastChannels = m_Channels->GetSelection();
      m_LastLoopLoad = m_LoopLoad->GetSelection();
      m_LastAttackLoad = m_AttackLoad->GetSelection();
      m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
    }
  }
  NotifyButtonStatesChanged();
}

void GOOrganSettingsPipesTab::UpdateAudioGroup(
  const std::vector<wxString> &audio_group, unsigned &pos, wxTreeItemId item) {
  TreeItemData *e = (TreeItemData *)m_Tree->GetItemData(item);
  if (e) {
    e->r_config.SetAudioGroup(audio_group[pos]);
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

void GOOrganSettingsPipesTab::OnTreeChanging(wxTreeEvent &e) {
  if (CheckForUnapplied())
    e.Veto();
}

void GOOrganSettingsPipesTab::OnTreeChanged(wxTreeEvent &e) {
  wxArrayTreeItemIds entries;
  if (m_LoadChangeCnt)
    return;
  m_LoadChangeCnt++;
  do {
    bool rescan = false;
    m_Tree->GetSelections(entries);
    for (unsigned i = 0; i < entries.size(); i++) {
      TreeItemData *e = (TreeItemData *)m_Tree->GetItemData(entries[i]);
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

void GOOrganSettingsPipesTab::OnAmplitudeChanged(wxCommandEvent &e) {
  double amp;
  if (m_Amplitude->GetValue().ToDouble(&amp))
    m_AmplitudeSpin->SetValue(amp);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnAmplitudeSpinChanged(wxSpinEvent &e) {
  m_Amplitude->ChangeValue(
    wxString::Format(wxT("%.1f"), (float)m_AmplitudeSpin->GetValue()));
  m_Amplitude->MarkDirty();
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnGainChanged(wxCommandEvent &e) {
  double gain;
  if (m_Gain->GetValue().ToDouble(&gain))
    m_GainSpin->SetValue(gain);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnGainSpinChanged(wxSpinEvent &e) {
  m_Gain->ChangeValue(
    wxString::Format(wxT("%.1f"), (float)m_GainSpin->GetValue()));
  m_Gain->MarkDirty();
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnManualTuningChanged(wxCommandEvent &e) {
  double tuning;

  if (m_ManualTuning->GetValue().ToDouble(&tuning))
    m_ManualTuningSpin->SetValue(tuning);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnManualTuningSpinChanged(wxSpinEvent &e) {
  m_ManualTuning->ChangeValue(
    wxString::Format(wxT("%.1f"), (float)m_ManualTuningSpin->GetValue()));
  m_ManualTuning->MarkDirty();
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnAutoTuningCorrectionChanged(wxCommandEvent &e) {
  double tuning;

  if (m_AutoTuningCorrection->GetValue().ToDouble(&tuning))
    m_AutoTuningCorrectionSpin->SetValue(tuning);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnAutoTuningCorrectionSpinChanged(
  wxSpinEvent &e) {
  m_AutoTuningCorrection->ChangeValue(wxString::Format(
    wxT("%.1f"), (float)m_AutoTuningCorrectionSpin->GetValue()));
  m_AutoTuningCorrection->MarkDirty();
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnDelayChanged(wxCommandEvent &e) {
  long delay;
  if (m_Delay->GetValue().ToLong(&delay))
    m_DelaySpin->SetValue(delay);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnDelaySpinChanged(wxSpinEvent &e) {
  m_Delay->ChangeValue(
    wxString::Format(wxT("%u"), (unsigned)m_DelaySpin->GetValue()));
  m_Delay->MarkDirty();
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnReleaseLengthChanged(wxCommandEvent &e) {
  unsigned releaseLength;

  if (str_to_release_length(m_ReleaseLength->GetValue(), releaseLength))
    m_ReleaseLengthSpin->SetValue(release_length_to_spin_index(releaseLength));
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnReleaseLengthSpinChanged(wxSpinEvent &e) {
  m_ReleaseLength->ChangeValue(release_length_to_str(
    spin_index_to_release_length(m_ReleaseLengthSpin->GetValue())));
  m_ReleaseLength->MarkDirty();
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnToneBalanceChanged(wxCommandEvent &e) {
  long value;

  if (m_ToneBalance->GetValue().ToLong(&value))
    m_ToneBalanceSpin->SetValue(value);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnToneBalanceSpinChanged(wxSpinEvent &e) {
  m_ToneBalance->ChangeValue(
    wxString::Format(wxT("%i"), m_ToneBalanceSpin->GetValue()));
  m_ToneBalance->MarkDirty();
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnBitsPerSampleChanged(wxCommandEvent &e) {
  RemoveEmpty(m_BitsPerSample);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnCompressChanged(wxCommandEvent &e) {
  RemoveEmpty(m_Compress);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnChannelsChanged(wxCommandEvent &e) {
  RemoveEmpty(m_Channels);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnLoopLoadChanged(wxCommandEvent &e) {
  RemoveEmpty(m_LoopLoad);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnAttackLoadChanged(wxCommandEvent &e) {
  RemoveEmpty(m_AttackLoad);
  NotifyModified();
}

void GOOrganSettingsPipesTab::OnReleaseLoadChanged(wxCommandEvent &e) {
  RemoveEmpty(m_ReleaseLoad);
  NotifyModified();
}

void GOOrganSettingsPipesTab::DistributeAudio() {
  if (CheckForUnapplied())
    return;
  wxArrayString strs;
  std::vector<wxString> group_list = r_config.GetAudioGroups();
  for (unsigned i = 0; i < group_list.size(); i++)
    strs.Add(group_list[i]);

  wxMultiChoiceDialog dlg(
    this, _("Select audio groups to distribute:"), _("Organ dialog"), strs);
  if (dlg.ShowModal() != wxID_OK) {
    return;
  }
  wxArrayInt sel = dlg.GetSelections();
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
  p_LastTreeItemData = NULL;
  Load(false);
}

void GOOrganSettingsPipesTab::ResetToDefault() {
  wxArrayTreeItemIds entries;
  m_Tree->GetSelections(entries);

  std::unordered_set<TreeItemData *> oiSet;
  bool hasChildren = false;

  // Fill idSet with entries
  for (const wxTreeItemId id : entries) {
    TreeItemData *p = (TreeItemData *)m_Tree->GetItemData(id);
    wxTreeItemIdValue cookie;

    if (p)
      oiSet.insert(p);
    if (!hasChildren && m_Tree->GetFirstChild(id, cookie).IsOk())
      hasChildren = true;
  }

  int wxForChildren = hasChildren
    ? wxMessageBox(
      _("Some selected objects have subobjects.\n"
        "Do you want the subobjects also to be reset to defaults?\n"
        "Note: resetting to defaults cannot be undone.\n"
        "Note: resetting all subobjects may take some time."),
      _("Resetting to defaults"),
      wxCENTRE | wxYES_NO | wxCANCEL | wxNO_DEFAULT,
      this)
    : wxNO;

  if (wxForChildren != wxCANCEL) {
    // add the children
    if (wxForChildren == wxYES) {
      // entries.size() it may not be precalculated because it is increased
      for (unsigned i = 0; i < entries.size(); i++) {
        const wxTreeItemId id = entries[i];
        wxTreeItemIdValue cookie;

        for (wxTreeItemId c = m_Tree->GetFirstChild(id, cookie); c.IsOk();
             c = m_Tree->GetNextChild(id, cookie)) {
          TreeItemData *p = (TreeItemData *)m_Tree->GetItemData(c);

          if (p && oiSet.find(p) == oiSet.end()) {
            oiSet.insert(p);
            entries.push_back(c);
            // c's children will be scanned later in the loop
          }
        }
      }
    }

    for (TreeItemData *e : oiSet) {
      e->r_config.SetAmplitude(e->r_config.GetDefaultAmplitude());
      e->r_config.SetGain(e->r_config.GetDefaultGain());
      e->r_config.SetManualTuning(0);
      e->r_config.SetAutoTuningCorrection(0);
      e->r_config.SetDelay(e->r_config.GetDefaultDelay());
      e->r_config.SetReleaseTail(0);
      e->r_config.SetToneBalanceValue(0);
      e->r_config.SetBitsPerSample(-1);
      e->r_config.SetChannels(-1);
      e->r_config.SetLoopLoad(-1);
      e->r_config.SetCompress(BOOL3_DEFAULT);
      e->r_config.SetAttackLoad(BOOL3_DEFAULT);
      e->r_config.SetReleaseLoad(BOOL3_DEFAULT);
      e->r_config.SetIgnorePitch(BOOL3_DEFAULT);
    }

    p_LastTreeItemData = NULL;
    Load(true);
  }
}

void GOOrganSettingsPipesTab::DiscardChanges() {
  p_LastTreeItemData = NULL;
  Load(true);
}

void GOOrganSettingsPipesTab::ApplyChanges() {
  double amp, gain, manualTuning, autoTuningCorrection;
  long delay, toneBalance;
  unsigned releaseLength;

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
    GOMessageBox(
      _("ManualTuning is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  if (
    !m_AutoTuningCorrection->GetValue().ToDouble(&autoTuningCorrection)
    && (m_AutoTuningCorrection->IsModified() && (autoTuningCorrection < -1800 || autoTuningCorrection > 1800))) {
    GOMessageBox(
      _("AutoTuningCorrection is invalid"),
      _("Error"),
      wxOK | wxICON_ERROR,
      this);
    return;
  }

  if (
    !m_Delay->GetValue().ToLong(&delay)
    && (m_Delay->IsModified() && (delay < 0 || delay > 10000))) {
    GOMessageBox(
      _("Tracker delay is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  const wxString releaseLengthStr = m_ReleaseLength->GetValue();
  const bool isReleaseLengthPresent = !releaseLengthStr.IsEmpty();

  if (
    isReleaseLengthPresent
    && !str_to_release_length(releaseLengthStr, releaseLength)) {
    GOMessageBox(
      _("Release Length is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  if (
    !m_ToneBalance->GetValue().ToLong(&toneBalance)
    && (m_ToneBalance->IsModified() && (toneBalance < -100 || toneBalance > 100))) {
    GOMessageBox(
      _("ToneBalance value is invalid"), _("Error"), wxOK | wxICON_ERROR, this);
    return;
  }

  for (unsigned i = 0; i < entries.size(); i++) {
    TreeItemData *e = (TreeItemData *)m_Tree->GetItemData(entries[i]);
    if (!e)
      continue;

    GOPipeConfigNode *parent = e->r_node.GetParent();

    if (m_Amplitude->IsModified())
      e->r_config.SetAmplitude(amp);
    if (m_Gain->IsModified())
      e->r_config.SetGain(gain);
    if (m_ManualTuning->IsModified())
      e->r_config.SetManualTuning(manualTuning);
    if (m_AutoTuningCorrection->IsModified())
      e->r_config.SetAutoTuningCorrection(autoTuningCorrection);
    if (m_Delay->IsModified())
      e->r_config.SetDelay(delay);
    if (isReleaseLengthPresent && m_ReleaseLength->IsModified()) {
      long parentReleaseLength = parent ? parent->GetEffectiveReleaseTail() : 0;
      bool isLessThanParent = releaseLength > 0
        && (!parentReleaseLength || releaseLength < parentReleaseLength);

      // calculate new effective release length
      if (!isLessThanParent)
        releaseLength = parentReleaseLength;
      e->r_config.SetReleaseTail(isLessThanParent ? releaseLength : 0);
    }
    if (m_ToneBalance->IsModified())
      e->r_config.SetToneBalanceValue(toneBalance);
    if (m_AudioGroup->GetValue() != m_LastAudioGroup)
      e->r_config.SetAudioGroup(m_AudioGroup->GetValue().Trim());
    if (m_BitsPerSample->GetSelection() != m_LastBitsPerSample)
      e->r_config.SetBitsPerSample(
        m_BitsPerSample->GetSelection() == 0
          ? (int8_t)-1
          : (int8_t)(m_BitsPerSample->GetSelection() + 7));
    if (m_Channels->GetSelection() != m_LastChannels)
      e->r_config.SetChannels((int8_t)(m_Channels->GetSelection() - 1));
    if (m_LoopLoad->GetSelection() != m_LastLoopLoad)
      e->r_config.SetLoopLoad((int8_t)(m_LoopLoad->GetSelection() - 1));
    if (m_Compress->GetSelection() != m_LastCompress)
      e->r_config.SetCompress(to_bool3(m_Compress->GetSelection() - 1));
    if (m_AttackLoad->GetSelection() != m_LastAttackLoad)
      e->r_config.SetAttackLoad(to_bool3(m_AttackLoad->GetSelection() - 1));
    if (m_ReleaseLoad->GetSelection() != m_LastReleaseLoad)
      e->r_config.SetReleaseLoad(to_bool3(m_ReleaseLoad->GetSelection() - 1));

    bool ignorePitch = m_IgnorePitch->IsChecked();

    if (ignorePitch != m_LastIgnorePitch) {
      bool parentIgnorePitch
        = parent ? parent->GetEffectiveIgnorePitch() : false;

      e->r_config.SetIgnorePitch(
        ignorePitch == parentIgnorePitch ? BOOL3_DEFAULT
                                         : to_bool3(ignorePitch));
    }
  }
  if (m_Amplitude->IsModified()) {
    m_Amplitude->ChangeValue(wxString::Format(wxT("%.1f"), amp));
    m_Amplitude
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_Gain->IsModified()) {
    m_Gain->ChangeValue(wxString::Format(wxT("%.1f"), gain));
    m_Gain->DiscardEdits(); // workaround of osx implementation bug
                            // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_ManualTuning->IsModified()) {
    m_ManualTuning->ChangeValue(wxString::Format(wxT("%.1f"), manualTuning));
    m_ManualTuning
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_AutoTuningCorrection->IsModified()) {
    m_AutoTuningCorrection->ChangeValue(
      wxString::Format(wxT("%.1f"), autoTuningCorrection));
    m_AutoTuningCorrection
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_Delay->IsModified()) {
    m_Delay->ChangeValue(wxString::Format(wxT("%lu"), delay));
    m_Delay->DiscardEdits(); // workaround of osx implementation bug
                             // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_ReleaseLength->IsModified()) {
    m_ReleaseLength->ChangeValue(release_length_to_str(releaseLength));
    m_ReleaseLength
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  if (m_ToneBalance->IsModified()) {
    m_ToneBalance->ChangeValue(wxString::Format(wxT("%ld"), toneBalance));
    m_ToneBalance
      ->DiscardEdits(); // workaround of osx implementation bug
                        // https://github.com/oleg68/GrandOrgue/issues/87
  }
  m_LastAudioGroup = m_AudioGroup->GetValue();
  m_LastIgnorePitch = m_IgnorePitch->IsChecked();
  m_LastBitsPerSample = m_BitsPerSample->GetSelection();
  m_LastCompress = m_Compress->GetSelection();
  m_LastChannels = m_Channels->GetSelection();
  m_LastLoopLoad = m_LoopLoad->GetSelection();
  m_LastAttackLoad = m_AttackLoad->GetSelection();
  m_LastReleaseLoad = m_ReleaseLoad->GetSelection();
  NotifyModified(false);
}
