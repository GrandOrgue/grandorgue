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
	ID_SETTER_SCOPED,
	ID_SETTER_FULL,
	ID_SETTER_DELETE,
	ID_SETTER_INSERT,

	ID_SETTER_GENERAL00,
	ID_SETTER_GENERAL01,
	ID_SETTER_GENERAL02,
	ID_SETTER_GENERAL03,
	ID_SETTER_GENERAL04,
	ID_SETTER_GENERAL05,
	ID_SETTER_GENERAL06,
	ID_SETTER_GENERAL07,
	ID_SETTER_GENERAL08,
	ID_SETTER_GENERAL09,
	ID_SETTER_GENERAL10,
	ID_SETTER_GENERAL11,
	ID_SETTER_GENERAL12,
	ID_SETTER_GENERAL13,
	ID_SETTER_GENERAL14,
	ID_SETTER_GENERAL15,
	ID_SETTER_GENERAL16,
	ID_SETTER_GENERAL17,
	ID_SETTER_GENERAL18,
	ID_SETTER_GENERAL19,
	ID_SETTER_GENERAL20,
	ID_SETTER_GENERAL21,
	ID_SETTER_GENERAL22,
	ID_SETTER_GENERAL23,
	ID_SETTER_GENERAL24,
	ID_SETTER_GENERAL25,
	ID_SETTER_GENERAL26,
	ID_SETTER_GENERAL27,
	ID_SETTER_GENERAL28,
	ID_SETTER_GENERAL29,

};

GOrgueSetter::GOrgueSetter(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_pos(0),
	m_framegeneral(0),
	m_general(0),
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
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, false));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	for(unsigned i = 0; i < 30; i++)
		m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	m_button[ID_SETTER_PREV]->GetMidiReceiver().SetIndex(0);
	m_button[ID_SETTER_NEXT]->GetMidiReceiver().SetIndex(1);
	m_button[ID_SETTER_SET]->GetMidiReceiver().SetIndex(15);

	SetSetterType(m_SetterType);
}

GOrgueSetter::~GOrgueSetter()
{
}

GOGUIPanel* GOrgueSetter::CreateGeneralsPanel(IniFileConfig& cfg)
{
	GOGUIControl* control;
	GOGUISetterButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, wxT("SetterGenerals"), GOGUI_SETTER_GENERALS);
	panel->Init(cfg, metrics, _("Generals"), wxT("SetterGeneralsPanel"));

	control = new GOGUIHW1Background(panel);
	panel->AddControl(control);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_SET]);
	button->Init(cfg, 1, 100, wxT("SetterGeneralsSet"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_REGULAR]);
	button->Init(cfg, 3, 100, wxT("SetterGerneralsRegular"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_SCOPE]);
	button->Init(cfg, 4, 100, wxT("SetterGeneralsScope"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_SCOPED]);
	button->Init(cfg, 5, 100, wxT("SetterGeneralsScoped"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_FULL]);
	button->Init(cfg, 7, 100, wxT("SetterGeneralsFull"));
	panel->AddControl(button);

	for(unsigned i = 0; i < 30; i++)
	{
		button = new GOGUISetterButton(panel, m_button[ID_SETTER_GENERAL00 + i]);
		button->Init(cfg, (i % 10) + 1, 100 + i / 10, wxString::Format(wxT("SetterGeneral%d"), i + 1), true);
		panel->AddControl(button);
	}

	return panel;
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

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_FULL]);
	button->Init(cfg, 7, 102, wxT("SetterFull"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_INSERT]);
	button->Init(cfg, 9, 102, wxT("SetterInsert"));
	panel->AddControl(button);

	button = new GOGUISetterButton(panel, m_button[ID_SETTER_DELETE]);
	button->Init(cfg, 10, 102, wxT("SetterDelete"));
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

	m_general.resize(0);
	for (unsigned i = 0; i < m_NumberOfFrameGenerals; i++)
	{
		m_general.push_back(new GOrgueFrameGeneral(m_organfile));
		buffer.Printf(wxT("SetterGeneral%03d"), i + 1);
		m_general[i]->Load(cfg, buffer);
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
	m_button[ID_SETTER_FULL]->Load(cfg, wxT("SetterFull"), _("Full"));

	m_button[ID_SETTER_INSERT]->Load(cfg, wxT("SetterInsert"), _("Insert"));
	m_button[ID_SETTER_DELETE]->Load(cfg, wxT("SetterDelete"), _("Delete"));

	for(unsigned i = 0; i < 10; i++)
	{
		wxString group;
		wxString buffer;
		buffer.Printf(_("__%d"), i);
		group.Printf(wxT("SetterL%d"), i);
		m_button[ID_SETTER_L0 + i]->Load(cfg, group, buffer);
	}

	for(unsigned i = 0; i < 30; i++)
	{
		wxString group;
		wxString buffer;
		buffer.Printf(_("%d"), i+1);
		group.Printf(wxT("SetterGeneral%d"), i);
		m_button[ID_SETTER_GENERAL00 + i]->Load(cfg, group, buffer);
	}
}

void GOrgueSetter::Save(IniFileConfig& cfg, bool prefix)
{
	cfg.SaveHelper(prefix, wxT("Organ"), wxT("NumberOfFrameGenerals"), m_framegeneral.size());

	for (unsigned j = 0; j < m_framegeneral.size(); j++)
		m_framegeneral[j]->Save(cfg, prefix);

	for (unsigned j = 0; j < m_general.size(); j++)
		m_general[j]->Save(cfg, prefix);

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
			case ID_SETTER_DELETE:
				for(unsigned j = m_pos; j < m_framegeneral.size() - 1; j++)
					m_framegeneral[j]->Copy(m_framegeneral[j + 1]);
				ResetDisplay();
				break;
			case ID_SETTER_INSERT:
				for (unsigned j = m_framegeneral.size() - 1; j > m_pos; j--)
					m_framegeneral[j]->Copy(m_framegeneral[j - 1]);
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
			case ID_SETTER_GENERAL00:
			case ID_SETTER_GENERAL01:
			case ID_SETTER_GENERAL02:
			case ID_SETTER_GENERAL03:
			case ID_SETTER_GENERAL04:
			case ID_SETTER_GENERAL05:
			case ID_SETTER_GENERAL06:
			case ID_SETTER_GENERAL07:
			case ID_SETTER_GENERAL08:
			case ID_SETTER_GENERAL09:
			case ID_SETTER_GENERAL10:
			case ID_SETTER_GENERAL11:
			case ID_SETTER_GENERAL12:
			case ID_SETTER_GENERAL13:
			case ID_SETTER_GENERAL14:
			case ID_SETTER_GENERAL15:
			case ID_SETTER_GENERAL16:
			case ID_SETTER_GENERAL17:
			case ID_SETTER_GENERAL18:
			case ID_SETTER_GENERAL19:
			case ID_SETTER_GENERAL20:
			case ID_SETTER_GENERAL21:
			case ID_SETTER_GENERAL22:
			case ID_SETTER_GENERAL23:
			case ID_SETTER_GENERAL24:
			case ID_SETTER_GENERAL25:
			case ID_SETTER_GENERAL26:
			case ID_SETTER_GENERAL27:
			case ID_SETTER_GENERAL28:
			case ID_SETTER_GENERAL29:
				m_general[i - ID_SETTER_GENERAL00]->Push();
				ResetDisplay();
				m_button[i]->Display(true);
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
	return m_button[ID_SETTER_FULL]->IsEngaged();
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

void GOrgueSetter::ResetDisplay()
{
	m_button[ID_SETTER_HOME]->Display(false);
	for(unsigned i = 0; i < 10; i++)
		m_button[ID_SETTER_L0 + i]->Display(false);
	for(unsigned i = 0; i < 30; i++)
		m_button[ID_SETTER_GENERAL00 + i]->Display(false);
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
	{
		m_framegeneral[m_pos]->Push();

		m_button[ID_SETTER_HOME]->Display(m_pos == 0);
		for(unsigned i = 0; i < 10; i++)
			m_button[ID_SETTER_L0 + i]->Display((m_pos % 10) == i);
		for(unsigned i = 0; i < 30; i++)
			m_button[ID_SETTER_GENERAL00 + i]->Display(false);
	}

	buffer.Printf(wxT("%03d"), m_pos);
	m_PosDisplay.SetName(buffer);
	if (pos != old_pos)
		::wxGetApp().frame->m_meters[2]->ChangeValue(m_pos);

}


