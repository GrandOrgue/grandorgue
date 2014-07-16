/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOGUIPanel.h"

#include "GOGUIButton.h"
#include "GOGUIControl.h"
#include "GOGUIDisplayMetrics.h"
#include "GOGUIEnclosure.h"
#include "GOGUIHW1Background.h"
#include "GOGUIHW1DisplayMetrics.h"
#include "GOGUIImage.h"
#include "GOGUILabel.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanelWidget.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueCoupler.h"
#include "GOrgueDC.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueGeneral.h"
#include "GOrgueManual.h"
#include "GOrguePanelView.h"
#include "GOrguePiston.h"
#include "GOrgueSetter.h"
#include "GOrgueSwitch.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include "Images.h"
#include <wx/image.h>

GOGUIPanel::GOGUIPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_controls(0),
	m_WoodImages(0),
	m_Name(),
	m_GroupName(),
	m_metrics(0),
	m_layout(0),
	m_view(0),
	m_size(0, 0, 0, 0),
	m_InitialOpenWindow(false)
{
	for (unsigned i = 0; i < 64; i++)
		m_WoodImages.push_back(LoadBitmap(wxString::Format(wxT("GO:wood%02d"), i + 1), wxEmptyString));
}

GOGUIPanel::~GOGUIPanel()
{
	if (m_metrics)
		delete m_metrics;
}

bool GOGUIPanel::InitialOpenWindow()
{
	return m_InitialOpenWindow;
}

GrandOrgueFile* GOGUIPanel::GetOrganFile()
{
	return m_organfile;
}

const wxString& GOGUIPanel::GetName()
{
	return m_Name;
}

const wxString& GOGUIPanel::GetGroupName()
{
	return m_GroupName;
}

GOrgueBitmap GOGUIPanel::LoadBitmap(wxString filename, wxString maskname)
{
	return m_organfile->GetBitmapCache().GetBitmap(filename, maskname);
}

void GOGUIPanel::SetView(GOrguePanelView* view)
{
	m_view = view;
}

void GOGUIPanel::Init(GOrgueConfigReader& cfg, GOGUIDisplayMetrics* metrics, wxString name, wxString group, wxString group_name)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_metrics = metrics;
	m_layout = metrics;
	m_Name = name;
	m_GroupName = group_name;
	m_controls.resize(0);

	int x = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowX"), -20, 10000, false, 0);
	int y = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowY"), -20, 10000, false, 0);
	int w = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowWidth"), 0, 10000, false, 0);
	int h = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowHeight"), 0, 10000, false, 0);
	m_size = wxRect(x, y, w, h);

	m_InitialOpenWindow = cfg.ReadBoolean(CMBSetting, m_group, wxT("WindowDisplayed"), false, false);
}


void GOGUIPanel::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_organfile->RegisterSaveableObject(this);
	m_group = group;
	m_metrics = new GOGUIHW1DisplayMetrics(cfg, m_organfile, group);
	m_layout = m_metrics;

	if (group.IsEmpty())
	{
		wxString buffer;
		m_Name = m_organfile->GetChurchName();
		m_GroupName = wxT("");
		m_group = group = wxT("Organ");

		{
			buffer.Printf(wxT("---"));
			GOGUIControl* control = new GOGUIHW1Background(this);
			control->Load(cfg, buffer);
			AddControl(control);
		}
		
		for(unsigned i = 0; i < m_organfile->GetFirstManualIndex(); i++)
			m_layout->RegisterManual(0);

		for (unsigned no = m_organfile->GetFirstManualIndex(), i = m_organfile->GetFirstManualIndex(); i <= m_organfile->GetManualAndPedalCount(); i++)
		{
			wxString group;
			group.Printf(wxT("Manual%03d"), i);
			if (m_organfile->GetManual(i)->IsDisplayed())
			{
				GOGUIControl* control = new GOGUIManualBackground(this, no++);
				control->Load(cfg, group);
				AddControl(control);
			}
		}

		unsigned NumberOfImages = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfImages"), 0, 999, false, 0);
		for (unsigned i = 0; i < NumberOfImages; i++)
		{
			buffer.Printf(wxT("Image%03d"), i + 1);
			GOGUIControl* control = new GOGUIImage(this);
			control->Load(cfg, buffer);
			AddControl(control);
		}

		for (unsigned i = 0; i < m_organfile->GetEnclosureCount(); i++)
			if (m_organfile->GetEnclosure(i)->IsDisplayed())
			{
				buffer.Printf(wxT("Enclosure%03d"), i + 1);
				GOGUIControl* control = new GOGUIEnclosure(this, m_organfile->GetEnclosure(i));
				control->Load(cfg, buffer);
				AddControl(control);
			}

		unsigned NumberOfSetterElements = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSetterElements"), 0, 999, false);
		for (unsigned i = 0; i < NumberOfSetterElements; i++)
		{
			buffer.Printf(wxT("SetterElement%03d"), i + 1);
			AddControl(m_organfile->GetSetter()->CreateGUIElement(cfg, buffer, this));
		}

		for (unsigned i = 0; i < m_organfile->GetTremulantCount(); i++)
			if (m_organfile->GetTremulant(i)->IsDisplayed())
			{
				buffer.Printf(wxT("Tremulant%03d"), i + 1);
				GOGUIControl* control = new GOGUIButton(this, m_organfile->GetTremulant(i));
				control->Load(cfg, buffer);
				AddControl(control);
			}	
		
		for (unsigned i = 0; i < m_organfile->GetDivisionalCouplerCount(); i++)
			if (m_organfile->GetDivisionalCoupler(i)->IsDisplayed())
			{
				buffer.Printf(wxT("DivisionalCoupler%03d"), i + 1);
				GOGUIControl* control = new GOGUIButton(this, m_organfile->GetDivisionalCoupler(i));
				control->Load(cfg, buffer);
				AddControl(control);
			}	
		
		for (unsigned i = 0; i < m_organfile->GetGeneralCount(); i++)
			if (m_organfile->GetGeneral(i)->IsDisplayed())
			{
				buffer.Printf(wxT("General%03d"), i + 1);
				GOGUIControl* control = new GOGUIButton(this, m_organfile->GetGeneral(i), true);
				control->Load(cfg, buffer);
				AddControl(control);
			}	

		for (unsigned i = 0; i < m_organfile->GetNumberOfReversiblePistons(); i++)
			if (m_organfile->GetPiston(i)->IsDisplayed())
			{
				buffer.Printf(wxT("ReversiblePiston%03d"), i + 1);
				GOGUIControl* control = new GOGUIButton(this, m_organfile->GetPiston(i), true);
				control->Load(cfg, buffer);
				AddControl(control);
			}	

		for (unsigned i = 0; i < m_organfile->GetSwitchCount(); i++)
			if (m_organfile->GetSwitch(i)->IsDisplayed())
			{
				buffer.Printf(wxT("Switch%03d"), i + 1);
				GOGUIControl* control = new GOGUIButton(this, m_organfile->GetSwitch(i), false);
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
					buffer.Printf(wxT("Coupler%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 64));
					GOGUIControl* control = new GOGUIButton(this, m_organfile->GetManual(i)->GetCoupler(j));
					control->Load(cfg, buffer);
					AddControl(control);
				}

			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetStopCount(); j++)
				if (m_organfile->GetManual(i)->GetStop(j)->IsDisplayed())
				{
					buffer.Printf(wxT("Stop%03d"), j + 1);
					buffer.Printf(wxT("Stop%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 448));
					GOGUIControl* control = new GOGUIButton(this, m_organfile->GetManual(i)->GetStop(j));
					control->Load(cfg, buffer);
					AddControl(control);
				}
				
			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetDivisionalCount(); j++)
				if (m_organfile->GetManual(i)->GetDivisional(j)->IsDisplayed())
				{
					buffer.Printf(wxT("Divisional%03d"), j + 1);
					buffer.Printf(wxT("Divisional%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 224));
					GOGUIControl* control = new GOGUIButton(this, m_organfile->GetManual(i)->GetDivisional(j), true);
					control->Load(cfg, buffer);
					AddControl(control);
				}
		}

		unsigned NumberOfLabels = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfLabels"), 0, 999);
		for (unsigned i = 0; i < NumberOfLabels; i++)
		{
			buffer.Printf(wxT("Label%03d"), i + 1);
			GOGUIControl* control = new GOGUILabel(this, NULL);
			control->Load(cfg, buffer);
			AddControl(control);
		}
	}
	else
	{
		wxString buffer;
		m_Name = cfg.ReadString(ODFSetting, group, wxT("Name"));
		m_GroupName = cfg.ReadString(ODFSetting, group, wxT("Group"), false);
		unsigned nb_manuals    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfManuals"), 0, m_organfile->GetManualAndPedalCount());
		unsigned first_manual  = cfg.ReadBoolean(ODFSetting, group, wxT("HasPedals")) ? 0 : 1;
		unsigned NumberOfEnclosures = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, m_organfile->GetEnclosureCount());
		unsigned NumberOfTremulants = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, m_organfile->GetTremulantCount());
		unsigned NumberOfSwitches = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSwitches"), 0, m_organfile->GetSwitchCount(), false, 0);
		unsigned NumberOfReversiblePistons = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfReversiblePistons"), 0, m_organfile->GetNumberOfReversiblePistons());
		unsigned NumberOfGenerals = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfGenerals"), 0, m_organfile->GetGeneralCount());
		unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfDivisionalCouplers"), 0, m_organfile->GetDivisionalCouplerCount());
		unsigned NumberOfStops = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfStops"), 0, 999);
		unsigned NumberOfCouplers = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfCouplers"), 0, 999);
		unsigned NumberOfDivisionals = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfDivisionals"), 0, 999);

		if (first_manual < m_organfile->GetFirstManualIndex())
			first_manual = m_organfile->GetFirstManualIndex();

		{
			buffer.Printf(wxT("---"));
			GOGUIControl* control = new GOGUIHW1Background(this);
			control->Load(cfg, buffer);
			AddControl(control);
		}
		
		for(unsigned i = 0; i < first_manual; i++)
			m_layout->RegisterManual(0);
		for (unsigned int i = first_manual; i <= nb_manuals; i++)
		{
			buffer.Printf(wxT("Manual%03d"), i);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer.Printf(wxT("Manual%03d"), manual_nb);
			GOGUIControl* control = new GOGUIManualBackground(this, i);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}

		unsigned NumberOfImages = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfImages"), 0, 999, false, 0);
		for (unsigned i = 0; i < NumberOfImages; i++)
		{
			buffer.Printf(wxT("Image%03d"), i + 1);
			GOGUIControl* control = new GOGUIImage(this);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}

		for (unsigned i = 0; i < NumberOfEnclosures; i++)
		{
			buffer.Printf(wxT("Enclosure%03d"), i + 1);
			unsigned enclosure_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetEnclosureCount());
			buffer.Printf(wxT("Enclosure%03d"), enclosure_nb);
			GOGUIControl* control = new GOGUIEnclosure(this, m_organfile->GetEnclosure(enclosure_nb - 1));
			m_organfile->MarkSectionInUse(group + buffer);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}

		unsigned NumberOfSetterElements = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfSetterElements"), 0, 999, false);
		for (unsigned i = 0; i < NumberOfSetterElements; i++)
		{
			buffer.Printf(wxT("SetterElement%03d"), i + 1);
			AddControl(m_organfile->GetSetter()->CreateGUIElement(cfg, group + buffer, this));
		}

		for (unsigned i = 0; i < NumberOfTremulants; i++)
		{
			buffer.Printf(wxT("Tremulant%03d"), i + 1);
			unsigned tremulant_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetTremulantCount());
			buffer.Printf(wxT("Tremulant%03d"), tremulant_nb);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetTremulant(tremulant_nb - 1));
			m_organfile->MarkSectionInUse(group + buffer);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}	
		
		for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++)
		{
			buffer.Printf(wxT("DivisionalCoupler%03d"), i + 1);
			unsigned coupler_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetDivisionalCouplerCount());
			buffer.Printf(wxT("DivisionalCoupler%03d"), coupler_nb);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetDivisionalCoupler(coupler_nb - 1));
			m_organfile->MarkSectionInUse(group + buffer);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}	
		
		for (unsigned i = 0; i < NumberOfGenerals; i++)
		{
			buffer.Printf(wxT("General%03d"), i + 1);
			unsigned general_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetGeneralCount());
			buffer.Printf(wxT("General%03d"), general_nb);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetGeneral(general_nb - 1), true);
			m_organfile->MarkSectionInUse(group + buffer);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}	

		for (unsigned i = 0; i < NumberOfReversiblePistons; i++)
		{
			buffer.Printf(wxT("ReversiblePiston%03d"), i + 1);
			unsigned piston_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetNumberOfReversiblePistons());
			buffer.Printf(wxT("ReversiblePiston%03d"), piston_nb);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetPiston(piston_nb - 1), true);
			m_organfile->MarkSectionInUse(group + buffer);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}	

		for (unsigned i = 0; i < NumberOfSwitches; i++)
		{
			buffer.Printf(wxT("Switch%03d"), i + 1);
			unsigned switch_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetSwitchCount());
			buffer.Printf(wxT("Switch%03d"), switch_nb);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetSwitch(switch_nb - 1), false);
			m_organfile->MarkSectionInUse(group + buffer);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}	

		for (unsigned int i = first_manual; i <= nb_manuals; i++)
		{
			buffer.Printf(wxT("Manual%03d"), i);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer.Printf(wxT("Manual%03d"), manual_nb);
			GOGUIControl* control = new GOGUIManual(this, m_organfile->GetManual(manual_nb), i);
			m_organfile->MarkSectionInUse(group + buffer);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}

		for(unsigned j = 0; j < NumberOfCouplers; j++)
		{
			buffer.Printf(wxT("Coupler%03dManual"), j + 1);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer.Printf(wxT("Coupler%03d"), j + 1);
			unsigned coupler_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetManual(manual_nb)->GetCouplerCount());
			buffer.Printf(wxT("Coupler%03d"), j + 1);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetCoupler(coupler_nb - 1));
			m_organfile->MarkSectionInUse(group + wxString::Format(wxT("Manual%03dCoupler%03d"), manual_nb, coupler_nb));
			control->Load(cfg, group + buffer);
			AddControl(control);
		}

		for(unsigned j = 0; j < NumberOfStops; j++)
		{
			buffer.Printf(wxT("Stop%03dManual"), j + 1);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer.Printf(wxT("Stop%03d"), j + 1);
			unsigned stop_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetManual(manual_nb)->GetStopCount());
			buffer.Printf(wxT("Stop%03d"), j + 1);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetStop(stop_nb - 1));
			m_organfile->MarkSectionInUse(group + wxString::Format(wxT("Manual%03dStop%03d"), manual_nb, stop_nb));
			control->Load(cfg, group + buffer);
			AddControl(control);
		}
				
		for(unsigned j = 0; j < NumberOfDivisionals; j++)
		{
			buffer.Printf(wxT("Divisional%03dManual"), j + 1);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer.Printf(wxT("Divisional%03d"), j + 1);
			unsigned divisional_nb  = cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetManual(manual_nb)->GetDivisionalCount());
			buffer.Printf(wxT("Divisional%03d"), j + 1);
			GOGUIControl* control = new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetDivisional(divisional_nb - 1), true);
			m_organfile->MarkSectionInUse(group + wxString::Format(wxT("Manual%03dDivisional%03d"), manual_nb, divisional_nb));
			control->Load(cfg, group + buffer);
			AddControl(control);
		}

		unsigned NumberOfLabels = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfLabels"), 0, 999);
		for (unsigned i = 0; i < NumberOfLabels; i++)
		{
			buffer.Printf(wxT("Label%03d"), i + 1);
			GOGUIControl* control = new GOGUILabel(this, NULL);
			control->Load(cfg, group + buffer);
			AddControl(control);
		}
	}

	int x = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowX"), -20, 10000, false, 0);
	int y = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowY"), -20, 10000, false, 0);
	int w = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowWidth"), 0, 10000, false, 0);
	int h = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowHeight"), 0, 10000, false, 0);
	m_size = wxRect(x, y, w, h);

	m_InitialOpenWindow = cfg.ReadBoolean(CMBSetting, m_group, wxT("WindowDisplayed"), false, false);
}

void GOGUIPanel::Layout()
{
	m_layout->Update();

	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->Layout();
}

unsigned GOGUIPanel::GetWidth()
{
	return m_metrics->GetScreenWidth();
}

unsigned GOGUIPanel::GetHeight()
{
	return m_metrics->GetScreenHeight();
}

wxRect GOGUIPanel::GetWindowSize()
{
	return m_size;
}

void GOGUIPanel::SetWindowSize(wxRect rect)
{
	m_size = rect;
}

void GOGUIPanel::SetInitialOpenWindow(bool open)
{
	m_InitialOpenWindow = open;
}

void GOGUIPanel::AddEvent(GOGUIControl* control)
{
	if (m_view)
		m_view->AddEvent(control);
}

void GOGUIPanel::AddControl(GOGUIControl* control)
{
	m_controls.push_back(control);
}

GOGUIDisplayMetrics* GOGUIPanel::GetDisplayMetrics()
{
	return m_metrics;
}

GOGUIDisplayMetrics* GOGUIPanel::GetLayoutEngine()
{
	return m_layout;
}

void GOGUIPanel::ControlChanged(void* control)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->ControlChanged(control);
}

void GOGUIPanel::Draw(GOrgueDC& dc)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		m_controls[i]->Draw(dc);
}

void GOGUIPanel::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteBoolean(m_group, wxT("WindowDisplayed"), m_InitialOpenWindow);
	wxRect size = m_size;
	int x = size.GetLeft();
	int y = size.GetTop();
	if (x < -20)
		x = -20;
	if (y < -20)
		y = -20;
	cfg.WriteInteger(m_group, wxT("WindowX"), x);
	cfg.WriteInteger(m_group, wxT("WindowY"), y);
	cfg.WriteInteger(m_group, wxT("WindowWidth"), size.GetWidth());
	cfg.WriteInteger(m_group, wxT("WindowHeight"), size.GetHeight());
}

void GOGUIPanel::Modified()
{
	m_organfile->Modified();
}

void GOGUIPanel::HandleKey(int key)
{
	switch(key)
	{
	case 259: /* Shift not down */
		m_organfile->GetSetter()->SetterActive(false);
		break;

	case 260: /* Shift down */
		m_organfile->GetSetter()->SetterActive(true);
		break;
	}
	
	m_organfile->HandleKey(key);
}

void GOGUIPanel::HandleMousePress(int x, int y, bool right, GOGUIMouseState& state)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		if (m_controls[i]->HandleMousePress(x, y, right, state))
			return;
}

void GOGUIPanel::HandleMouseScroll(int x, int y, int amount)
{
	for(unsigned i = 0; i < m_controls.size(); i++)
		if (m_controls[i]->HandleMouseScroll(x, y, amount))
			return;
}

void GOGUIPanel::TileWood(GOrgueDC& dc, unsigned index, int sx, int sy, int cx, int cy)
{
	dc.TileBitmap(m_WoodImages[index - 1], wxRect(sx, sy, cx, cy), 0, 0);
}
