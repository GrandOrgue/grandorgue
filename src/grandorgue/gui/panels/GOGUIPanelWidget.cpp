/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

BEGIN_EVENT_TABLE(GOGUIPanelWidget, wxPanel)
EVT_ERASE_BACKGROUND(GOGUIPanelWidget::OnErase)
EVT_PAINT(GOGUIPanelWidget::OnPaint)
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
    m_Background(&m_BGImage),
    m_Scale(1),
    m_FontScale(1),
    m_PressedPoint(default_point) {
  initFont();
  SetLabel(m_panel->GetName());
  m_ClientBitmap.Create(
    m_panel->GetWidth() * m_Scale, m_panel->GetHeight() * m_Scale);
  m_panel->PrepareDraw(m_Scale, NULL);
  OnUpdate();
  m_BGImage = m_ClientBitmap.ConvertToImage();
  m_Background.PrepareBitmap(m_Scale, wxRect(0, 0, 0, 0), NULL);
  m_BGInit = true;
  SetCanFocus(true);
}

GOGUIPanelWidget::~GOGUIPanelWidget() {}

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
  if (!HasFocus())
    SetFocus();
}

wxSize GOGUIPanelWidget::UpdateSize(wxSize size) {
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
  m_panel->PrepareDraw(m_Scale, m_BGInit ? &m_Background : NULL);
  OnUpdate();
  Refresh();
  return GetSize();
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
  if (m_BGInit)
    m_ClientBitmap = (wxBitmap)m_BGImage.Scale(
      m_panel->GetWidth() * m_Scale + 0.5,
      m_panel->GetHeight() * m_Scale + 0.5,
      wxIMAGE_QUALITY_BICUBIC);
  else
    m_ClientBitmap = wxBitmap(
      m_panel->GetWidth() * m_Scale + 0.5,
      m_panel->GetHeight() * m_Scale + 0.5);
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

    k = WXKtoVK(k);
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
