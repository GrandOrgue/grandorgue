/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsDeviceMatchDialog.h"

#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>

#include "config/GODeviceNamePattern.h"
#include "help/GOHelpRequestor.h"

BEGIN_EVENT_TABLE(GOSettingsDeviceMatchDialog, wxDialog)
EVT_TEXT(ID_LOGICAL_NAME, GOSettingsDeviceMatchDialog::OnLogicalNameChanged)
EVT_TEXT(ID_REGEX, GOSettingsDeviceMatchDialog::OnRegexChanged)
EVT_BUTTON(wxID_HELP, GOSettingsDeviceMatchDialog::OnHelp)
END_EVENT_TABLE()

GOSettingsDeviceMatchDialog::GOSettingsDeviceMatchDialog(
  wxWindow *parent, std::vector<const GODeviceNamePattern *> *otherDevices)
  : wxDialog(
    parent,
    wxID_ANY,
    _("Device matching properties"),
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
    CreateSeparatedButtonSizer(wxOK | wxCANCEL | wxHELP), 0, wxEXPAND, 5);

  m_StatusBar = new wxStatusBar(this);
  topSizer->Add(m_StatusBar, 0, wxEXPAND);

  SetSizer(topSizer);
  Fit();
}

void GOSettingsDeviceMatchDialog::FillWith(
  const GODeviceNamePattern &namePattern) {
  m_PhysicalName = namePattern.GetPhysicalName();
  t_PhysicalName->ChangeValue(namePattern.GetPhysicalName());
  t_LogicalName->ChangeValue(namePattern.GetLogicalName());
  t_regex->ChangeValue(namePattern.GetRegEx());
}

bool GOSettingsDeviceMatchDialog::ValidateLogicalName(wxString &errMsg) {
  bool isValid = true;
  const wxString &newLogicalName = t_LogicalName->GetValue();

  if (newLogicalName.IsEmpty()) {
    errMsg = _("Logical device name must not be empty");
    isValid = false;
  }
  if (isValid && p_OtherDevices)
    // Check logicalName for uniqueness
    for (const GODeviceNamePattern *pDev : *p_OtherDevices)
      if (
        pDev->GetPhysicalName() != m_PhysicalName
        && pDev->GetLogicalName() == newLogicalName) {
        errMsg = _("Logical device name is already used by ")
          + pDev->GetPhysicalName();
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

void GOSettingsDeviceMatchDialog::OnLogicalNameChanged(wxCommandEvent &event) {
  wxString errMsg;

  ValidateLogicalName(errMsg);
  m_StatusBar->SetStatusText(errMsg);
}

bool GOSettingsDeviceMatchDialog::ValidateRegex(wxString &errMsg) {
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

void GOSettingsDeviceMatchDialog::OnRegexChanged(wxCommandEvent &event) {
  wxString errMsg;

  ValidateRegex(errMsg);
  m_StatusBar->SetStatusText(errMsg);
}

void GOSettingsDeviceMatchDialog::OnHelp(wxCommandEvent &event) {
  GOHelpRequestor::DisplayHelp("Device Matching", IsModal());
}

bool GOSettingsDeviceMatchDialog::Validate() {
  wxString errMsg;
  bool isValid = true;

  if (isValid)
    isValid = ValidateLogicalName(errMsg);
  if (isValid)
    isValid = ValidateRegex(errMsg);
  if (!isValid)
    wxMessageBox(
      errMsg, _("Device matching error"), wxOK | wxCENTRE | wxICON_ERROR);
  return isValid;
}

void GOSettingsDeviceMatchDialog::SaveTo(GODeviceNamePattern &namePattern) {
  namePattern.SetLogicalName(t_LogicalName->GetValue());
  namePattern.SetRegEx(t_regex->GetValue());
}
