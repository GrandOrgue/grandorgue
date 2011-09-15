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

#include "GOGUILabel.h"
#include "GOGUIHW1Background.h"
#include "GOGUIPanel.h"
#include "GOGUISetterButton.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOrgueFrameGeneral.h"
#include "GOrgueMeter.h"
#include "GOrgueSetter.h"
#include "GOrgueSetterButton.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "IniFileConfig.h"

enum {
	ID_SETTER_PREV = 0,
	ID_SETTER_NEXT,
	ID_SETTER_SET,
	ID_SETTER_M1,
	ID_SETTER_M10,
	ID_SETTER_M100,
	ID_SETTER_P1,
	ID_SETTER_P10,
	ID_SETTER_P100,
	ID_SETTER_CURRENT,
	ID_SETTER_HOME,
	ID_SETTER_L0,
	ID_SETTER_L1,
	ID_SETTER_L2,
	ID_SETTER_L3,
	ID_SETTER_L4,
	ID_SETTER_L5,
	ID_SETTER_L6,
	ID_SETTER_L7,
	ID_SETTER_L8,
	ID_SETTER_L9,
	ID_SETTER_REGULAR,
	ID_SETTER_SCOPE,
	ID_SETTER_SCOPED
};

GOrgueSetter::GOrgueSetter(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_pos(0),
	m_framegeneral(0),
	m_button(0),
	m_PosDisplay(organfile),
	m_SetterType(SETTER_REGULAR)
{
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, false));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	for(unsigned i = 0; i < 10; i++)
		m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	m_button[ID_SETTER_PREV]->GetMidiReceiver().SetIndex(0);
	m_button[ID_SETTER_NEXT]->GetMidiReceiver().SetIndex(1);
	m_button[ID_SETTER_SET]->GetMidiReceiver().SetIndex(15);

	SetSetterType(m_SetterType);
}

GOrgueSetter::~GOrgueSetter()
{
}

GOGUIPanel* GOrgueSetter::CreateSetterPanel(IniFileConfig& cfg)
{
	GOGUIControl* control;
	GOGUISetterButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, wxT("Setter"), GOGUI_SETTER_SETTER);
	panel->Init(cfg, metrics, _("Setter"), wxT("SetterPanel"));

	control = new GOGUIHW1Background(panel);
	panel->AddControl(control);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_PosDisplay);
	PosDisplay->Init(cfg, 350, 10, wxT("SetterCurrentPosition"));
	panel->AddControl(PosDisplay);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_CURRENT]);
	button->Init(cfg, 1, 100, wxT("SetterCurrent"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_M100]);
	button->Init(cfg, 2, 100, wxT("SetterM100"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_M10]);
	button->Init(cfg, 3, 100, wxT("SetterM10"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_M1]);
	button->Init(cfg, 4, 100, wxT("SetterM1"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_PREV]);
	button->Init(cfg, 5, 100, wxT("SetterPrev"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_NEXT]);
	button->Init(cfg, 6, 100, wxT("SetterNext"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_P1]);
	button->Init(cfg, 7, 100, wxT("SetterP1"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_P10]);
	button->Init(cfg, 8, 100, wxT("SetterP10"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_P100]);
	button->Init(cfg, 9, 100, wxT("SetterP100"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_HOME]);
	button->Init(cfg, 10, 100, wxT("SetterHome"));
	panel->AddControl(button);

	for(unsigned i = 0; i < 10; i++)
	{
		button = new GOGUISetterButton(panel, m_button[ID_SETTER_L0 + i]);
		button->Init(cfg, i + 1, 101, wxString::Format(wxT("SetterL%d"), i));
		panel->AddControl(button);
	}

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_SET]);
	button->Init(cfg, 1, 102, wxT("SetterSet"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_REGULAR]);
	button->Init(cfg, 3, 102, wxT("SetterRegular"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_SCOPE]);
	button->Init(cfg, 4, 102, wxT("SetterScope"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_SCOPED]);
	button->Init(cfg, 5, 102, wxT("SetterScoped"));
	panel->AddControl(button);

	return panel;
}

void GOrgueSetter::Load(IniFileConfig& cfg)
{
	wxString buffer;
	unsigned m_NumberOfFrameGenerals = 1000;

	m_framegeneral.resize(0);
	for (unsigned i = 0; i < m_NumberOfFrameGenerals; i++)
	{
		m_framegeneral.push_back(new GOrgueFrameGeneral(m_organfile));
		buffer.Printf(wxT("FrameGeneral%03d"), i + 1);
		m_framegeneral[i]->Load(cfg, buffer);
	}

	m_button[ID_SETTER_PREV]->Load(cfg, wxT("SetterPrev"), _("Previous"));
	m_button[ID_SETTER_NEXT]->Load(cfg, wxT("SetterNext"), _("Next"));
	m_button[ID_SETTER_SET]->Load(cfg, wxT("SetterSet"), _("Set"));
	m_button[ID_SETTER_M1]->Load(cfg, wxT("SetterM1"), _("-1"));
	m_button[ID_SETTER_M10]->Load(cfg, wxT("SetterM10"), _("-10"));
	m_button[ID_SETTER_M100]->Load(cfg, wxT("SetterM100"), _("-100"));
	m_button[ID_SETTER_P1]->Load(cfg, wxT("SetterP1"), _("+1"));
	m_button[ID_SETTER_P10]->Load(cfg, wxT("SetterP10"), _("+10"));
	m_button[ID_SETTER_P100]->Load(cfg, wxT("SetterP100"), _("+100"));
	m_button[ID_SETTER_CURRENT]->Load(cfg, wxT("SetterCurrent"), _("Current"));
	m_button[ID_SETTER_HOME]->Load(cfg, wxT("SetterHome"), _("000"));

	m_button[ID_SETTER_REGULAR]->Load(cfg, wxT("SetterRegular"), _("Regular"));
	m_button[ID_SETTER_SCOPE]->Load(cfg, wxT("SetterScope"), _("Scope"));
	m_button[ID_SETTER_SCOPED]->Load(cfg, wxT("SetterScoped"), _("Scoped"));

	for(unsigned i = 0; i < 10; i++)
	{
		wxString group;
		wxString buffer;
		buffer.Printf(_("__%d"), i);
		group.Printf(wxT("SetterL%d"), i);
		m_button[ID_SETTER_L0 + i]->Load(cfg, group, buffer);
	}
}

void GOrgueSetter::Save(IniFileConfig& cfg, bool prefix)
{
	cfg.SaveHelper(prefix, wxT("Organ"), wxT("NumberOfFrameGenerals"), m_framegeneral.size());

	for (unsigned j = 0; j < m_framegeneral.size(); j++)
		m_framegeneral[j]->Save(cfg, prefix);

	for (unsigned j = 0; j < m_button.size(); j++)
		m_button[j]->Save(cfg, prefix);
}

void GOrgueSetter::ProcessMidi(const GOrgueMidiEvent& event)
{
	for(unsigned i = 0; i < m_button.size(); i++)
		m_button[i]->ProcessMidi(event);
}

void GOrgueSetter::Change(GOrgueSetterButton* button)
{
	for(unsigned i = 0; i < m_button.size(); i++)
		if (m_button[i] == button)
			switch(i)
			{
			case ID_SETTER_PREV:
				Prev();
				break;
			case ID_SETTER_NEXT:
				Next();
				break;
			case ID_SETTER_SET:
				::wxGetApp().frame->UpdateWindowUI();
				break;
			case ID_SETTER_M1:
				SetPosition(m_pos - 1, false);
				break;
			case ID_SETTER_M10:
				SetPosition(m_pos - 10, false);
				break;
			case ID_SETTER_M100:
				SetPosition(m_pos - 100, false);
				break;
			case ID_SETTER_P1:
				SetPosition(m_pos + 1, false);
				break;
			case ID_SETTER_P10:
				SetPosition(m_pos + 10, false);
				break;
			case ID_SETTER_P100:
				SetPosition(m_pos + 100, false);
				break;
			case ID_SETTER_HOME:
				SetPosition(0, false);
				break;
			case ID_SETTER_CURRENT:
				SetPosition(m_pos);
				break;
			case ID_SETTER_L0:
			case ID_SETTER_L1:
			case ID_SETTER_L2:
			case ID_SETTER_L3:
			case ID_SETTER_L4:
			case ID_SETTER_L5:
			case ID_SETTER_L6:
			case ID_SETTER_L7:
			case ID_SETTER_L8:
			case ID_SETTER_L9:
				SetPosition(m_pos - (m_pos % 10) + i - ID_SETTER_L0);
				break;
			case ID_SETTER_REGULAR:
				SetSetterType(SETTER_REGULAR);
				break;
			case ID_SETTER_SCOPE:
				SetSetterType(SETTER_SCOPE);
				break;
			case ID_SETTER_SCOPED:
				SetSetterType(SETTER_SCOPED);
				break;
			}
}

void GOrgueSetter::PreparePlayback()
{
	wxString buffer;
	buffer.Printf(wxT("%03d"), m_pos);
	m_PosDisplay.SetName(buffer);
	::wxGetApp().frame->m_meters[2]->ChangeValue(m_pos);
}

bool GOrgueSetter::IsSetterActive()
{
	return m_button[ID_SETTER_SET]->IsEngaged();
}

bool GOrgueSetter::StoreInvisibleObjects()
{
	return false;
}

void GOrgueSetter::SetterActive(bool on)
{
	m_button[ID_SETTER_SET]->Set(on);
}

void GOrgueSetter::ToggleSetter()
{
	m_button[ID_SETTER_SET]->Push();
}

void GOrgueSetter::Next()
{
	SetPosition(m_pos + 1);
}

void GOrgueSetter::Prev()
{
	SetPosition(m_pos - 1);
}

void GOrgueSetter::Push()
{
	SetPosition(m_pos);
}

unsigned GOrgueSetter::GetPosition()
{
	return m_pos;
}

SetterType GOrgueSetter::GetSetterType()
{
	return m_SetterType;
}

void GOrgueSetter::SetSetterType(SetterType type)
{
	m_SetterType = type;
	m_button[ID_SETTER_REGULAR]->Display(type == SETTER_REGULAR);
	m_button[ID_SETTER_SCOPE]->Display(type == SETTER_SCOPE);
	m_button[ID_SETTER_SCOPED]->Display(type == SETTER_SCOPED);
}

void GOrgueSetter::SetPosition(int pos, bool push)
{
	wxString buffer;
	int old_pos = m_pos;
	while (pos < 0)
		pos += m_framegeneral.size();
	while (pos >= (int)m_framegeneral.size())
		pos -= m_framegeneral.size();
	m_pos = pos;
	if (push)
		m_framegeneral[m_pos]->Push();

	buffer.Printf(wxT("%03d"), m_pos);
	m_PosDisplay.SetName(buffer);
	if (pos != old_pos)
		::wxGetApp().frame->m_meters[2]->ChangeValue(m_pos);
}


