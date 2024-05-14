/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsReverb.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "config/GOConfig.h"
#include "files/GOStandardFile.h"
#include "wxcontrols/GOFilePickerCtrl.h"

#include "GOWave.h"

BEGIN_EVENT_TABLE(GOSettingsReverb, wxPanel)
EVT_CHECKBOX(ID_ENABLED, GOSettingsReverb::OnEnabled)
EVT_FILEPICKER_CHANGED(ID_FILE, GOSettingsReverb::OnFileChanged)
EVT_TEXT(ID_GAIN, GOSettingsReverb::OnGainChanged)
EVT_SPIN(ID_GAIN_SPIN, GOSettingsReverb::OnGainSpinChanged)
END_EVENT_TABLE()

GOSettingsReverb::GOSettingsReverb(GOConfig &settings, wxWindow *parent)
  : wxPanel(parent, wxID_ANY), m_config(settings) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  topSizer->Add(
    m_Enabled = new wxCheckBox(
      this,
      ID_ENABLED,
      _("Enable Convolution Reverb"),
      wxDefaultPosition,
      wxDefaultSize),
    0,
    wxALL,
    5);

  wxBoxSizer *boxReverb
    = new wxStaticBoxSizer(wxVERTICAL, this, _("&Convolution reverb"));
  wxFlexGridSizer *grid = new wxFlexGridSizer(2, 5, 5);

  grid->AddGrowableCol(1, 1);
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Impulse response:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  m_File = new GOFilePickerCtrl(
    this,
    ID_FILE,
    _("Select an impulse file"),
    _("*.wav"),
    wxDefaultPosition,
    wxDefaultSize,
    wxFLP_USE_TEXTCTRL | wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_SMALL);
  grid->Add(m_File, 1, wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Delay (ms):")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_Delay = new wxSpinCtrl(
      this, ID_DELAY, wxEmptyString, wxDefaultPosition, wxDefaultSize),
    0,
    wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Start offset:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_StartOffset = new wxSpinCtrl(
      this, ID_START_OFFSET, wxEmptyString, wxDefaultPosition, wxDefaultSize),
    0,
    wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Length:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_Length = new wxSpinCtrl(
      this, ID_LENGTH, wxEmptyString, wxDefaultPosition, wxDefaultSize),
    0,
    wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Channel:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_Channel
    = new wxChoice(this, ID_CHANNEL, wxDefaultPosition, wxDefaultSize),
    0,
    wxALL);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Gain:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

  wxSizer *box2 = new wxBoxSizer(wxHORIZONTAL);

  m_Gain = new wxTextCtrl(this, ID_GAIN, wxEmptyString);
  m_GainSpin = new wxSpinButton(this, ID_GAIN_SPIN);
  box2->Add(m_Gain);
  box2->Add(m_GainSpin);
  grid->Add(box2);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Direct Sound:")),
    0,
    wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  grid->Add(
    m_Direct = new wxCheckBox(
      this, ID_DIRECT, _("Add dirac pulse"), wxDefaultPosition, wxDefaultSize));

  boxReverb->Add(grid, 0, wxEXPAND | wxALL, 5);

  topSizer->Add(boxReverb, 0, wxEXPAND | wxALL, 5);
  this->SetSizer(topSizer);
  topSizer->Fit(this);

  m_GainSpin->SetRange(1, 1000);
  m_Delay->SetRange(0, 10000);

  m_Enabled->SetValue(m_config.ReverbEnabled());
  UpdateEnabled();

  m_StartOffset->SetRange(0, m_config.ReverbStartOffset());
  m_Length->SetRange(0, m_config.ReverbLen());

  m_File->SetPath(m_config.ReverbFile());
  UpdateLimits();

  m_Gain->ChangeValue(wxString::Format(wxT("%f"), m_config.ReverbGain()));
  m_GainSpin->SetValue(m_config.ReverbGain() * 20);
  m_StartOffset->SetValue(m_config.ReverbStartOffset());
  m_Length->SetValue(m_config.ReverbLen());
  m_Channel->SetSelection(m_config.ReverbChannel() - 1);
  m_Direct->SetValue(m_config.ReverbDirect());
  m_Delay->SetValue(m_config.ReverbDelay());
}

void GOSettingsReverb::UpdateLimits() {
  if (m_File->GetPath() == wxEmptyString || !m_Enabled->GetValue()) {
    m_StartOffset->Disable();
    m_Length->Disable();
    m_Channel->Disable();
    return;
  }
  GOWave wav;
  try {
    GOStandardFile file(m_File->GetPath());
    wav.Open(&file);
    m_StartOffset->Enable();
    m_Length->Enable();
    m_Channel->Enable();

    m_Channel->Clear();
    for (unsigned i = 1; i <= wav.GetChannels(); i++)
      m_Channel->Append(wxString::Format(wxT("%d"), i));
    m_Channel->SetSelection(0);
    m_StartOffset->SetRange(0, wav.GetLength());
    m_Length->SetRange(0, wav.GetLength());
  } catch (wxString error) {
    wxMessageBox(
      error + _("\n\nPlease select a valid impulse response file"),
      _("Reverb error"),
      wxOK | wxICON_ERROR,
      this);
  }
}

void GOSettingsReverb::UpdateFile() {
  UpdateLimits();
  m_Channel->SetSelection(0);
  m_StartOffset->SetValue(0);
  m_Length->SetValue(m_Length->GetMax());
}

void GOSettingsReverb::UpdateEnabled() {
  if (m_Enabled->GetValue()) {
    m_Direct->Enable();
    m_File->Enable();
    m_Channel->Enable();
    m_StartOffset->Enable();
    m_Length->Enable();
    m_Delay->Enable();
    m_Gain->Enable();
    m_GainSpin->Enable();
    UpdateLimits();
  } else {
    m_Direct->Disable();
    m_File->Disable();
    m_Channel->Disable();
    m_StartOffset->Disable();
    m_Length->Disable();
    m_Delay->Disable();
    m_Gain->Disable();
    m_GainSpin->Disable();
  }
}

void GOSettingsReverb::OnEnabled(wxCommandEvent &event) { UpdateEnabled(); }

void GOSettingsReverb::OnFileChanged(wxFileDirPickerEvent &e) { UpdateFile(); }

void GOSettingsReverb::OnGainSpinChanged(wxSpinEvent &e) {
  m_Gain->ChangeValue(
    wxString::Format(wxT("%f"), (float)(m_GainSpin->GetValue() / 20.0)));
}

void GOSettingsReverb::OnGainChanged(wxCommandEvent &e) {
  double gain;
  if (m_Gain->GetValue().ToDouble(&gain))
    m_GainSpin->SetValue(gain * 20);
}

bool GOSettingsReverb::Validate() {
  if (m_Enabled->GetValue()) {
    if (!wxFileExists(m_File->GetPath())) {
      wxMessageBox(
        wxString::Format(
          _("Impulse response file '%s' not found"), m_File->GetPath().c_str()),
        _("Warning"),
        wxOK | wxICON_WARNING,
        this);
      return false;
    }
  }
  return wxPanel::Validate();
}

bool GOSettingsReverb::TransferDataFromWindow() {
  m_config.ReverbEnabled(m_Enabled->IsChecked());
  m_config.ReverbDirect(m_Direct->IsChecked());
  m_config.ReverbFile(m_File->GetPath());
  m_config.ReverbLen(m_Length->GetValue());
  m_config.ReverbDelay(m_Delay->GetValue());
  m_config.ReverbStartOffset(m_StartOffset->GetValue());
  m_config.ReverbChannel(m_Channel->GetSelection() + 1);
  double gain;
  if (m_Gain->GetValue().ToDouble(&gain))
    m_config.ReverbGain(gain);
  return true;
}
