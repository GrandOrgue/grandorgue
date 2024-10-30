/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GONEWRELEASEDIALOG_H
#define GONEWRELEASEDIALOG_H

#include <wx/checkbox.h>
#include <wx/event.h>

#include "common/GOSimpleDialog.h"
#include "config/GOConfig.h"
#include "updater/GOUpdateChecker.h"

class GONewReleaseDialog : public GOSimpleDialog {
public:
  GONewReleaseDialog(
    wxWindow *parent,
    GOConfig &config,
    const GOUpdateChecker::ReleaseMetadata &release);
  void OnDownloadButton(wxCommandEvent &event);
  bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
private:
  wxCheckBox *m_checkUpdatesAtStartupCtrl;
  GOConfig &r_config;
};

#endif
