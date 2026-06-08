/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODocumentView.h"

#include "gui/dialogs/common/GODialogCloser.h"

#include "GODocumentBase.h"

GODocumentView::GODocumentView(GODocumentBase *doc, wxWindow *wnd)
  : m_doc(doc), m_wnd(wnd) {}

GODocumentView::~GODocumentView() {
  if (m_doc)
    m_doc->unregisterWindow(this);
  m_doc = NULL;
}

void GODocumentView::RemoveView() {
  m_doc = NULL;

  const bool isAutoDestroyed = dynamic_cast<GODialogCloser *>(m_wnd);

  m_wnd->Hide();
  if (!isAutoDestroyed)
    // GODialogCloser destroys itself on hide
    m_wnd->Destroy();
}

void GODocumentView::ShowView() {
  if (GODialogCloser *pDialog = dynamic_cast<GODialogCloser *>(m_wnd))
    pDialog->ShowAdvanced(true);
  else {
    m_wnd->Show();
    m_wnd->Raise();
  }
}

void GODocumentView::SyncState() {}