/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <wx/event.h>

#include "GONewReleaseDialog.h"
#include "go_ids.h"

BEGIN_EVENT_TABLE(GONewReleaseDialog, GOSimpleDialog)
EVT_BUTTON(ID_DOWNLOAD_NEW_RELEASE, GONewReleaseDialog::OnDownloadButton)
END_EVENT_TABLE()

GONewReleaseDialog::GONewReleaseDialog(
  wxWindow *parent,
  GOConfig &config,
  const GOUpdateChecker::ReleaseMetadata &release)
  : GOSimpleDialog(
    parent,
    "NewRelease",
    _("New release available"),
    config.m_DialogSizes,
    wxEmptyString,
    0,
    wxCLOSE),
    r_config(config) {
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
  m_checkUpdatesAtStartupCtrl->SetValue(r_config.CheckForUpdatesAtStartup());
  sizer->Add(m_checkUpdatesAtStartupCtrl, 0, wxALL, 5);

  // Add Download button that opens the browser
  wxSizer *const pButtonSizer = GetButtonSizer();
  if (pButtonSizer) {
    wxButton *downloadBtn
      = new wxButton(this, ID_DOWNLOAD_NEW_RELEASE, _("Download"));
    pButtonSizer->Insert(
      2, downloadBtn, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 10);
  }

  // Compute layout & set dialog size
  LayoutWithInnerSizer(sizer);
}

void GONewReleaseDialog::OnDownloadButton(wxCommandEvent &event) {
  GOUpdateChecker::OpenDownloadPageInBrowser();
}

bool GONewReleaseDialog::TransferDataFromWindow() {
  bool newValue = m_checkUpdatesAtStartupCtrl->GetValue();
  r_config.CheckForUpdatesAtStartup(newValue);
  return true;
}
