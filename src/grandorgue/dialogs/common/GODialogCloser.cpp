/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODialogCloser.h"

GODialogCloser::GODialogCloser(wxDialog *pDlg) : p_dialog(pDlg) {
  pDlg->Bind(wxEVT_CLOSE_WINDOW, &GODialogCloser::OnCloseWindow, this);
  pDlg->Bind(wxEVT_SHOW, &GODialogCloser::OnShow, this);
}

bool GODialogCloser::ShowAdvanced(bool isAutoDestroyable) {
  m_AutoDestroyable = isAutoDestroyable;
  return p_dialog->Show(true);
}

int GODialogCloser::ShowModalAdvanced(bool isAutoDestroyable) {
  m_AutoDestroyable = isAutoDestroyable;
  return p_dialog->ShowModal();
}

void GODialogCloser::CloseAdvanced(int retCode) {
  if (p_dialog->IsModal())
    p_dialog->EndModal(retCode);
  else
    p_dialog->Show(false);
}

void GODialogCloser::OnCloseWindow(wxCloseEvent &event) {
  CloseAdvanced(wxID_CANCEL);
}

void GODialogCloser::OnShow(wxShowEvent &event) {
  if (event.IsShown())
    OnShow();
  else
    OnHide();
}

void GODialogCloser::OnShow() {}

void GODialogCloser::OnHide() {
  if (m_AutoDestroyable) {
    m_AutoDestroyable = false;
    p_dialog->Destroy();
  }
}
