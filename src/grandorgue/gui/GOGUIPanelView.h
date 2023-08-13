/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIPANELVIEW_H
#define GOGUIPANELVIEW_H

#include <wx/scrolwin.h>
#include <wx/toplevel.h>

#include "document-base/GOView.h"

class GOGUIControl;
class GOGUIPanel;
class GOGUIPanelWidget;

class GOGUIPanelView : public wxScrolledWindow, public GOView {
private:
  GOGUIPanelWidget *m_panelwidget;
  GOGUIPanel *m_panel;
  wxTopLevelWindow *m_TopWindow;
  wxSize m_Scroll;

  void OnSize(wxSizeEvent &event);

public:
  GOGUIPanelView(
    GODocumentBase *doc, GOGUIPanel *panel, wxTopLevelWindow *parent);
  ~GOGUIPanelView();

  void AddEvent(GOGUIControl *control);

  // creates a PanelView and a wxFrame filled with this PanelView
  static GOGUIPanelView *createWithFrame(
    GODocumentBase *doc, GOGUIPanel *panel);
  void SyncState();

  void Raise();
  bool Destroy();
  void RemoveView();

  DECLARE_EVENT_TABLE()
};

#endif
