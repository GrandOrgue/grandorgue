/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
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
  GOGUIPanel *p_panel;
  wxImage m_BackgroundImage;
  GOBitmap m_Background;
  wxBitmap m_ClientBitmap;
  wxBitmap m_PreviewBitmap;
  bool m_HasPendingControlRedraw = false;
  double m_Scale;
  double m_LastRedrawScale;
  double m_FontScale;

  /** Rebuilds the control bitmaps after the last live resize event. */
  wxTimer m_ResizeTimer;

  /**
   * A point where the mouse has been pressed. Used for deduplication
   */
  wxPoint m_PressedPoint;

  void initFont();

  /** Computes and stores m_Scale so that the panel fits into `size`. */
  void ComputeScale(const wxSize &size);
  /** The panel's pixel size at the current m_Scale. */
  wxSize GetScaledPanelSize() const;
  /** Rebuilds the panel bitmap at the current scale and repaints. */
  void FullRedraw();

  void OnPaint(wxPaintEvent &event);
  void OnGOControl(wxCommandEvent &event);
  void OnResizeTimer(wxTimerEvent &event);

  /**
   * Stores m_PressedPoint and calls p_panel->HandleMousePress with relative
   * coords
   */
  void HandleMousePress(const wxPoint &point, bool isRight);
  /**
   * calls p_panel->HandleMouseRelease and clears m_PressedPoint
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
    GOGUIPanel *pPanel, wxWindow *pParent, wxWindowID id = wxID_ANY);
  ~GOGUIPanelWidget();

  void Focus();

  /**
   * Sets the panel's size immediately, with a full sharp redraw and no
   * live-drag preview/debounce. Intended for one-off sizing (e.g. when a
   * panel window is first opened), as opposed to UpdatePreviewSize() which
   * is for live window resizing. Returns the actual size of the scaled
   * panel.
   */
  wxSize SetInitialSize(const wxSize &size);
  /**
   * Handles a live resize tick: computes the new scale, shows a cheap
   * bilinear preview immediately, and (re)starts the debounce timer that
   * will trigger a full sharp redraw once resizing settles (see
   * OnResizeTimer()). Returns the actual size of the scaled panel.
   */
  wxSize UpdatePreviewSize(const wxSize &size);

  DECLARE_EVENT_TABLE();
};

#endif
