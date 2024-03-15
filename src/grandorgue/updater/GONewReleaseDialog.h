/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GONEWRELEASEDIALOG_H
#define GONEWRELEASEDIALOG_H

#include "config/GOConfig.h"
#include "updater/GOUpdateChecker.h"
#include <wx/wx.h>

class GONewReleaseDialog : public wxDialog {
public:
  GONewReleaseDialog(
    wxWindow *parent, GOConfig &config, GOReleaseMetadata release);
  void HandleDownloadButton(const wxCommandEvent &event);
  void HandleOkButton(const wxCommandEvent &event);

private:
  wxCheckBox *m_checkUpdatesAtStartupCtrl;
  GOConfig &m_config;
};

#endif
