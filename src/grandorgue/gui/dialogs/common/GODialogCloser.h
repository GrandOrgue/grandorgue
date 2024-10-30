/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIALOGCLOSER_H
#define GODIALOGCLOSER_H

#include <wx/dialog.h>
#include <wx/event.h>

/*
 * A class that can do additional actions on hiding a dialog window
 * - Automatically EndModal()
 * - Automatically destroy
 */

class GODialogCloser {
private:
  wxDialog *p_dialog;

  bool m_AutoDestroyable = false;
  virtual void OnShow(wxShowEvent &event);

protected:
  /**
   * If the dialog is called modally then calls EndModal with the retcode
   * specified Otherwise simple closes the dialog window. It may cause
   * destroying the dialog
   */
  void CloseAdvanced(int retCode = wxID_CANCEL);

  virtual void OnCloseWindow(wxCloseEvent &event);
  virtual void OnShow();
  virtual void OnHide();

public:
  GODialogCloser(wxDialog *pDlg);

  virtual bool ShowAdvanced(bool isAutoDestroyable = true);

  virtual int ShowModalAdvanced(bool isAutoDestroyable = false);
};

#endif /* GODIALOGCLOSER_H */
