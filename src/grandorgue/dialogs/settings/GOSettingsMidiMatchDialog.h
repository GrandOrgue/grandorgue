/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSMIDIMATCHDIALOG_H
#define GOSETTINGSMIDIMATCHDIALOG_H

#include <wx/dialog.h>

#include <vector>

#include "config/GOMidiDeviceConfig.h"

class wxCheckListBox;
class wxStatusBar;
class wxTextCtrl;

class GOSettingsMidiMatchDialog : public wxDialog {
private:
  enum { ID_LOGICAL_NAME = 200, ID_REGEX };

  // for checking for logical name uniqueness
  std::vector<GOMidiDeviceConfig *> *p_OtherDevices;

  wxString m_PhysicalName;

  wxTextCtrl *t_PhysicalName;
  wxTextCtrl *t_LogicalName;
  wxTextCtrl *t_regex;

  wxStatusBar *m_StatusBar;

  bool ValidateLogicalName(wxString &errMsg);
  bool ValidateRegex(wxString &errMsg);

  void OnLogicalNameChanged(wxCommandEvent &event);
  void OnRegexChanged(wxCommandEvent &event);
  void OnHelp(wxCommandEvent &event);

public:
  GOSettingsMidiMatchDialog(
    wxWindow *parent, std::vector<GOMidiDeviceConfig *> *otherDevices = NULL);

  void FillWith(const GOMidiDeviceConfig &devConf);

  virtual bool Validate();

  void SaveTo(GOMidiDeviceConfig &devConf);

  DECLARE_EVENT_TABLE()
};

#endif /* GOSETTINGSMIDIMATCHDIALOG_H */
