/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPANELVIEW_H
#define GOPANELVIEW_H

#include <wx/scrolwin.h>
#include <wx/toplevel.h>

#include "GOView.h"

class GOGUIControl;
class GOGUIPanel;
class GOGUIPanelWidget;

class GOPanelView : public wxScrolledWindow, public GOView {
private:
  GOGUIPanelWidget *m_panelwidget;
  GOGUIPanel *m_panel;
  wxTopLevelWindow
    *m_TopWindow; // only if the parent is top level window, else NULL
  wxSize m_Scroll;

  void OnSize(wxSizeEvent &event);

public:
  GOPanelView(GODocumentBase *doc, GOGUIPanel *panel, wxWindow *parent);
  ~GOPanelView();

  void AddEvent(GOGUIControl *control);

  // creates a PanelView and a wxFrame filled with this PanelView
  static GOPanelView *createWithFrame(GODocumentBase *doc, GOGUIPanel *panel);
  void SyncState();

  void Raise();
  bool Destroy();
  void RemoveView();

  DECLARE_EVENT_TABLE()
};

#endif
