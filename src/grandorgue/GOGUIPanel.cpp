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
#include "GOGUILayoutEngine.h"
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
	m_BackgroundControls(0),
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
	if (m_layout)
		delete m_layout;
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
	m_layout = new GOGUILayoutEngine(*m_metrics);
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
	wxString panel_group;
	wxString panel_prefix;
	bool is_main_panel;
	bool new_format = false;

	if (group.IsEmpty())
	{
		m_Name = m_organfile->GetChurchName();
		m_GroupName = wxT("");
		m_group = wxT("Organ");
		if (new_format)
		{
			panel_group = wxT("Panel000");
			panel_prefix = panel_group;
		}
		else
		{
			panel_group = m_group;
			panel_prefix = wxEmptyString;
		}
		is_main_panel = true;
	}
	else
	{
		panel_group = group;
		panel_prefix = panel_group;
		is_main_panel = false;
		m_Name = cfg.ReadString(ODFSetting, panel_group, wxT("Name"));
		m_GroupName = cfg.ReadString(ODFSetting, panel_group, wxT("Group"), false);
	}

	m_metrics = new GOGUIHW1DisplayMetrics(cfg, panel_group);
	m_layout = new GOGUILayoutEngine(*m_metrics);

	unsigned first_manual  = cfg.ReadBoolean(ODFSetting, panel_group, wxT("HasPedals")) ? 0 : 1;
	for(unsigned i = 0; i < first_manual; i++)
		m_layout->RegisterManual(0);

	LoadBackgroundControl(new GOGUIHW1Background(this), cfg, wxString::Format(wxT("---")));

	unsigned NumberOfImages = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfImages"), 0, 999, false, 0);
	for (unsigned i = 0; i < NumberOfImages; i++)
	{
		LoadControl(new GOGUIImage(this), cfg, panel_prefix + wxString::Format(wxT("Image%03d"), i + 1));
	}

	if (is_main_panel)
	{
		for (unsigned i = 0; i < m_organfile->GetEnclosureCount(); i++)
			if (m_organfile->GetEnclosure(i)->IsDisplayed())
			{
				LoadControl(new GOGUIEnclosure(this, m_organfile->GetEnclosure(i)), cfg, wxString::Format(wxT("Enclosure%03d"), i + 1));
			}

		if (!new_format)
		{
			unsigned NumberOfSetterElements = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfSetterElements"), 0, 999, false, 0);
			for (unsigned i = 0; i < NumberOfSetterElements; i++)
			{
				wxString buffer = wxString::Format(wxT("SetterElement%03d"), i + 1);
				GOGUIControl* control = m_organfile->GetSetter()->CreateGUIElement(cfg, buffer, this);
				if (!control)
					throw (wxString)wxString::Format(_("Unkown SetterElement in section %s"), buffer.c_str());
				LoadControl(control, cfg, buffer);
			}
		}

		for (unsigned i = 0; i < m_organfile->GetTremulantCount(); i++)
			if (m_organfile->GetTremulant(i)->IsDisplayed())
			{
				LoadControl(new GOGUIButton(this, m_organfile->GetTremulant(i)), cfg, wxString::Format(wxT("Tremulant%03d"), i + 1));
			}	
		
		for (unsigned i = 0; i < m_organfile->GetDivisionalCouplerCount(); i++)
			if (m_organfile->GetDivisionalCoupler(i)->IsDisplayed())
			{
				LoadControl(new GOGUIButton(this, m_organfile->GetDivisionalCoupler(i)), cfg, wxString::Format(wxT("DivisionalCoupler%03d"), i + 1));
			}	
		
		for (unsigned i = 0; i < m_organfile->GetGeneralCount(); i++)
			if (m_organfile->GetGeneral(i)->IsDisplayed())
			{
				LoadControl(new GOGUIButton(this, m_organfile->GetGeneral(i), true), cfg, wxString::Format(wxT("General%03d"), i + 1));
			}	

		for (unsigned i = 0; i < m_organfile->GetNumberOfReversiblePistons(); i++)
			if (m_organfile->GetPiston(i)->IsDisplayed())
			{
				LoadControl(new GOGUIButton(this, m_organfile->GetPiston(i), true), cfg, wxString::Format(wxT("ReversiblePiston%03d"), i + 1));
			}	

		for (unsigned i = 0; i < m_organfile->GetSwitchCount(); i++)
			if (m_organfile->GetSwitch(i)->IsDisplayed())
			{
				LoadControl(new GOGUIButton(this, m_organfile->GetSwitch(i), false), cfg, wxString::Format(wxT("Switch%03d"), i + 1));
			}	

		for (unsigned int i = m_organfile->GetFirstManualIndex(); i <= m_organfile->GetManualAndPedalCount(); i++)
		{
			wxString manual_group = wxString::Format(wxT("Manual%03d"), i);
			if (m_organfile->GetManual(i)->IsDisplayed())
			{
				LoadBackgroundControl(new GOGUIManualBackground(this, m_layout->GetManualNumber()), cfg, manual_group);
				LoadControl(new GOGUIManual(this, m_organfile->GetManual(i), m_layout->GetManualNumber()), cfg, manual_group);
			}

			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetCouplerCount(); j++)
				if (m_organfile->GetManual(i)->GetCoupler(j)->IsDisplayed())
				{
					wxString buffer = wxString::Format(wxT("Coupler%03d"), j + 1);
					buffer = wxString::Format(wxT("Coupler%03d"), cfg.ReadInteger(ODFSetting, manual_group, buffer, 1, 64));
					LoadControl(new GOGUIButton(this, m_organfile->GetManual(i)->GetCoupler(j)), cfg, buffer);
				}

			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetStopCount(); j++)
				if (m_organfile->GetManual(i)->GetStop(j)->IsDisplayed())
				{
					wxString buffer = wxString::Format(wxT("Stop%03d"), j + 1);
					buffer = wxString::Format(wxT("Stop%03d"), cfg.ReadInteger(ODFSetting, manual_group, buffer, 1, 448));
					LoadControl(new GOGUIButton(this, m_organfile->GetManual(i)->GetStop(j)), cfg, buffer);
				}
				
			for(unsigned j = 0; j < m_organfile->GetManual(i)->GetDivisionalCount(); j++)
				if (m_organfile->GetManual(i)->GetDivisional(j)->IsDisplayed())
				{
					wxString buffer = wxString::Format(wxT("Divisional%03d"), j + 1);
					buffer = wxString::Format(wxT("Divisional%03d"), cfg.ReadInteger(ODFSetting, manual_group, buffer, 1, 224));
					LoadControl(new GOGUIButton(this, m_organfile->GetManual(i)->GetDivisional(j), true), cfg, buffer);
				}
		}
	}
	else if (!new_format)
	{
		unsigned NumberOfEnclosures = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfEnclosures"), 0, m_organfile->GetEnclosureCount());
		for (unsigned i = 0; i < NumberOfEnclosures; i++)
		{
			wxString buffer = wxString::Format(wxT("Enclosure%03d"), i + 1);
			unsigned enclosure_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetEnclosureCount());
			buffer = wxString::Format(wxT("Enclosure%03d"), enclosure_nb);
			LoadControl(new GOGUIEnclosure(this, m_organfile->GetEnclosure(enclosure_nb - 1)), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + buffer);
		}

		unsigned NumberOfSetterElements = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfSetterElements"), 0, 999, false);
		for (unsigned i = 0; i < NumberOfSetterElements; i++)
		{
			wxString buffer = wxString::Format(wxT("SetterElement%03d"), i + 1);
			GOGUIControl* control = m_organfile->GetSetter()->CreateGUIElement(cfg, panel_prefix + buffer, this);
			if (!control)
				throw (wxString)wxString::Format(_("Unkown SetterElement in section %s"), (panel_prefix + buffer).c_str());
			LoadControl(control, cfg, panel_prefix + buffer);
		}

		unsigned NumberOfTremulants = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfTremulants"), 0, m_organfile->GetTremulantCount());
		for (unsigned i = 0; i < NumberOfTremulants; i++)
		{
			wxString buffer = wxString::Format(wxT("Tremulant%03d"), i + 1);
			unsigned tremulant_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetTremulantCount());
			buffer = wxString::Format(wxT("Tremulant%03d"), tremulant_nb);
			LoadControl(new GOGUIButton(this, m_organfile->GetTremulant(tremulant_nb - 1)), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + buffer);
		}	
		
		unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfDivisionalCouplers"), 0, m_organfile->GetDivisionalCouplerCount());
		for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++)
		{
			wxString buffer = wxString::Format(wxT("DivisionalCoupler%03d"), i + 1);
			unsigned coupler_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetDivisionalCouplerCount());
			buffer = wxString::Format(wxT("DivisionalCoupler%03d"), coupler_nb);
			LoadControl(new GOGUIButton(this, m_organfile->GetDivisionalCoupler(coupler_nb - 1)), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + buffer);
		}	
		
		unsigned NumberOfGenerals = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfGenerals"), 0, m_organfile->GetGeneralCount());
		for (unsigned i = 0; i < NumberOfGenerals; i++)
		{
			wxString buffer = wxString::Format(wxT("General%03d"), i + 1);
			unsigned general_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetGeneralCount());
			buffer = wxString::Format(wxT("General%03d"), general_nb);
			LoadControl(new GOGUIButton(this, m_organfile->GetGeneral(general_nb - 1), true), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + buffer);
		}	

		unsigned NumberOfReversiblePistons = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfReversiblePistons"), 0, m_organfile->GetNumberOfReversiblePistons());
		for (unsigned i = 0; i < NumberOfReversiblePistons; i++)
		{
			wxString buffer = wxString::Format(wxT("ReversiblePiston%03d"), i + 1);
			unsigned piston_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetNumberOfReversiblePistons());
			buffer = wxString::Format(wxT("ReversiblePiston%03d"), piston_nb);
			LoadControl(new GOGUIButton(this, m_organfile->GetPiston(piston_nb - 1), true), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + buffer);
		}	

		unsigned NumberOfSwitches = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfSwitches"), 0, m_organfile->GetSwitchCount(), false, 0);
		for (unsigned i = 0; i < NumberOfSwitches; i++)
		{
			wxString buffer = wxString::Format(wxT("Switch%03d"), i + 1);
			unsigned switch_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetSwitchCount());
			buffer = wxString::Format(wxT("Switch%03d"), switch_nb);
			LoadControl(new GOGUIButton(this, m_organfile->GetSwitch(switch_nb - 1), false), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + buffer);
		}	

		unsigned nb_manuals = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfManuals"), 0, m_organfile->GetManualAndPedalCount());
		for (unsigned int i = first_manual; i <= nb_manuals; i++)
		{
			wxString buffer = wxString::Format(wxT("Manual%03d"), i);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer = wxString::Format(wxT("Manual%03d"), manual_nb);
			LoadBackgroundControl(new GOGUIManualBackground(this, i), cfg, panel_prefix + buffer);
			LoadControl(new GOGUIManual(this, m_organfile->GetManual(manual_nb), m_layout->GetManualNumber()), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + buffer);
		}

		unsigned NumberOfCouplers = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfCouplers"), 0, 999);
		for(unsigned j = 0; j < NumberOfCouplers; j++)
		{
			wxString buffer = wxString::Format(wxT("Coupler%03dManual"), j + 1);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer = wxString::Format(wxT("Coupler%03d"), j + 1);
			unsigned coupler_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetManual(manual_nb)->GetCouplerCount());
			buffer = wxString::Format(wxT("Coupler%03d"), j + 1);
			LoadControl(new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetCoupler(coupler_nb - 1)), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + wxString::Format(wxT("Manual%03dCoupler%03d"), manual_nb, coupler_nb));
		}

		unsigned NumberOfStops = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfStops"), 0, 999);
		for(unsigned j = 0; j < NumberOfStops; j++)
		{
			wxString buffer = wxString::Format(wxT("Stop%03dManual"), j + 1);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer = wxString::Format(wxT("Stop%03d"), j + 1);
			unsigned stop_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetManual(manual_nb)->GetStopCount());
			buffer = wxString::Format(wxT("Stop%03d"), j + 1);
			LoadControl(new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetStop(stop_nb - 1)), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + wxString::Format(wxT("Manual%03dStop%03d"), manual_nb, stop_nb));
		}
				
		unsigned NumberOfDivisionals = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfDivisionals"), 0, 999);
		for(unsigned j = 0; j < NumberOfDivisionals; j++)
		{
			wxString buffer = wxString::Format(wxT("Divisional%03dManual"), j + 1);
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			buffer = wxString::Format(wxT("Divisional%03d"), j + 1);
			unsigned divisional_nb  = cfg.ReadInteger(ODFSetting, panel_group, buffer, 1, m_organfile->GetManual(manual_nb)->GetDivisionalCount());
			buffer = wxString::Format(wxT("Divisional%03d"), j + 1);
			LoadControl(new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetDivisional(divisional_nb - 1), true), cfg, panel_prefix + buffer);
			m_organfile->MarkSectionInUse(panel_prefix + wxString::Format(wxT("Manual%03dDivisional%03d"), manual_nb, divisional_nb));
		}
	}

	if (!new_format)
	{
		unsigned NumberOfLabels = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfLabels"), 0, 999);
		for (unsigned i = 0; i < NumberOfLabels; i++)
		{
			wxString buffer = wxString::Format(wxT("Label%03d"), i + 1);
			LoadControl(new GOGUILabel(this, NULL), cfg, panel_prefix + buffer);
		}
	}

	unsigned NumberOfGUIElements = 0;
	if (new_format)
		NumberOfGUIElements = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfGUIElements"), 0, 999, false, 0);
	for (unsigned i = 0; i < NumberOfGUIElements; i++)
	{
		wxString buffer = panel_group + wxString::Format(wxT("Element%03d"), i + 1);
		wxString type = cfg.ReadString(ODFSetting, buffer, wxT("Type"));
		if (type == wxT("Divisional"))
		{
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Manual"), m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			unsigned divisional_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Divisional"), 1, m_organfile->GetManual(manual_nb)->GetDivisionalCount());
			LoadControl(new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetDivisional(divisional_nb - 1), true), cfg, buffer);
		}
		else if (type == wxT("Enclosure"))
		{
			unsigned enclosure_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Enclosure"), 1, m_organfile->GetEnclosureCount());
			LoadControl(new GOGUIEnclosure(this, m_organfile->GetEnclosure(enclosure_nb - 1)), cfg, buffer);
		}
		else if (type == wxT("Tremulant"))
		{
			unsigned tremulant_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Tremulant"), 1, m_organfile->GetTremulantCount());
			LoadControl(new GOGUIButton(this, m_organfile->GetTremulant(tremulant_nb - 1)), cfg, buffer);
		}
		else if (type == wxT("DivisionalCoupler"))
		{
			unsigned coupler_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("DivisionalCoupler"), 1, m_organfile->GetDivisionalCouplerCount());
			LoadControl(new GOGUIButton(this, m_organfile->GetDivisionalCoupler(coupler_nb - 1)), cfg, buffer);
		}
		else if (type == wxT("General"))
		{
			unsigned general_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("General"), 1, m_organfile->GetGeneralCount());
			LoadControl(new GOGUIButton(this, m_organfile->GetGeneral(general_nb - 1), true), cfg, buffer);
		}
		else if (type == wxT("ReversiblePiston"))
		{
			unsigned piston_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("ReversiblePiston"), 1, m_organfile->GetNumberOfReversiblePistons());
			LoadControl(new GOGUIButton(this, m_organfile->GetPiston(piston_nb - 1), true), cfg, buffer);
		}
		else if (type == wxT("Switch"))
		{
			unsigned switch_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Switch"), 1, m_organfile->GetSwitchCount());
			LoadControl(new GOGUIButton(this, m_organfile->GetSwitch(switch_nb - 1), false), cfg, buffer);
		}
		else if (type == wxT("Coupler"))
		{
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Manual"), m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			unsigned coupler_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Coupler"), 1, m_organfile->GetManual(manual_nb)->GetCouplerCount());
			LoadControl(new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetCoupler(coupler_nb - 1)), cfg, buffer);
		}
		else if (type == wxT("Stop"))
		{
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Manual"), m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			unsigned stop_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Stop"), 1, m_organfile->GetManual(manual_nb)->GetStopCount());
			LoadControl(new GOGUIButton(this, m_organfile->GetManual(manual_nb)->GetStop(stop_nb - 1)), cfg, buffer);
		}
		else if (type == wxT("Label"))
		{
			LoadControl(new GOGUILabel(this, NULL), cfg, buffer);
		}
		else if (type == wxT("Manual"))
		{
			unsigned manual_nb  = cfg.ReadInteger(ODFSetting, buffer, wxT("Manual"), m_organfile->GetFirstManualIndex(), m_organfile->GetManualAndPedalCount());
			LoadBackgroundControl(new GOGUIManualBackground(this, m_layout->GetManualNumber()), cfg, buffer);
			LoadControl(new GOGUIManual(this, m_organfile->GetManual(manual_nb), m_layout->GetManualNumber()), cfg, buffer);
		}
		else
		{
			GOGUIControl* control = m_organfile->GetSetter()->CreateGUIElement(cfg, buffer, this);
			if (!control)
				throw (wxString)wxString::Format(_("Unkown SetterElement in section %s"), buffer.c_str());
			LoadControl(control, cfg, buffer);
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

void GOGUIPanel::LoadControl(GOGUIControl* control, GOrgueConfigReader& cfg, wxString group)
{
	control->Load(cfg, group);
	AddControl(control);
}

void GOGUIPanel::LoadBackgroundControl(GOGUIControl* control, GOrgueConfigReader& cfg, wxString group)
{
	control->Load(cfg, group);
	m_controls.insert(m_BackgroundControls, control);
	m_BackgroundControls++;
}

void GOGUIPanel::AddControl(GOGUIControl* control)
{
	m_controls.push_back(control);
}

GOGUIDisplayMetrics* GOGUIPanel::GetDisplayMetrics()
{
	return m_metrics;
}

GOGUILayoutEngine* GOGUIPanel::GetLayoutEngine()
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
