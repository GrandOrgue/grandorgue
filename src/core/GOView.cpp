/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOView.h"

#include "GODocumentBase.h"

GOView::GOView(GODocumentBase *doc, wxWindow *wnd) : m_doc(doc), m_wnd(wnd) {}

GOView::~GOView() {
  if (m_doc)
    m_doc->unregisterWindow(this);
  m_doc = NULL;
}

void GOView::RemoveView() {
  m_doc = NULL;
  m_wnd->Hide();
  m_wnd->Destroy();
}

void GOView::ShowView() {
  m_wnd->Show();
  m_wnd->Raise();
}

void GOView::SyncState() {}
