/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODOCUMENTVIEW_H
#define GODOCUMENTVIEW_H

#include <wx/window.h>

class GODocumentBase;

class GODocumentView {
private:
  GODocumentBase *m_doc;
  wxWindow *m_wnd;

public:
  GODocumentView(GODocumentBase *doc, wxWindow *wnd);
  virtual ~GODocumentView();

  bool HasDocument() const { return m_doc != NULL; }

  GODocumentBase *GetDocument() const { return m_doc; }

  virtual void RemoveView();
  void ShowView();

  virtual void SyncState();
};

#endif