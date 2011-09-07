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
#include "GOGUIDisplayMetrics.h"
#include "GOrgueEnclosure.h"
#include "GOrgueFrameGeneral.h"
#include "GOrgueLabel.h"
#include "GOrgueMeter.h"
#include "GOrgueMidi.h"
#include "GOrgueSound.h"
#include "GOrgueTremulant.h"
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
extern const unsigned char* ImageLoader_Wood[];
extern int c_ImageLoader_Wood[];

OrganPanel::OrganPanel(wxWindow* parent) :
	wxPanel(parent, wxID_ANY,wxDefaultPosition, wxDefaultSize,  wxTAB_TRAVERSAL, wxT("panel"))
{
}

void OrganPanel::TileWood(wxDC& dc, int which, int sx, int sy, int cx, int cy)
{
	int x, y;
	wxMemoryInputStream mem((const char*)ImageLoader_Wood[(which - 1) >> 1], c_ImageLoader_Wood[(which - 1) >> 1]);
	wxImage img(mem, wxBITMAP_TYPE_JPEG);
	if ((which - 1) & 1)
		img = img.Rotate90();
	wxBitmap bmp(img);
	dc.SetClippingRegion(sx, sy, cx, cy);
	for (y = sy & 0xFFFFFF00; y < sy + cy; y += 256)
		for (x = sx & 0xFFFFFF00; x < sx + cx; x += 256)
			dc.DrawBitmap(bmp, x, y, false);
	dc.DestroyClippingRegion();
}

void OrganPanel::OnUpdate(wxView *WXUNUSED(sender), wxObject *hint)
{

	int i, j;
	wxFont font = *wxNORMAL_FONT;

	m_clientOrigin = GetClientAreaOrigin();

	if (organfile)
		m_display_metrics = organfile->GetDisplayMetrics();

	m_clientBitmap = wxBitmap(m_display_metrics->GetScreenWidth(), m_display_metrics->GetScreenHeight());
	wxMemoryDC dc;
	dc.SelectObject(m_clientBitmap);

	TileWood(dc, m_display_metrics->GetDrawstopBackgroundImageNum(), 0, 0, m_display_metrics->GetCenterX(), m_display_metrics->GetScreenHeight());
	TileWood(dc, m_display_metrics->GetDrawstopBackgroundImageNum(), m_display_metrics->GetCenterX() + m_display_metrics->GetCenterWidth(), 0, m_display_metrics->GetScreenWidth() - (m_display_metrics->GetCenterX() + m_display_metrics->GetCenterWidth()), m_display_metrics->GetScreenHeight());
	TileWood(dc, m_display_metrics->GetConsoleBackgroundImageNum(), m_display_metrics->GetCenterX(), 0, m_display_metrics->GetCenterWidth(), m_display_metrics->GetScreenHeight());

	if (m_display_metrics->HasPairDrawstopCols())
	{
		for (i = 0; i < (m_display_metrics->NumberOfDrawstopColsToDisplay() >> 2); i++)
		{
			TileWood(dc,
				m_display_metrics->GetDrawstopInsetBackgroundImageNum(),
				i * 174 + m_display_metrics->GetJambLeftX() - 5,
				m_display_metrics->GetJambLeftRightY(),
				166,
				m_display_metrics->GetJambLeftRightHeight());
			TileWood(dc,
				m_display_metrics->GetDrawstopInsetBackgroundImageNum(),
				i * 174 + m_display_metrics->GetJambRightX() - 5,
				m_display_metrics->GetJambLeftRightY(),
				166,
				m_display_metrics->GetJambLeftRightHeight());
		}
	}

	if (m_display_metrics->HasTrimAboveExtraRows())
		TileWood(dc,
			m_display_metrics->GetKeyVertBackgroundImageNum(),
			m_display_metrics->GetCenterX(),
			m_display_metrics->GetCenterY(),
			m_display_metrics->GetCenterWidth(),
			8);

	if (m_display_metrics->GetJambTopHeight() + m_display_metrics->GetPistonTopHeight())
		TileWood(dc,
			m_display_metrics->GetKeyHorizBackgroundImageNum(),
			m_display_metrics->GetCenterX(),
			m_display_metrics->GetJambTopY(),
			m_display_metrics->GetCenterWidth(),
			m_display_metrics->GetJambTopHeight() + m_display_metrics->GetPistonTopHeight());

	for (unsigned i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
	{

		GOrgueManual* man = organfile->GetManual(i);
		if (man->IsDisplayed())
			man->Draw(dc);

	}

	j = (m_display_metrics->GetScreenWidth() - m_display_metrics->GetEnclosureWidth() + 6) >> 1;
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);
	for (unsigned l = 0; l < organfile->GetEnclosureCount(); l++)
		organfile->GetEnclosure(l)->DrawLabel(dc);

	DrawClickables(&dc);

	for (unsigned j = 0; j < organfile->GetLabelCount(); j++)
		organfile->GetLabel(j)->Draw(dc);

	dc.SelectObject(wxNullBitmap);

	GetParent()->SetClientSize(
		m_display_metrics->GetScreenWidth(),
		m_display_metrics->GetScreenHeight());

	SetSize(
		m_display_metrics->GetScreenWidth(),
		m_display_metrics->GetScreenHeight());

	GetParent()->Center(wxBOTH);
	GetParent()->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	GetParent()->Refresh();

}

void OrganPanel::OnErase(wxEraseEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !m_display_metrics->GetJambLeftRightWidth())
	{
		event.Skip();
		return;
	}
	wxDC* dc = event.GetDC();
	OnDraw(dc);
}
void OrganPanel::OnPaint(wxPaintEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !m_display_metrics->GetJambLeftRightWidth())
	{
		event.Skip();
		return;
	}
	wxPaintDC dc(this);
	OnDraw((wxDC*)&dc);
}

void OrganPanel::OnDrawstop(wxCommandEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !m_display_metrics->GetJambLeftRightWidth())
		return;

	wxMemoryDC mdc;
	mdc.SelectObject(m_clientBitmap);
	wxClientDC dc(this);
	dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

	static_cast<GOrgueDrawable*>(event.GetClientData())->Draw(0, 0, &mdc, &dc);
}

void OrganPanel::CopyToScreen(wxDC* mdc, const wxRect& rect)
{
	wxClientDC dc(this);
	dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);
	
	dc.Blit(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), mdc, rect.GetX(), rect.GetY());
}

void OrganPanel::OnGOControl(wxCommandEvent& event)
{
	if (!m_clientBitmap.Ok() || !organfile || !m_display_metrics->GetJambLeftRightWidth())
		return;

	GOGUIControl* control = static_cast<GOGUIControl*>(event.GetClientData());

	wxMemoryDC mdc;
	mdc.SelectObject(m_clientBitmap);
	
	control->Draw(&mdc);
	CopyToScreen(&mdc, control->GetBoundingRect());
}

void OrganPanel::OnNoteOnOff(wxCommandEvent& event)
{

	if (!m_clientBitmap.Ok() || !organfile || !m_display_metrics->GetJambLeftRightWidth())
		return;

	wxMemoryDC mdc;
	mdc.SelectObject(m_clientBitmap);
	wxClientDC dc(this);
	dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

	GOrgueManual* man = organfile->GetManual(event.GetInt());
	if (man->IsDisplayed())
	{
		man->DrawKey(mdc, event.GetExtraLong());
		man->DrawKey( dc, event.GetExtraLong());
	}

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
	{
		organfile->GetPanel(0)->HandleMousePress(xx, yy, right);

		for (unsigned i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
		{
			if (dc || !organfile->GetManual(i)->IsDisplayed())
				continue;

			const GOGUIDisplayMetrics::MANUAL_RENDER_INFO& mri = m_display_metrics->GetManualRenderInfo(i);

			wxRect rect
				(mri.x
				,mri.y
				,mri.width
				,mri.height
				);

			if (rect.Contains(xx, yy))
			{
				if (right)
					organfile->GetManual(i)->MIDI();
				else
					organfile->GetManual(i)->Click(xx, yy);
			}

		}
	}

	if (scroll)
		organfile->GetPanel(0)->HandleMouseScroll(xx, yy, scroll);

	for (unsigned l = 0; l < organfile->GetEnclosureCount(); l++)
	{
		if (organfile->GetEnclosure(l)->Draw(xx, yy, dc))
		{
			if (right)
				organfile->GetEnclosure(l)->MIDI();
			else if (scroll)
				organfile->GetEnclosure(l)->Scroll(scroll > 0);
		}
	}
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
				if (organfile && doc && doc->b_loaded && (k = WXKtoVK(k)))
					organfile->GetPanel(0)->HandleKey(k);
				event.Skip();
			}
		}
	}
	event.Skip();
}

void OrganPanel::OnDraw(wxDC* dc)
{
	if (!m_clientBitmap.Ok() || !organfile || !m_display_metrics->GetJambLeftRightWidth())
		return;
	dc->DrawBitmap(m_clientBitmap, m_clientOrigin.x, m_clientOrigin.y, false);
}
