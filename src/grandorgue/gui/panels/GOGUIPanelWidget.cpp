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
  GOGUIPanel *pPanel, wxWindow *pParent, wxWindowID id)
  : wxPanel(pParent, id),
    p_panel(pPanel),
    m_Scale(1),
    m_LastRedrawScale(1),
    m_FontScale(1),
    m_ResizeTimer(this, ID_PANEL_RESIZE_TIMER),
    m_PressedPoint(default_point) {
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  initFont();
  SetLabel(p_panel->GetName());

  wxBitmap background(p_panel->GetWidth(), p_panel->GetHeight());
  wxMemoryDC dc(background);
  GODC panelDC(&dc, m_Scale, m_FontScale);

  dc.SetBackground(*wxBLACK_BRUSH);
  dc.Clear();
  p_panel->DrawBackground(panelDC);
  dc.SelectObject(wxNullBitmap);
  m_BackgroundImage = background.ConvertToImage();
  m_Background.SetSourceImage(&m_BackgroundImage);
  m_Background.BuildScaledBitmap(1, wxRect(), nullptr);
  FullRedraw();
  SetCanFocus(p_panel->IsKeyboardInputUsed());
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
  const bool isKeyboardUsed = p_panel->IsKeyboardInputUsed();

  SetCanFocus(isKeyboardUsed);
  if (isKeyboardUsed && !HasFocus())
    SetFocus();
}

void GOGUIPanelWidget::ComputeScale(const wxSize &size) {
  double scaleX = size.GetWidth() / (double)p_panel->GetWidth();
  double scaleY = size.GetHeight() / (double)p_panel->GetHeight();
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
    p_panel->GetWidth() * m_Scale + 0.5, p_panel->GetHeight() * m_Scale + 0.5);
}

void GOGUIPanelWidget::FullRedraw() {
  const wxSize scaledSize = GetScaledPanelSize();

  p_panel->PrepareDraw(m_Scale, &m_Background);
  m_ClientBitmap = wxBitmap(scaledSize.GetWidth(), scaledSize.GetHeight());

  wxMemoryDC dc(m_ClientBitmap);
  GODC panelDC(&dc, m_Scale, m_FontScale);

  dc.SetBackground(*wxBLACK_BRUSH);
  dc.Clear();
  p_panel->Draw(panelDC);
  dc.SelectObject(wxNullBitmap);
  m_PreviewBitmap = wxNullBitmap;
  m_HasPendingControlRedraw = false;
  m_LastRedrawScale = m_Scale;
  SetSize(scaledSize);
  Refresh(false);
}

wxSize GOGUIPanelWidget::SetInitialSize(const wxSize &size) {
  m_ResizeTimer.Stop();
  ComputeScale(size);
  FullRedraw();
  return GetSize();
}

wxSize GOGUIPanelWidget::UpdatePreviewSize(const wxSize &size) {
  const double previousScale = m_Scale;

  ComputeScale(size);

  if (m_Scale != previousScale) {
    const wxSize scaledSize = GetScaledPanelSize();

    // Scale the last complete frame for immediate feedback during a drag.
    if (m_Scale == m_LastRedrawScale)
      m_PreviewBitmap = wxNullBitmap;
    else
      m_PreviewBitmap = wxBitmap(m_ClientBitmap.ConvertToImage().Scale(
        scaledSize.GetWidth(), scaledSize.GetHeight(), wxIMAGE_QUALITY_NORMAL));

    SetSize(scaledSize);
    Refresh(false);
  }

  if (m_PreviewBitmap.IsOk() || m_HasPendingControlRedraw)
    m_ResizeTimer.StartOnce(RESIZE_SETTLE_MS);
  else
    m_ResizeTimer.Stop();

  return GetSize();
}

void GOGUIPanelWidget::OnResizeTimer(wxTimerEvent &WXUNUSED(event)) {
  if (m_Scale != m_LastRedrawScale || m_HasPendingControlRedraw)
    FullRedraw();
}

void GOGUIPanelWidget::OnPaint(wxPaintEvent &WXUNUSED(event)) {
  wxPaintDC dc(this);

  dc.DrawBitmap(
    m_PreviewBitmap.IsOk() ? m_PreviewBitmap : m_ClientBitmap, 0, 0, false);
}

void GOGUIPanelWidget::OnGOControl(wxCommandEvent &event) {
  if (m_PreviewBitmap.IsOk()) {
    // The prepared controls still use the scale of the last complete frame.
    m_HasPendingControlRedraw = true;
  } else {
    auto &control = *static_cast<GOGUIControl *>(event.GetClientData());
    wxMemoryDC dc(m_ClientBitmap);
    GODC panelDC(&dc, m_LastRedrawScale, m_FontScale);

    control.Draw(panelDC);
    RefreshRect(panelDC.ScaleRect(control.GetBoundingRect()), false);
  }
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
  p_panel->HandleMousePress(point.x / m_Scale, point.y / m_Scale, isRight);
}

void GOGUIPanelWidget::HandleMouseRelease(bool isRight) {
  p_panel->HandleMouseRelease(isRight);
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
    p_panel->HandleMouseScroll(
      event.GetX() / m_Scale, event.GetY() / m_Scale, event.GetWheelRotation());
  event.Skip();
}

void GOGUIPanelWidget::OnKeyCommand(wxKeyEvent &event) {
  int k = event.GetKeyCode();
  if (!event.AltDown()) {
    if (!event.ShiftDown())
      p_panel->HandleKey(259); /* Disable setter */
    if (event.ShiftDown() || event.GetKeyCode() == WXK_SHIFT)
      p_panel->HandleKey(260); /* Enable setter */

    k = GOKeyConvert::wXKtoVK(k);
    if (k)
      p_panel->HandleKey(k);
  }
  event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
  event.Skip();
}

void GOGUIPanelWidget::OnKeyUp(wxKeyEvent &event) {
  int k = event.GetKeyCode();
  if (k == WXK_SHIFT)
    p_panel->HandleKey(259); /* Disable setter */
  event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
  event.Skip();
}
