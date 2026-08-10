/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIPanelWidget.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>

#include "primitives/GODC.h"
#include "primitives/GOFont.h"

#include "GOGUIControl.h"
#include "GOGUIPanel.h"
#include "GOKeyConvert.h"

DEFINE_LOCAL_EVENT_TYPE(wxEVT_GOCONTROL)

static constexpr int ID_PANEL_RESIZE_TIMER = wxID_HIGHEST + 1;
static constexpr unsigned RESIZE_SETTLE_MS = 150;

BEGIN_EVENT_TABLE(GOGUIPanelWidget, wxPanel)
EVT_ERASE_BACKGROUND(GOGUIPanelWidget::OnErase)
EVT_PAINT(GOGUIPanelWidget::OnPaint)
EVT_TIMER(ID_PANEL_RESIZE_TIMER, GOGUIPanelWidget::OnResizeTimer)
EVT_COMMAND(0, wxEVT_GOCONTROL, GOGUIPanelWidget::OnGOControl)
EVT_MOTION(GOGUIPanelWidget::OnMouseMove)
EVT_LEFT_DOWN(GOGUIPanelWidget::OnMouseLeftDown)
EVT_LEFT_DCLICK(GOGUIPanelWidget::OnMouseLeftDclick)
EVT_LEFT_UP(GOGUIPanelWidget::OnMouseLeftUp)
EVT_RIGHT_DOWN(GOGUIPanelWidget::OnMouseRightDown)
EVT_RIGHT_DCLICK(GOGUIPanelWidget::OnMouseRightDown)
EVT_MOUSEWHEEL(GOGUIPanelWidget::OnMouseScroll)
EVT_KEY_DOWN(GOGUIPanelWidget::OnKeyCommand)
EVT_KEY_UP(GOGUIPanelWidget::OnKeyUp)
END_EVENT_TABLE()

const wxPoint default_point(wxDefaultCoord, wxDefaultCoord);

GOGUIPanelWidget::GOGUIPanelWidget(
  GOGUIPanel *panel, wxWindow *parent, wxWindowID id)
  : wxPanel(parent, id),
    m_panel(panel),
    m_BGInit(false),
    m_ResizeInProgress(false),
    m_Scale(1),
    m_LastRedrawScale(1),
    m_FontScale(1),
    m_ResizeTimer(this, ID_PANEL_RESIZE_TIMER),
    m_PressedPoint(default_point) {
  m_Background.SetSourceImage(&m_BGImage);
  initFont();
  SetLabel(m_panel->GetName());
  m_ClientBitmap.Create(
    m_panel->GetWidth() * m_Scale, m_panel->GetHeight() * m_Scale);
  m_panel->PrepareDraw(m_Scale, NULL);
  OnUpdate();
  m_BGImage = m_ClientBitmap.ConvertToImage();
  m_Background.BuildScaledBitmap(m_Scale, wxRect(0, 0, 0, 0), NULL);
  m_BGInit = true;
  SetCanFocus(m_panel->IsKeyboardInputUsed());
}

GOGUIPanelWidget::~GOGUIPanelWidget() {
  /* Relies on wx discarding any wxTimerEvent already queued for this timer
  at the moment of Stop()/destruction, on all supported backends. */
  m_ResizeTimer.Stop();
}

void GOGUIPanelWidget::initFont() {
  wxMemoryDC dc;
  GOFont font;
  wxCoord cx, cy;
  font.SetPoints(39);
  dc.SetFont(font.GetFont(1));
  dc.GetTextExtent(wxT("M"), &cx, &cy);
  m_FontScale = 62.0 / cy;
}

void GOGUIPanelWidget::Focus() {
  const bool isKeyboardUsed = m_panel->IsKeyboardInputUsed();

  SetCanFocus(isKeyboardUsed);
  if (isKeyboardUsed && !HasFocus())
    SetFocus();
}

void GOGUIPanelWidget::ComputeScale(const wxSize &size) {
  double scaleX = size.GetWidth() / (double)m_panel->GetWidth();
  double scaleY = size.GetHeight() / (double)m_panel->GetHeight();
  if (scaleX < scaleY) // To fit all, we need to use the SMALLEST of the two
    m_Scale = scaleX;
  else
    m_Scale = scaleY;
  if (m_Scale > 2) // Actually useless, because max frame size is set to
                   // initial size
    m_Scale = 2;
  if (m_Scale < 0.25) // Let's not make the window content TOO small. There
                      // will be scrollbars if the user sizes to smaller than
                      // this. Without this limit, sizing too a too small value
                      // causes a crash!
    m_Scale = 0.25;
}

wxSize GOGUIPanelWidget::GetScaledPanelSize() const {
  return wxSize(
    m_panel->GetWidth() * m_Scale + 0.5, m_panel->GetHeight() * m_Scale + 0.5);
}

void GOGUIPanelWidget::FullRedraw() {
  m_panel->PrepareDraw(m_Scale, m_BGInit ? &m_Background : NULL);
  OnUpdate();
  m_LastRedrawScale = m_Scale;
  Refresh();
}

wxSize GOGUIPanelWidget::SetInitialSize(const wxSize &size) {
  ComputeScale(size);
  FullRedraw();
  return GetSize();
}

wxSize GOGUIPanelWidget::UpdatePreviewSize(const wxSize &size) {
  ComputeScale(size);

  const wxSize scaledSize = GetScaledPanelSize();

  // Cheap immediate feedback for a live drag: rescale the last sharply
  // redrawn bitmap instead of re-running PrepareDraw()/OnUpdate(), which
  // rescale every key/stop/texture with BICUBIC quality and made dragging a
  // panel border unusably slow when done on every single WM_SIZE tick. The
  // sharp, full-quality redraw happens once in OnResizeTimer(), after
  // resizing has been idle for a moment.
  //
  // The snapshot is captured lazily at the start of each resize series so
  // that it reflects whatever m_ClientBitmap shows right now - including
  // single-control redraws (OnGOControl) done since the last full redraw.
  // Always rescale from that snapshot, never from m_ClientBitmap: during a
  // drag m_ClientBitmap is itself the result of this same preview rescale,
  // so chaining off it would compound quality loss tick after tick instead
  // of staying as sharp as a single rescale from the original allows.
  //
  // wxDC::StretchBlit was tried here first, but on wxMSW it maps to GDI
  // StretchBlt in COLORONCOLOR (nearest-neighbour) mode, which duplicates or
  // drops whole rows/columns at non-integer scale factors instead of
  // interpolating - visibly blocky. wxImage::Scale() with NORMAL (bilinear)
  // quality looks smooth and is still cheap here because it runs once on the
  // already-composed panel bitmap, not once per control like BICUBIC mode
  // does in OnUpdate()/PrepareDraw().
  if (!m_ResizeInProgress) {
    m_ResizeInProgress = true;
    m_PreResizeBitmap = m_ClientBitmap;
  }
  if (
    m_PreResizeBitmap.IsOk() && scaledSize.GetWidth() > 0
    && scaledSize.GetHeight() > 0)
    m_ClientBitmap = (wxBitmap)m_PreResizeBitmap.ConvertToImage().Scale(
      scaledSize.GetWidth(), scaledSize.GetHeight(), wxIMAGE_QUALITY_NORMAL);
  SetSize(scaledSize.GetWidth(), scaledSize.GetHeight());
  Refresh();
  m_ResizeTimer.StartOnce(RESIZE_SETTLE_MS);
  return GetSize();
}

void GOGUIPanelWidget::OnResizeTimer(wxTimerEvent &WXUNUSED(event)) {
  m_ResizeInProgress = false;

  /* Skip the redraw when the series ended at the very scale that is
   * already sharply rendered (the user dragged out and back, or size
   * events delivered while a long redraw was running re-armed the timer).
   * m_ClientBitmap already shows the 1:1 preview of the sharp bitmap plus
   * any control redraws done on top of it, so there is nothing a full
   * redraw could improve. Without this check a slow redraw ran up to three
   * times back to back for identical output. */
  if (m_Scale != m_LastRedrawScale)
    FullRedraw();
}

void GOGUIPanelWidget::OnDraw(wxDC *dc) {
  dc->DrawBitmap(m_ClientBitmap, 0, 0, false);
}

void GOGUIPanelWidget::OnErase(wxEraseEvent &event) {
  wxDC *dc = event.GetDC();
  OnDraw(dc);
}

void GOGUIPanelWidget::OnPaint(wxPaintEvent &event) {
  wxPaintDC dc(this);
  OnDraw(&dc);
}

void GOGUIPanelWidget::OnUpdate() {
  const wxSize scaledSize = GetScaledPanelSize();

  if (m_BGInit)
    m_ClientBitmap = (wxBitmap)m_BGImage.Scale(
      scaledSize.GetWidth(), scaledSize.GetHeight(), wxIMAGE_QUALITY_BICUBIC);
  else
    m_ClientBitmap = wxBitmap(scaledSize.GetWidth(), scaledSize.GetHeight());
  wxMemoryDC dc;
  dc.SelectObject(m_ClientBitmap);
  GODC DC(&dc, m_Scale, m_FontScale);

  m_panel->Draw(DC);
  SetSize(m_ClientBitmap.GetWidth(), m_ClientBitmap.GetHeight());
}

void GOGUIPanelWidget::OnGOControl(wxCommandEvent &event) {
  GOGUIControl *control = static_cast<GOGUIControl *>(event.GetClientData());

  wxMemoryDC mdc;
  mdc.SelectObject(m_ClientBitmap);
  GODC DC(&mdc, m_Scale, m_FontScale);

  control->Draw(DC);
  RefreshRect(DC.ScaleRect(control->GetBoundingRect()), false);
  event.Skip();
}

bool GOGUIPanelWidget::ForwardMouseEvent(wxMouseEvent &event) {
  if (GetClientRect().Contains(event.GetPosition()))
    return false;
  wxPoint pos = ClientToScreen(event.GetPosition());
  wxWindow *window = wxFindWindowAtPoint(pos);
  if (window) {
    wxMouseEvent e = event;
    e.SetPosition(window->ScreenToClient(pos));
    window->HandleWindowEvent(e);
  }
  return true;
}

void GOGUIPanelWidget::HandleMousePress(const wxPoint &point, bool isRight) {
  if (!isRight)
    m_PressedPoint = point;
  m_panel->HandleMousePress(point.x / m_Scale, point.y / m_Scale, isRight);
}

void GOGUIPanelWidget::HandleMouseRelease(bool isRight) {
  m_panel->HandleMouseRelease(isRight);
  if (!isRight)
    m_PressedPoint = default_point;
}

void GOGUIPanelWidget::OnMouseMove(wxMouseEvent &event) {
  if (!event.LeftIsDown()) {
    HandleMouseRelease(false);
    return;
  }
  if (ForwardMouseEvent(event))
    return;

  wxPoint point = event.GetPosition();

  // some windows managers call several times at the same position. Skip if so
  if (point != m_PressedPoint)
    HandleMousePress(point, false);
  event.Skip();
}

void GOGUIPanelWidget::OnMouseLeftDown(wxMouseEvent &event) {
  Focus();
  if (ForwardMouseEvent(event))
    return;

  wxPoint point = event.GetPosition();

  // some windows managers call several times at the same position. Skip if so
  if (point != m_PressedPoint) {
    HandleMouseRelease(false);
    HandleMousePress(point, false);
  }
  event.Skip();
}

void GOGUIPanelWidget::OnMouseLeftDclick(wxMouseEvent &event) {
  if (!ForwardMouseEvent(event)) {
    HandleMouseRelease(false);
    HandleMousePress(event.GetPosition(), false);
    event.Skip();
  }
}

void GOGUIPanelWidget::OnMouseRightDown(wxMouseEvent &event) {
  Focus();

  HandleMousePress(event.GetPosition(), true);
  event.Skip();
}

void GOGUIPanelWidget::OnMouseLeftUp(wxMouseEvent &event) {
  if (m_PressedPoint.IsFullySpecified() && !event.LeftIsDown()) {
    HandleMouseRelease(false);
    event.Skip();
  }
}

void GOGUIPanelWidget::OnMouseScroll(wxMouseEvent &event) {
  if (event.GetWheelRotation())
    m_panel->HandleMouseScroll(
      event.GetX() / m_Scale, event.GetY() / m_Scale, event.GetWheelRotation());
  event.Skip();
}

void GOGUIPanelWidget::OnKeyCommand(wxKeyEvent &event) {
  int k = event.GetKeyCode();
  if (!event.AltDown()) {
    if (!event.ShiftDown())
      m_panel->HandleKey(259); /* Disable setter */
    if (event.ShiftDown() || event.GetKeyCode() == WXK_SHIFT)
      m_panel->HandleKey(260); /* Enable setter */

    k = GOKeyConvert::wXKtoVK(k);
    if (k)
      m_panel->HandleKey(k);
  }
  event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
  event.Skip();
}

void GOGUIPanelWidget::OnKeyUp(wxKeyEvent &event) {
  int k = event.GetKeyCode();
  if (k == WXK_SHIFT)
    m_panel->HandleKey(259); /* Disable setter */
  event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
  event.Skip();
}
