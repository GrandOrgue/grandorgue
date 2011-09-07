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

#include "OrganPanel.h"

#include <algorithm>
#include <wx/event.h>
#include <wx/mstream.h>
#include "GOrgueFrameGeneral.h"
#include "GOrgueMeter.h"
#include "GOrgueMidi.h"
#include "GOrgueSound.h"
#include "GOGUIPanel.h"
#include "GOGUIControl.h"
#include "GrandOrgue.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueFrame.h"
#include "GrandOrgueID.h"
#include "KeyConvert.h"
#include "OrganDocument.h"

BEGIN_EVENT_TABLE(OrganPanel, wxPanel)
EVT_ERASE_BACKGROUND(OrganPanel::OnErase)
EVT_LEFT_DOWN(OrganPanel::OnMouseLeftDown)
EVT_LEFT_DCLICK(OrganPanel::OnMouseLeftDown)
EVT_RIGHT_DOWN(OrganPanel::OnMouseRightDown)
EVT_RIGHT_DCLICK(OrganPanel::OnMouseRightDown)
EVT_MOUSEWHEEL(OrganPanel::OnMouseScroll)
EVT_KEY_DOWN(OrganPanel::OnKeyCommand)
EVT_PAINT(OrganPanel::OnPaint)

END_EVENT_TABLE()

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;

OrganPanel::OrganPanel(wxWindow* parent) :
	wxPanel(parent, wxID_ANY,wxDefaultPosition, wxDefaultSize,  wxTAB_TRAVERSAL, wxT("panel"))
{
}

void OrganPanel::OnUpdate(wxView *WXUNUSED(sender), wxObject *hint)
{

	wxFont font = *wxNORMAL_FONT;

	m_clientOrigin = GetClientAreaOrigin();

	m_clientBitmap = wxBitmap(organfile->GetPanel(0)->GetWidth(), organfile->GetPanel(0)->GetHeight());
	wxMemoryDC dc;
	dc.SelectObject(m_clientBitmap);

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);

	DrawClickables(&dc);

	dc.SelectObject(wxNullBitmap);

	GetParent()->SetClientSize(
		organfile->GetPanel(0)->GetWidth(),
		organfile->GetPanel(0)->GetHeight());

	SetSize(
		organfile->GetPanel(0)->GetWidth(),
		organfile->GetPanel(0)->GetHeight());

	GetParent()->Center(wxBOTH);
	GetParent()->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	GetParent()->Refresh();

}

void OrganPanel::OnErase(wxEraseEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile)
	{
		event.Skip();
		return;
	}
	wxDC* dc = event.GetDC();
	OnDraw(dc);
}
void OrganPanel::OnPaint(wxPaintEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile)
	{
		event.Skip();
		return;
	}
	wxPaintDC dc(this);
	OnDraw((wxDC*)&dc);
}

void OrganPanel::CopyToScreen(wxDC* mdc, const wxRect& rect)
{
	wxClientDC dc(this);
	dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);
	
	dc.Blit(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), mdc, rect.GetX(), rect.GetY());
}

void OrganPanel::OnGOControl(wxCommandEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile)
		return;

	GOGUIControl* control = static_cast<GOGUIControl*>(event.GetClientData());

	wxMemoryDC mdc;
	mdc.SelectObject(m_clientBitmap);
	
	control->Draw(&mdc);
	CopyToScreen(&mdc, control->GetBoundingRect());
}

void OrganPanel::OnMouseLeftDown(wxMouseEvent& event)
{
	wxClientDC dc(this);
	DrawClickables(NULL, event.GetX(), event.GetY());
	event.Skip();
}

void OrganPanel::OnMouseRightDown(wxMouseEvent& event)
{
	wxClientDC dc(this);
	DrawClickables(NULL, event.GetX(), event.GetY(), true);
	event.Skip();
}

void OrganPanel::OnMouseScroll(wxMouseEvent& event)
{
	wxClientDC dc(this);
	DrawClickables(
		NULL,
		event.GetX() + m_clientOrigin.x,
		event.GetY() + m_clientOrigin.y,
		false,
		event.GetWheelRotation());
	event.Skip();
}


void OrganPanel::DrawClickables(wxDC* dc, int xx, int yy, bool right, int scroll)
{
	if (!m_clientBitmap.Ok())
		return;

	if (dc)
		organfile->GetPanel(0)->Draw(dc);

	if (!scroll)
		organfile->GetPanel(0)->HandleMousePress(xx, yy, right);

	if (scroll)
		organfile->GetPanel(0)->HandleMouseScroll(xx, yy, scroll);
}

void OrganPanel::OnKeyCommand(wxKeyEvent& event)
{
	if (g_sound && g_sound->GetMidi().SetterActive() ^ event.ShiftDown())
	{
		::wxGetApp().frame->ProcessCommand(ID_AUDIO_MEMSET);
		UpdateWindowUI();
	}

	int k = event.GetKeyCode();
	if ( !event.AltDown())
	{

		GOrgueMeter* meter = ::wxGetApp().frame->m_meters[2];
		OrganDocument* doc = (OrganDocument*)::wxGetApp().m_docManager->GetCurrentDocument();
		switch(k)
		{
			case WXK_ESCAPE:
			{
				::wxGetApp().frame->ProcessCommand(ID_AUDIO_PANIC);
				break;
			}
			case WXK_LEFT:
			{
				meter->SetValue(meter->GetValue() - 1);
				break;
			}
			case WXK_DOWN:
			{
				if (organfile)
					organfile->GetFrameGeneral(meter->GetValue() - 1)->Push();
				break;
			}
			case WXK_RIGHT:
			{
				meter->SetValue(meter->GetValue() + 1);
				break;
			}
			default:
			{
				if (organfile && doc && (k = WXKtoVK(k)))
					organfile->GetPanel(0)->HandleKey(k);
				event.Skip();
			}
		}
	}
	event.Skip();
}

void OrganPanel::OnDraw(wxDC* dc)
{
	if (!m_clientBitmap.Ok() || !organfile)
		return;
	dc->DrawBitmap(m_clientBitmap, m_clientOrigin.x, m_clientOrigin.y, false);
}
