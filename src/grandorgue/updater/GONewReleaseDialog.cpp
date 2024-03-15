/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GONewReleaseDialog.h"
#include "settings.h"

GONewReleaseDialog::GONewReleaseDialog(
  wxWindow *parent, GOConfig &config, GOReleaseMetadata release)
  : wxDialog(parent, wxID_ANY, _("New release available")), m_config(config) {
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  // Version name
  wxStaticText *versionLabel = new wxStaticText(
    this, wxID_ANY, wxString::Format(_("New version: %s"), release.version));
  sizer->Add(versionLabel, 0, wxALL, 5);

  // Changelog
  wxStaticText *changelogLabel
    = new wxStaticText(this, wxID_ANY, _("Changelog:"));
  sizer->Add(changelogLabel, 0, wxTOP | wxLEFT | wxRIGHT, 5);
  wxTextCtrl *changelogCtrl = new wxTextCtrl(
    this,
    wxID_ANY,
    release.changelog,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_MULTILINE | wxHSCROLL);
  changelogCtrl->SetEditable(false);
  changelogCtrl->SetSizeHints(600, 200);
  sizer->Add(changelogCtrl, 1, wxALL | wxEXPAND, 5);

  // Checkbox to disable updates at startup
  m_checkUpdatesAtStartupCtrl
    = new wxCheckBox(this, wxID_ANY, _("Check for updates at startup"));
  m_checkUpdatesAtStartupCtrl->SetValue(m_config.CheckForUpdatesAtStartup());
  sizer->Add(m_checkUpdatesAtStartupCtrl, 0, wxALL, 5);

  // Buttons
  wxStdDialogButtonSizer *buttonsSizer = new wxStdDialogButtonSizer();
  wxButton *okButton = new wxButton(this, wxID_EXECUTE, _("Download"));
  wxButton *cancelButton = new wxButton(this, wxID_OK);
  buttonsSizer->Add(okButton, 0, wxLEFT, 5);
  buttonsSizer->Add(cancelButton, 0, wxLEFT, 5);
  sizer->Add(buttonsSizer, 0, wxALIGN_RIGHT | wxALL, 5);

  // Compute layout & set dialog size
  SetSizerAndFit(sizer);

  // Bind event handlers
  Bind(
    wxEVT_BUTTON,
    &GONewReleaseDialog::HandleDownloadButton,
    this,
    wxID_EXECUTE);
  Bind(wxEVT_BUTTON, &GONewReleaseDialog::HandleOkButton, this, wxID_OK);
}

void GONewReleaseDialog::HandleDownloadButton(const wxCommandEvent &event) {
  wxLaunchDefaultBrowser(DOWNLOAD_URL);
}

void GONewReleaseDialog::HandleOkButton(const wxCommandEvent &event) {
  bool newValue = m_checkUpdatesAtStartupCtrl->GetValue();
  if (m_config.CheckForUpdatesAtStartup() != newValue) {
    // Save new config
    m_config.CheckForUpdatesAtStartup(newValue);
    m_config.Flush();
  }
  EndModal(0);
}
