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
  GOGUIPanel *m_panel;
  wxImage m_BGImage;
  bool m_BGInit;
  GOBitmap m_Background;
  wxBitmap m_ClientBitmap;

  /**
   * Snapshot of m_ClientBitmap taken lazily at the start of a resize series
   * (the first UpdatePreviewSize() call after the last full redraw).
   * UpdatePreviewSize() always rescales from this one for its cheap
   * live-drag preview rather than from m_ClientBitmap - which during a drag
   * is itself already a rescaled preview - to avoid repeatedly rescaling an
   * already-rescaled (and thus progressively softer) image. Capturing
   * lazily (instead of maintaining a copy after every draw) also keeps
   * single-control redraws (OnGOControl) visible in the preview: whatever
   * m_ClientBitmap shows when the drag starts is what gets rescaled.
   */
  wxBitmap m_PreResizeBitmap;

  /**
   * True from the first UpdatePreviewSize() call of a resize series until
   * the debounced full redraw in OnResizeTimer(). Controls when
   * m_PreResizeBitmap is (re)captured.
   */
  bool m_ResizeInProgress;

  double m_Scale;

  /**
   * The scale of the last full, sharp redraw (constructor, SetInitialSize()
   * or OnResizeTimer()). Lets OnResizeTimer() skip the expensive redraw
   * when a resize series ends at the very scale that is already rendered -
   * without this, size events delivered while a long redraw was running
   * re-armed the timer and caused identical back-to-back redraws.
   */
  double m_LastRedrawScale;

  double m_FontScale;

  /**
   * Fires once resizing has been idle for a short while.
   * UpdatePreviewSize() only does a cheap rescale of m_PreResizeBitmap on
   * every WM_SIZE tick (a live drag can fire dozens of those); the
   * expensive, sharp re-render of every control bitmap only runs once here,
   * after the user stops moving the border - otherwise every pixel of mouse
   * movement triggered a full BICUBIC rescale of every key/stop/texture,
   * making resizing unusably slow.
   */
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
