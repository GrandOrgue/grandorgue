/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMidiMatchDialog.h"

#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>

#include "help/GOHelpRequestor.h"

BEGIN_EVENT_TABLE(GOSettingsMidiMatchDialog, wxDialog)
EVT_TEXT(ID_LOGICAL_NAME, GOSettingsMidiMatchDialog::OnLogicalNameChanged)
EVT_TEXT(ID_REGEX, GOSettingsMidiMatchDialog::OnRegexChanged)
EVT_BUTTON(wxID_HELP, GOSettingsMidiMatchDialog::OnHelp)
END_EVENT_TABLE()

GOSettingsMidiMatchDialog::GOSettingsMidiMatchDialog(
  wxWindow *parent, std::vector<GOMidiDeviceConfig *> *otherDevices)
  : wxDialog(
    parent,
    wxID_ANY,
    _("MIDI device matching properties"),
    wxDefaultPosition,
    wxSize(400, -1),
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT),
    p_OtherDevices(otherDevices) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer *const mainSizer = new wxFlexGridSizer(2);

  mainSizer->AddGrowableCol(1, 1);
  mainSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Physical device name:")),
    0,
    wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL,
    5);
  t_PhysicalName = new wxTextCtrl(
    this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(400, -1));
  t_PhysicalName->SetEditable(false);
  mainSizer->Add(t_PhysicalName, 1, wxEXPAND | wxALL, 5);
  mainSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Logical device name:")),
    0,
    wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL,
    5);
  t_LogicalName = new wxTextCtrl(this, ID_LOGICAL_NAME);
  mainSizer->Add(t_LogicalName, 1, wxEXPAND | wxALL, 5);
  mainSizer->Add(
    new wxStaticText(this, wxID_ANY, _("Physical name regex pattern:")),
    0,
    wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL,
    5);
  t_regex = new wxTextCtrl(this, ID_REGEX);
  mainSizer->Add(t_regex, 1, wxEXPAND | wxALL, 5);
  topSizer->Add(mainSizer, 0, wxALL | wxEXPAND, 5);

  topSizer->Add(
    CreateSeparatedButtonSizer(wxOK | wxCANCEL | wxHELP),
    0,
    wxALIGN_RIGHT | wxALL | wxEXPAND,
    5);

  m_StatusBar = new wxStatusBar(this);
  topSizer->Add(m_StatusBar, 0, wxEXPAND);

  SetSizer(topSizer);
  Fit();
}

void GOSettingsMidiMatchDialog::FillWith(const GOMidiDeviceConfig &devConf) {
  m_PhysicalName = devConf.m_PhysicalName;
  t_PhysicalName->ChangeValue(devConf.m_PhysicalName);
  t_LogicalName->ChangeValue(devConf.m_LogicalName);
  t_regex->ChangeValue(devConf.m_RegEx);
}

bool GOSettingsMidiMatchDialog::ValidateLogicalName(wxString &errMsg) {
  bool isValid = true;
  const wxString &newLogicalName = t_LogicalName->GetValue();

  if (newLogicalName.IsEmpty()) {
    errMsg = _("Logical device name must not be empty");
    isValid = false;
  }
  if (isValid && p_OtherDevices)
    // Check logicalName for uniqueness
    for (const GOMidiDeviceConfig *pDev : *p_OtherDevices)
      if (
        pDev->m_PhysicalName != m_PhysicalName
        && pDev->m_LogicalName == newLogicalName) {
        errMsg
          = _("Logical device name is already used by ") + pDev->m_PhysicalName;
        isValid = false;
        break;
      }
  if (
    isValid && newLogicalName != m_PhysicalName
    && t_regex->GetValue().IsEmpty()) {
    errMsg = _("Regex must be specified when physical and logical device names "
               "differ");
    isValid = false;
  }
  if (isValid)
    errMsg = wxEmptyString;
  return isValid;
}

void GOSettingsMidiMatchDialog::OnLogicalNameChanged(wxCommandEvent &event) {
  wxString errMsg;

  ValidateLogicalName(errMsg);
  m_StatusBar->SetStatusText(errMsg);
}

bool GOSettingsMidiMatchDialog::ValidateRegex(wxString &errMsg) {
  bool isValid = true;
  const wxString &newRegex = t_regex->GetValue();

  if (!newRegex.IsEmpty()) {
    wxRegEx regex(newRegex);

    if (!regex.IsValid()) {
      errMsg = _("Invalid regex");
      isValid = false;
    } else if (!regex.Matches(m_PhysicalName, 0)) {
      errMsg = _("The regex does not match the physical device name");
      isValid = false;
    }
  }
  if (isValid)
    errMsg = wxEmptyString;

  return isValid;
}

void GOSettingsMidiMatchDialog::OnRegexChanged(wxCommandEvent &event) {
  wxString errMsg;

  ValidateRegex(errMsg);
  m_StatusBar->SetStatusText(errMsg);
}

void GOSettingsMidiMatchDialog::OnHelp(wxCommandEvent &event) {
  GOHelpRequestor::DisplayHelp("MIDI Matching", IsModal());
}

bool GOSettingsMidiMatchDialog::Validate() {
  wxString errMsg;
  bool isValid = true;

  if (isValid)
    isValid = ValidateLogicalName(errMsg);
  if (isValid)
    isValid = ValidateRegex(errMsg);
  if (!isValid)
    wxMessageBox(
      errMsg, _("MIDI matching error"), wxOK | wxCENTRE | wxICON_ERROR);
  return isValid;
}

void GOSettingsMidiMatchDialog::SaveTo(GOMidiDeviceConfig &devConf) {
  devConf.m_LogicalName = t_LogicalName->GetValue();
  devConf.SetRegEx(t_regex->GetValue());
}
