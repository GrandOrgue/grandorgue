/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOGUIControl.h"
#include "GOGUIPanel.h"
#include "GOGUIPanelWidget.h"
#include "KeyConvert.h"

DEFINE_LOCAL_EVENT_TYPE(wxEVT_GOCONTROL)

BEGIN_EVENT_TABLE(GOGUIPanelWidget, wxPanel)
	EVT_ERASE_BACKGROUND(GOGUIPanelWidget::OnErase)
	EVT_PAINT(GOGUIPanelWidget::OnPaint)
	EVT_COMMAND(0, wxEVT_GOCONTROL, GOGUIPanelWidget::OnGOControl)
	EVT_LEFT_DOWN(GOGUIPanelWidget::OnMouseLeftDown)
	EVT_LEFT_DCLICK(GOGUIPanelWidget::OnMouseLeftDown)
	EVT_RIGHT_DOWN(GOGUIPanelWidget::OnMouseRightDown)
	EVT_RIGHT_DCLICK(GOGUIPanelWidget::OnMouseRightDown)
	EVT_MOUSEWHEEL(GOGUIPanelWidget::OnMouseScroll)
	EVT_KEY_DOWN(GOGUIPanelWidget::OnKeyCommand)
END_EVENT_TABLE()

GOGUIPanelWidget::GOGUIPanelWidget(GOGUIPanel* panel, wxWindow* parent, wxWindowID id) :
	wxPanel(parent, id),
	m_panel(panel)
{
	m_panel->SetWindow(this);
	SetLabel(m_panel->GetName());
	m_ClientBitmap.Create(m_panel->GetWidth(), m_panel->GetHeight());
	OnUpdate();
	SetFocus();
}

GOGUIPanelWidget::~GOGUIPanelWidget()
{
	m_panel->SetWindow(NULL);
}

void GOGUIPanelWidget::OnDraw(wxDC* dc)
{
	dc->DrawBitmap(m_ClientBitmap, 0, 0, false);
}

void GOGUIPanelWidget::OnErase(wxEraseEvent& event)
{
	wxDC* dc = event.GetDC();
	OnDraw(dc);
}

void GOGUIPanelWidget::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	OnDraw(&dc);
}

void GOGUIPanelWidget::OnUpdate()
{
	m_ClientBitmap = wxBitmap(m_panel->GetWidth(), m_panel->GetHeight());
	wxMemoryDC dc;
	dc.SelectObject(m_ClientBitmap);

	m_panel->Draw(&dc);
	SetSize(m_panel->GetWidth(), m_panel->GetHeight());
}

void GOGUIPanelWidget::CopyToScreen(wxDC* mdc, const wxRect& rect)
{
	wxClientDC dc(this);
	
	dc.Blit(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), mdc, rect.GetX(), rect.GetY());
}

void GOGUIPanelWidget::OnGOControl(wxCommandEvent& event)
{
	GOGUIControl* control = static_cast<GOGUIControl*>(event.GetClientData());

	wxMemoryDC mdc;
	mdc.SelectObject(m_ClientBitmap);

	control->Draw(&mdc);
	CopyToScreen(&mdc, control->GetBoundingRect());
	event.Skip();
}

void GOGUIPanelWidget::OnMouseLeftDown(wxMouseEvent& event)
{
	m_panel->HandleMousePress(event.GetX(), event.GetY(), false);
	event.Skip();
}

void GOGUIPanelWidget::OnMouseRightDown(wxMouseEvent& event)
{
	m_panel->HandleMousePress(event.GetX(), event.GetY(), true);
	event.Skip();
}

void GOGUIPanelWidget::OnMouseScroll(wxMouseEvent& event)
{
	if (event.GetWheelRotation())
		m_panel->HandleMouseScroll(event.GetX(), event.GetY(), event.GetWheelRotation());
	event.Skip();
}

void GOGUIPanelWidget::OnKeyCommand(wxKeyEvent& event)
{
	int k = event.GetKeyCode();
	if ( !event.AltDown())
	{
		k = WXKtoVK(k);
		m_panel->HandleKey(k);
	}
	event.ResumePropagation(wxEVENT_PROPAGATE_MAX);
	event.Skip();
}


