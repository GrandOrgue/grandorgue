/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSMIDIINITIAL_H
#define GOSETTINGSMIDIINITIAL_H

#include "gui/dialogs/common/GODialogTab.h"

class wxButton;
class wxGridEvent;

class GOConfig;
class GOGrid;
class GOMidi;

class GOSettingsMidiInitial : public GODialogTab {
  enum {
    ID_INITIALS,
    ID_PROPERTIES,
  };

private:
  GOConfig &r_config;
  GOMidi &r_midi;

  GOGrid *m_Initials;
  wxButton *m_Properties;

  void ApplyAdditionalSizes(const GOAdditionalSizeKeeper &sizeKeeper) override;
  void CaptureAdditionalSizes(
    GOAdditionalSizeKeeper &sizeKeeper) const override;

  bool TransferDataToWindow() override;

  void OnInitialsSelected(wxGridEvent &event);

  void ConfigureInitial();

  void OnInitialsDoubleClick(wxGridEvent &event) { ConfigureInitial(); }
  void OnProperties(wxCommandEvent &event) { ConfigureInitial(); }

public:
  GOSettingsMidiInitial(
    GOConfig &settings,
    GOMidi &midi,
    GOTabbedDialog *pDlg,
    const wxString &name,
    const wxString &label);

  DECLARE_EVENT_TABLE()
};

#endif // GOSETTINGSMIDIINITIAL_H
