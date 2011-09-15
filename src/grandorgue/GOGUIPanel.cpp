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
#include "GOGUIDrawStop.h"
#include "GOGUIEnclosure.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanel.h"
#include "GOGUIPanelWidget.h"
#include "GOGUIPushbutton.h"
#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueGeneral.h"
#include "GOrgueManual.h"
#include "GOrguePiston.h"
#include "GOrgueSetter.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include "GOGUIDisplayMetrics.h"
#include "GOGUIHW1DisplayMetrics.h"
#include "IniFileConfig.h"
#include "Images.h"

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
	for (unsigned i = 0; i < GetImageCount_Stop(); i++)
		m_images.push_back(new wxBitmap(GetImage_Stop(i)));

	for (unsigned i = 0; i < GetImageCount_Wood(); i++)
	{
		wxBitmap bmp = GetImage_Wood(i);
		m_WoodImages.push_back(new wxBitmap(bmp));
		m_WoodImages.push_back(new wxBitmap(bmp.ConvertToImage().Rotate90()));
	}
}

GOGUIPanel::~GOGUIPanel()
{
	if (m_metrics)
		delete m_metrics;
	if (m_parent)
		m_parent->Destroy();
	if (m_window)
		m_window->Destroy();
}

GrandOrgueFile* GOGUIPanel::GetOrganFile()
{
	return m_organfile;
}

const wxString& GOGUIPanel::GetName()
{
	return m_Name;
}

GOGUIPanelWidget* GOGUIPanel::GetWindow()
{
	return m_window;
}

void GOGUIPanel::SetWindow(GOGUIPanelWidget* window)
{
	m_window = window;
}


void GOGUIPanel::Init(GOGUIDisplayMetrics* metrics, wxString name)
{
	m_metrics = metrics;
	m_Name = name;
	m_controls.resize(0);
}


void GOGUIPanel::Load(IniFileConfig& cfg, wxString group)
{
	m_metrics = new GOGUIHW1DisplayMetrics(cfg, m_organfile, wxT(""));

	{
		wxString buffer;
		m_Name = m_organfile->GetChurchName();

		{
			buffer.Printf(wxT("---"));
			GOGUIControl* control = new GOGUIHW1Background(this);
			control->Load(cfg, buffer);
			AddControl(control);
		}
		
		for (unsigned int i = m_organfile->GetFirstManualIndex(); i <= m_organfile->GetManualAndPedalCount(); i++)
		{
			wxString group;
			group.Printf(wxT("Manual%03d"), i);
			if (m_organfile->GetManual(i)->IsDisplayed())
			{
				GOGUIControl* control = new GOGUIManualBackground(this, i);
				control->Load(cfg, group);
				AddControl(control);
			}
		}

		for (unsigned i = 0; i < m_organfile->GetEnclosureCount(); i++)
		{
			buffer.Printf(wxT("Enclosure%03d"), i + 1);
			GOGUIControl* control = new GOGUIEnclosure(this, m_organfile->GetEnclosure(i), i);
			control->Load(cfg, buffer);
			AddControl(control);
		}

		for (unsigned i = 0; i < m_organfile->GetTremulantCount(); i++)
			if (m_organfile->GetTremulant(i)->IsDisplayed())
			{
				buffer.Printf(wxT("Tremulant%03d"), i + 1);
				GOGUIControl* control = new GOGUIDrawstop(this, m_organfile->GetTremulant(i));
				control->Load(cfg, buffer);
				AddControl(control);
			}	
		
		for (unsigned i = 0; i < m_organfile->GetDivisionalCouplerCount(); i++)
			if (m_organfile->GetDivisionalCoupler(i)->IsDisplayed())
			{
				buffer.Printf(wxT("DivisionalCoupler%03d"), i + 1);
				GOGUIControl* control = new GOGUIDrawstop(this, m_organfile->GetDivisionalCoupler(i));
				control->Load(cfg, buffer);
				AddControl(control);
			}	
		
		for (unsigned i = 0; i < m_organfile->GetGeneralCount(); i++)
			if (m_organfile->GetGeneral(i)->IsDisplayed())
			{
				buffer.Printf(wxT("General%03d"), i + 1);
				GOGUIControl* control = new GOGUIPushbutton(this, m_organfile->GetGeneral(i));
				control->Load(cfg, buffer);
				AddControl(control);
			}	

		for (unsigned i = 0; i < m_organfile->GetNumberOfReversiblePistons(); i++)
			if (m_organfile->GetPiston(i)->IsDisplayed())
			{
				buffer.Printf(wxT("ReversiblePiston%03d"), i + 1);
				GOGUIControl* control = new GOGUIPushbutton(this, m_organfile->GetPiston(i));
				control->Load(cfg, buffer);
				AddControl(control);
			}	

		for (unsigned int i = m_organfile->GetFirstManualIndex(); i <= m_organfile->GetManualAndPedalCount(); i++)
		{
			wxString group;
			group.Printf(wxT("Manual%03d"), i);
			if (m_organfile->GetManual(i)->IsDisplayed())
			{
				GOGUIControl* control = new GOGUIManual(this, m_organfile->GetManual(i), i);
				control->Load(cfg, group);
				AddControl(control);
			}

			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetCouplerCount(); j++)
				if (m_organfile->GetManual(i)->GetCoupler(j)->IsDisplayed())
				{
					buffer.Printf(wxT("Coupler%03d"), j + 1);
					buffer.Printf(wxT("Coupler%03d"), cfg.ReadInteger(group, buffer, 1, 64));
					GOGUIControl* control = new GOGUIDrawstop(this, m_organfile->GetManual(i)->GetCoupler(j));
					control->Load(cfg, buffer);
					AddControl(control);
				}

			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetStopCount(); j++)
				if (m_organfile->GetManual(i)->GetStop(j)->IsDisplayed())
				{
					buffer.Printf(wxT("Stop%03d"), j + 1);
					buffer.Printf(wxT("Stop%03d"), cfg.ReadInteger(group, buffer, 1, 448));
					GOGUIControl* control = new GOGUIDrawstop(this, m_organfile->GetManual(i)->GetStop(j));
					control->Load(cfg, buffer);
					AddControl(control);
				}
				
			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetDivisionalCount(); j++)
				if (m_organfile->GetManual(i)->GetDivisional(j)->IsDisplayed())
				{
					buffer.Printf(wxT("Divisional%03d"), j + 1);
					buffer.Printf(wxT("Divisional%03d"), cfg.ReadInteger(group, buffer, 1, 224));
					GOGUIControl* control = new GOGUIPushbutton(this, m_organfile->GetManual(i)->GetDivisional(j));
					control->Load(cfg, buffer);
					AddControl(control);
				}
		}

		unsigned m_NumberOfLabels = cfg.ReadInteger(group, wxT("NumberOfLabels"), 0, 16);
		for (unsigned i = 0; i < m_NumberOfLabels; i++)
		{
			buffer.Printf(wxT("Label%03d"), i + 1);
			GOGUIControl* control = new GOGUILabel(this, NULL);
			control->Load(cfg, buffer);
			AddControl(control);
		}
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
	return m_parent;
}

void GOGUIPanel::SetParentWindow(wxWindow* win)
{
	m_parent = win;
}

void GOGUIPanel::AddEvent(GOGUIControl* control)
{
	wxCommandEvent event(wxEVT_GOCONTROL, 0);
	event.SetClientData(control);
	if (m_window)
		m_window->AddPendingEvent(event);
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
	switch(key)
	{
	case 256: /* Left */
		m_organfile->GetSetter()->Prev();
		break;

	case 257: /* right */
		m_organfile->GetSetter()->Next();
		break;

	case 258: /* down */
		m_organfile->GetSetter()->Push();
		break;

	case 259: /* Shift not down */
		m_organfile->GetSetter()->SetterActive(false);
		break;

	case 260: /* Shift down */
		m_organfile->GetSetter()->SetterActive(true);
		break;
	}
	
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
