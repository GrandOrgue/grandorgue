/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIPANELWIDGET_H
#define GOGUIPANELWIDGET_H

#include <wx/bitmap.h>
#include <wx/panel.h>

#include "primitives/GOBitmap.h"

class GOGUIPanel;

DECLARE_LOCAL_EVENT_TYPE(wxEVT_GOCONTROL, -1)

class GOGUIPanelWidget : public wxPanel {
private:
  GOGUIPanel *m_panel;
  wxImage m_BGImage;
  bool m_BGInit;
  GOBitmap m_Background;
  wxBitmap m_ClientBitmap;
  double m_Scale;
  double m_FontScale;

  /**
   * A point where the mouse has been pressed. Used for deduplication
   */
  wxPoint m_PressedPoint;

  void initFont();
  void OnCreate(wxWindowCreateEvent &event);
  void OnDraw(wxDC *dc);
  void OnErase(wxEraseEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnGOControl(wxCommandEvent &event);

  /**
   * Stores m_PressedPoint and calls m_Panel->HandleMousePress with relative
   * coords
   */
  void HandleMousePress(const wxPoint &point, bool isRight);
  /**
   * calls m_Panel->HandleMouseRelease and clears m_PressedPoint
   */
  void HandleMouseRelease(bool isRight);

  void OnMouseMove(wxMouseEvent &event);
  void OnMouseLeftDown(wxMouseEvent &event);
  void OnMouseLeftUp(wxMouseEvent &event);
  void OnMouseLeftDclick(wxMouseEvent &event);
  void OnMouseRightDown(wxMouseEvent &event);
  void OnMouseScroll(wxMouseEvent &event);
  bool ForwardMouseEvent(wxMouseEvent &event);
  void OnKeyCommand(wxKeyEvent &event);
  void OnKeyUp(wxKeyEvent &event);

public:
  GOGUIPanelWidget(
    GOGUIPanel *panel, wxWindow *parent, wxWindowID id = wxID_ANY);
  ~GOGUIPanelWidget();

  void Focus();
  void OnUpdate();
  wxSize UpdateSize(wxSize size);

  DECLARE_EVENT_TABLE();
};

#endif
