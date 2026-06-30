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
#include <wx/timer.h>

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

  /**
   * The bitmap produced by the last full, sharp redraw (OnUpdate() right
   * after PrepareDraw()). UpdateSize() always rescales from this one for its
   * cheap live-drag preview rather than from m_ClientBitmap - which during a
   * drag may itself already be a rescaled preview - to avoid repeatedly
   * rescaling an already-rescaled (and thus progressively softer) image.
   */
  wxBitmap m_LastFullBitmap;
  double m_Scale;
  double m_FontScale;

  /**
   * Fires once resizing has been idle for a short while. UpdateSize() only
   * does a cheap rescale of m_LastFullBitmap on every WM_SIZE tick (a live
   * drag can fire dozens of those); the expensive, sharp re-render of every
   * control bitmap only runs once here, after the user stops moving the
   * border - otherwise every pixel of mouse movement triggered a full
   * BICUBIC rescale of every key/stop/texture, making resizing unusably
   * slow.
   */
  wxTimer m_ResizeTimer;

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
  void OnResizeTimer(wxTimerEvent &event);

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
