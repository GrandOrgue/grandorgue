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

#include <wx/image.h>
#include <wx/mstream.h>
#include "GOGUIControl.h"
#include "GOGUIPanel.h"
#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueGeneral.h"
#include "GOrgueManual.h"
#include "GOrguePiston.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include "GOGUIDisplayMetrics.h"
#include "IniFileConfig.h"
#include "Images.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"

GOGUIPanel::GOGUIPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_controls(0),
	m_images(0),
	m_WoodImages(0),
	m_Name(),
	m_metrics(0),
	m_window(0),
	m_parent(0)
{
	for (int i = 0; i < count_ImageLoader_Stops; ++i)
	{
		wxMemoryInputStream mem((const char*)ImageLoader_Stops[i], c_ImageLoader_Stops[i]);
		wxImage img(mem, wxBITMAP_TYPE_PNG);
		m_images.push_back(new wxBitmap(img));
	}
	for (int i = 0; i < count_ImageLoader_Wood; ++i)
	{
		wxMemoryInputStream mem((const char*)ImageLoader_Wood[i], c_ImageLoader_Wood[i]);
		wxImage img(mem, wxBITMAP_TYPE_JPEG);
		m_WoodImages.push_back(new wxBitmap(img));
		img = img.Rotate90();
		m_WoodImages.push_back(new wxBitmap(img));
	}
}

GOGUIPanel::~GOGUIPanel()
{
	if (m_metrics)
		delete m_metrics;
	if (m_parent)
		m_parent->Destroy();
}

GrandOrgueFile* GOGUIPanel::GetOrganFile()
{
	return m_organfile;
}

const wxString& GOGUIPanel::GetName()
{
	return m_Name;
}

wxWindow* GOGUIPanel::GetWindow()
{
	return m_window;
}

void GOGUIPanel::SetWindow(wxWindow* window)
{
	m_window = window;
}

void GOGUIPanel::Load(IniFileConfig& cfg, wxString group)
{
	m_metrics = new GOGUIDisplayMetrics(cfg);

	{
		m_Name = m_organfile->GetChurchName();
	}
}

unsigned GOGUIPanel::GetWidth()
{
	return m_metrics->GetScreenWidth();
}

unsigned GOGUIPanel::GetHeight()
{
	return m_metrics->GetScreenHeight();
}

wxWindow* GOGUIPanel::GetParentWindow()
{
	return ::wxGetApp().frame;
}

void GOGUIPanel::SetParentWindow(wxWindow* win)
{
	m_parent = win;
}

void GOGUIPanel::AddEvent(GOGUIControl* control)
{
	wxCommandEvent event(wxEVT_GOCONTROL, 0);
	event.SetClientData(control);
	::wxGetApp().frame->AddPendingEvent(event);
}

void GOGUIPanel::AddControl(GOGUIControl* control)
{
	m_controls.push_back(control);
}

GOGUIDisplayMetrics* GOGUIPanel::GetDisplayMetrics()
{
	return m_metrics;
}

void GOGUIPanel::ControlChanged(void* control)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->ControlChanged(control);
}

void GOGUIPanel::Draw(wxDC* dc)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->Draw(dc);
}

void GOGUIPanel::Save(IniFileConfig& cfg, bool prefix)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->Save(cfg, prefix);
}

void GOGUIPanel::Modified()
{
	m_organfile->Modified();
}

wxBitmap* GOGUIPanel::GetImage(unsigned index)
{
	return m_images[index];
}

wxBitmap* GOGUIPanel::GetWoodImage(unsigned index)
{
	return m_WoodImages[index];
}

void GOGUIPanel::HandleKey(int key)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->HandleKey(key);
}

void GOGUIPanel::HandleMousePress(int x, int y, bool right)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->HandleMousePress(x, y, right);
}

void GOGUIPanel::HandleMouseScroll(int x, int y, int amount)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->HandleMouseScroll(x, y, amount);
}

void GOGUIPanel::TileWood(wxDC* dc, int which, int sx, int sy, int cx, int cy)
{
	int x, y;
	wxBitmap *bmp = GetWoodImage(which - 1);
	dc->SetClippingRegion(sx, sy, cx, cy);
	for (y = sy & 0xFFFFFF00; y < sy + cy; y += 256)
		for (x = sx & 0xFFFFFF00; x < sx + cx; x += 256)
			dc->DrawBitmap(*bmp, x, y, false);
	dc->DestroyClippingRegion();
}

wxString GOGUIPanel::WrapText(wxDC* dc, const wxString& string, int width)
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
			dc->GetTextExtent(line + wxT(' ') + work, &cx, &cy);
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
	return str;
}
