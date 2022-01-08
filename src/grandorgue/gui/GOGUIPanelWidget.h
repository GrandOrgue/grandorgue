/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIPANELWIDGET_H
#define GOGUIPANELWIDGET_H

#include <wx/bitmap.h>
#include <wx/panel.h>

#include "GOBitmap.h"

class GOGUIPanel;

DECLARE_LOCAL_EVENT_TYPE(wxEVT_GOCONTROL, -1)

class GOGUIPanelWidget : public wxPanel {
 private:
  GOGUIPanel* m_panel;
  wxImage m_BGImage;
  bool m_BGInit;
  GOBitmap m_Background;
  wxBitmap m_ClientBitmap;
  double m_Scale;
  double m_FontScale;

  void initFont();
  void OnCreate(wxWindowCreateEvent& event);
  void OnDraw(wxDC* dc);
  void OnErase(wxEraseEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnGOControl(wxCommandEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnMouseLeftDown(wxMouseEvent& event);
  void OnMouseRightDown(wxMouseEvent& event);
  void OnMouseScroll(wxMouseEvent& event);
  bool ForwardMouseEvent(wxMouseEvent& event);
  void OnKeyCommand(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);

 public:
  GOGUIPanelWidget(GOGUIPanel* panel, wxWindow* parent,
                   wxWindowID id = wxID_ANY);
  ~GOGUIPanelWidget();

  void Focus();
  void OnUpdate();
  wxSize UpdateSize(wxSize size);

  DECLARE_EVENT_TABLE();
};

#endif
