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
#include "GOrgueCoupler.h"
#include "GOrgueDrawStop.h"
#include "GOrgueDisplayMetrics.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueDrawStop.h"
#include "GOrgueEnclosure.h"
#include "GOrgueGeneral.h"
#include "GOrgueLabel.h"
#include "GOrgueMeter.h"
#include "GOrgueMidi.h"
#include "GOrguePiston.h"
#include "GOrguePushbutton.h"
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
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

	for (i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
	{

		GOrgueManual* man = organfile->GetManual(i);
		if (man->IsDisplayed())
			man->Draw(dc);

	}

	for (j = 0; j < organfile->GetDivisionalCouplerCount(); j++)
	{
		if (organfile->GetDivisionalCoupler(j)->Displayed)
		{
			font.SetPointSize(organfile->GetDivisionalCoupler(j)->DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->GetDivisionalCoupler(j)->Name, 51);
		}
	}

	for (unsigned j = 0; j < organfile->GetTremulantCount(); j++)
	{
		if (organfile->GetTremulant(j)->Displayed)
		{
			font.SetPointSize(organfile->GetTremulant(j)->DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->GetTremulant(j)->Name, 51);
		}
	}

	for (j = 0; j < organfile->GetGeneralCount(); j++)
	{
		if (organfile->GetGeneral(j)->Displayed)
		{
			font.SetPointSize(organfile->GetGeneral(j)->DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->GetGeneral(j)->Name, 28);
		}
	}

	for (j = 0; j < organfile->GetNumberOfReversiblePistons(); j++)
	{
		if (organfile->GetPiston(j)->Displayed)
		{
			font.SetPointSize(organfile->GetPiston(j)->DispLabelFontSize);
			dc.SetFont(font);
			WrapText(dc, organfile->GetPiston(j)->Name, 28);
		}
	}

	j = (m_display_metrics->GetScreenWidth() - m_display_metrics->GetEnclosureWidth() + 6) >> 1;
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);
	for (unsigned l = 0; l < organfile->GetEnclosureCount(); l++)
		organfile->GetEnclosure(l)->DrawLabel(dc);

	DrawClickables(&dc);

	for (j = 0; j < organfile->GetLabelCount(); j++)
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


void OrganPanel::HelpDrawStop(GOrgueDrawstop* stop, wxDC* dc, int xx, int yy, bool right)
{
	if (stop->Draw(xx, yy, dc))
	{
		if (right)
			stop->MIDI();
		else
		{
			stop->Push();
			wxMemoryDC mdc;
			mdc.SelectObject(m_clientBitmap);
			wxClientDC dc(this);
			dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

			stop->Draw(0, 0, &mdc, &dc);
		}
    }
}

void OrganPanel::HelpDrawButton(GOrguePushbutton* button, wxDC* dc, int xx, int yy, bool right)
{
	if (button->Draw(xx, yy, dc))
	{
		if (right)
			button->MIDI();
		else
		{
			button->Push();
			wxMemoryDC mdc;
			mdc.SelectObject(m_clientBitmap);
			wxClientDC dc(this);
			dc.SetDeviceOrigin(m_clientOrigin.x, m_clientOrigin.y);

			button->Draw(0, 0, &mdc, &dc);
		}
	}
}

void OrganPanel::DrawClickables(wxDC* dc, int xx, int yy, bool right, int scroll)
{
	int i, j;
	if (!m_clientBitmap.Ok())
		return;

	if (!scroll)
	{
		for (i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
		{
			for (j = 0; j < organfile->GetManual(i)->GetStopCount(); j++)
				HelpDrawStop(organfile->GetManual(i)->GetStop(j), dc, xx, yy, right);
			for (j = 0; j < organfile->GetManual(i)->GetCouplerCount(); j++)
				HelpDrawStop(organfile->GetManual(i)->GetCoupler(j), dc, xx, yy, right);
			for (j = 0; j < organfile->GetManual(i)->GetDivisionalCount(); j++)
				HelpDrawButton(organfile->GetManual(i)->GetDivisional(j), dc, xx, yy, right);
			if (dc || !organfile->GetManual(i)->IsDisplayed())
				continue;

			const GOrgueDisplayMetrics::MANUAL_RENDER_INFO& mri = m_display_metrics->GetManualRenderInfo(i);

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
			}

		}

		for (unsigned k = 0; k < organfile->GetTremulantCount(); k++)
			HelpDrawStop(organfile->GetTremulant(k), dc, xx, yy, right);
		for (j = 0; j < organfile->GetDivisionalCouplerCount(); j++)
			HelpDrawStop(organfile->GetDivisionalCoupler(j), dc, xx, yy, right);
		for (j = 0; j < organfile->GetGeneralCount(); j++)
			HelpDrawButton(organfile->GetGeneral(j), dc, xx, yy, right);
		for (j = 0; j < organfile->GetNumberOfReversiblePistons(); j++)
			HelpDrawButton(organfile->GetPiston(j), dc, xx, yy, right);
	}

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

void OrganPanel::WrapText(wxDC& dc, wxString& string, int width)
{
	wxString str, line, work;
	wxCoord cx, cy;

	/* string.Length() + 1 iterations */
	for(unsigned i = 0; i <= string.Length(); i++)
	{
		bool maybreak = false;
		if (string[i] == wxT(' ') || string[i] == wxT('\n'))
		{
			if (work.length() < 2)
				maybreak = false;
			else
				maybreak = true;
		}
		if (maybreak || i == string.Length())
		{
			if (!work.Length())
				continue;
			dc.GetTextExtent(line + wxT(' ') + work, &cx, &cy);
			if (cx > width)
			{
				if (!str.Length())
					str = line;
				else
					str = str + wxT('\n') + line;
				line = wxT("");
			}

			if (!line.Length())
				line = work;
			else
				line = line + wxT(' ') + work;

			work = wxT("");
		}
		else
		{
			if (string[i] == wxT(' ') || string[i] == wxT('\n'))
			{
				if (work.Length() && work[work.Length()-1] != wxT(' '))
					work += wxT(' ');
			}	
			else
				work += string[i];
		}
	}
	if (!str.Length())
		str = line;
	else
		str = str + wxT('\n') + line;
	string = str;
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
				{
					for (int i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
					{
						for (int j = 0; j < organfile->GetManual(i)->GetStopCount(); j++)
						{
							if (k == organfile->GetManual(i)->GetStop(j)->ShortcutKey)
								organfile->GetManual(i)->GetStop(j)->Push();
						}
						for (int j = 0; j < organfile->GetManual(i)->GetCouplerCount(); j++)
							if (k == organfile->GetManual(i)->GetCoupler(j)->ShortcutKey)
								organfile->GetManual(i)->GetCoupler(j)->Push();
						for (int j = 0; j < organfile->GetManual(i)->GetDivisionalCount(); j++)
							if (k == organfile->GetManual(i)->GetDivisional(j)->ShortcutKey)
								organfile->GetManual(i)->GetDivisional(j)->Push();
					}
					for (unsigned j = 0; j < organfile->GetTremulantCount(); j++)
						if (k == organfile->GetTremulant(j)->ShortcutKey)
							organfile->GetTremulant(j)->Push();
					for (int j = 0; j < organfile->GetDivisionalCouplerCount(); j++)
						if (k == organfile->GetDivisionalCoupler(j)->ShortcutKey)
							organfile->GetDivisionalCoupler(j)->Push();
					for (int j = 0; j < organfile->GetGeneralCount(); j++)
						if (k == organfile->GetGeneral(j)->ShortcutKey)
							organfile->GetGeneral(j)->Push();
					for (int j = 0; j < organfile->GetNumberOfReversiblePistons(); j++)
						if (k == organfile->GetPiston(j)->ShortcutKey)
							organfile->GetPiston(j)->Push();
				}
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
