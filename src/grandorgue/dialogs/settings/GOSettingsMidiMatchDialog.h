/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSDEVICEMATCHDIALOG_H
#define GOSETTINGSDEVICEMATCHDIALOG_H

#include <wx/dialog.h>

#include <vector>

class wxCheckListBox;
class wxStatusBar;
class wxTextCtrl;
class GODeviceNamePattern;

class GOSettingsDeviceMatchDialog : public wxDialog {
private:
  enum { ID_LOGICAL_NAME = 200, ID_REGEX };

  // for checking for logical name uniqueness
  std::vector<const GODeviceNamePattern *> *p_OtherDevices;

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
  GOSettingsDeviceMatchDialog(
    wxWindow *parent,
    std::vector<const GODeviceNamePattern *> *otherDevices = NULL);

  void FillWith(const GODeviceNamePattern &devNamePattern);

  virtual bool Validate();

  void SaveTo(GODeviceNamePattern &devNamePattern);

  DECLARE_EVENT_TABLE()
};

#endif /* GOSETTINGSDEVICEMATCHDIALOG_H */
