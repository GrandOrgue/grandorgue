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

#include "GOrgueSetter.h"

#include "GOGUIButton.h"
#include "GOGUIEnclosure.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrgueEvent.h"
#include "GOrgueFrameGeneral.h"
#include "GOrgueManual.h"
#include "GOrgueSetterButton.h"
#include "GOrgueSettings.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueID.h"
#include <wx/app.h>
#include <wx/intl.h>
#include <wx/window.h>

#define FRAME_GENERALS 1000
#define GENERAL_BANKS 10
#define GENERALS 50
#define CRESCENDO_STEPS 32

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
	ID_SETTER_GC,
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
	ID_SETTER_GENERAL30,
	ID_SETTER_GENERAL31,
	ID_SETTER_GENERAL32,
	ID_SETTER_GENERAL33,
	ID_SETTER_GENERAL34,
	ID_SETTER_GENERAL35,
	ID_SETTER_GENERAL36,
	ID_SETTER_GENERAL37,
	ID_SETTER_GENERAL38,
	ID_SETTER_GENERAL39,
	ID_SETTER_GENERAL40,
	ID_SETTER_GENERAL41,
	ID_SETTER_GENERAL42,
	ID_SETTER_GENERAL43,
	ID_SETTER_GENERAL44,
	ID_SETTER_GENERAL45,
	ID_SETTER_GENERAL46,
	ID_SETTER_GENERAL47,
	ID_SETTER_GENERAL48,
	ID_SETTER_GENERAL49,
	ID_SETTER_GENERAL_PREV,
	ID_SETTER_GENERAL_NEXT,

	ID_SETTER_CRESCENDO_PREV,
	ID_SETTER_CRESCENDO_CURRENT,
	ID_SETTER_CRESCENDO_NEXT,
	ID_SETTER_CRESCENDO_A,
	ID_SETTER_CRESCENDO_B,
	ID_SETTER_CRESCENDO_C,
	ID_SETTER_CRESCENDO_D,

	ID_SETTER_TEMPERAMENT_PREV,
	ID_SETTER_TEMPERAMENT_NEXT,
	ID_SETTER_PITCH_M1,
	ID_SETTER_PITCH_M10,
	ID_SETTER_PITCH_M100,
	ID_SETTER_PITCH_P1,
	ID_SETTER_PITCH_P10,
	ID_SETTER_PITCH_P100,
	ID_SETTER_TRANSPOSE_DOWN,
	ID_SETTER_TRANSPOSE_UP,

	ID_SETTER_SAVE,

	ID_SETTER_LABEL, /* Must be the last elements */
	ID_SETTER_GENERAL_LABEL,
	ID_SETTER_CRESCENDO_LABEL,
	ID_SETTER_TEMPERAMENT_LABEL,
	ID_SETTER_PITCH_LABEL,
	ID_SETTER_TRANSPOSE_LABEL,
	ID_SETTER_CRESCENDO_SWELL,
};

const struct IniFileEnumEntry GOrgueSetter::m_setter_element_types[] = {
	{ wxT("Prev"), ID_SETTER_PREV },
	{ wxT("Next"), ID_SETTER_NEXT },
	{ wxT("Set"), ID_SETTER_SET },
	{ wxT("M1"), ID_SETTER_M1 },
	{ wxT("M10"), ID_SETTER_M10 },
	{ wxT("M100"), ID_SETTER_M100 },
	{ wxT("P1"), ID_SETTER_P1 },
	{ wxT("P10"), ID_SETTER_P10 },
	{ wxT("P100"), ID_SETTER_P100 },
	{ wxT("Current"), ID_SETTER_CURRENT },
	{ wxT("Home"), ID_SETTER_HOME },
	{ wxT("GC"), ID_SETTER_GC },
	{ wxT("L1"), ID_SETTER_L1 },
	{ wxT("L2"), ID_SETTER_L2 },
	{ wxT("L3"), ID_SETTER_L3 },
	{ wxT("L4"), ID_SETTER_L4 },
	{ wxT("L5"), ID_SETTER_L5 },
	{ wxT("L6"), ID_SETTER_L6 },
	{ wxT("L7"), ID_SETTER_L7 },
	{ wxT("L8"), ID_SETTER_L8 },
	{ wxT("L9"), ID_SETTER_L9 },
	{ wxT("L0"), ID_SETTER_L0 },
	{ wxT("Regular"), ID_SETTER_REGULAR },
	{ wxT("Scope"), ID_SETTER_SCOPE },
	{ wxT("Scoped"), ID_SETTER_SCOPED },
	{ wxT("Label"), ID_SETTER_LABEL },
	{ wxT("Full"), ID_SETTER_FULL },
	{ wxT("Insert"), ID_SETTER_INSERT },
	{ wxT("Delete"), ID_SETTER_DELETE },
	{ wxT("General01"), ID_SETTER_GENERAL00 },
	{ wxT("General02"), ID_SETTER_GENERAL01 },
	{ wxT("General03"), ID_SETTER_GENERAL02 },
	{ wxT("General04"), ID_SETTER_GENERAL03 },
	{ wxT("General05"), ID_SETTER_GENERAL04 },
	{ wxT("General06"), ID_SETTER_GENERAL05 },
	{ wxT("General07"), ID_SETTER_GENERAL06 },
	{ wxT("General08"), ID_SETTER_GENERAL07 },
	{ wxT("General09"), ID_SETTER_GENERAL08 },
	{ wxT("General10"), ID_SETTER_GENERAL09 },
	{ wxT("General11"), ID_SETTER_GENERAL10 },
	{ wxT("General12"), ID_SETTER_GENERAL11 },
	{ wxT("General13"), ID_SETTER_GENERAL12 },
	{ wxT("General14"), ID_SETTER_GENERAL13 },
	{ wxT("General15"), ID_SETTER_GENERAL14 },
	{ wxT("General16"), ID_SETTER_GENERAL15 },
	{ wxT("General17"), ID_SETTER_GENERAL16 },
	{ wxT("General18"), ID_SETTER_GENERAL17 },
	{ wxT("General19"), ID_SETTER_GENERAL18 },
	{ wxT("General20"), ID_SETTER_GENERAL19 },
	{ wxT("General21"), ID_SETTER_GENERAL20 },
	{ wxT("General22"), ID_SETTER_GENERAL21 },
	{ wxT("General23"), ID_SETTER_GENERAL22 },
	{ wxT("General24"), ID_SETTER_GENERAL23 },
	{ wxT("General25"), ID_SETTER_GENERAL24 },
	{ wxT("General26"), ID_SETTER_GENERAL25 },
	{ wxT("General27"), ID_SETTER_GENERAL26 },
	{ wxT("General28"), ID_SETTER_GENERAL27 },
	{ wxT("General29"), ID_SETTER_GENERAL28 },
	{ wxT("General30"), ID_SETTER_GENERAL29 },
	{ wxT("General31"), ID_SETTER_GENERAL30 },
	{ wxT("General32"), ID_SETTER_GENERAL31 },
	{ wxT("General33"), ID_SETTER_GENERAL32 },
	{ wxT("General34"), ID_SETTER_GENERAL33 },
	{ wxT("General35"), ID_SETTER_GENERAL34 },
	{ wxT("General36"), ID_SETTER_GENERAL35 },
	{ wxT("General37"), ID_SETTER_GENERAL36 },
	{ wxT("General38"), ID_SETTER_GENERAL37 },
	{ wxT("General39"), ID_SETTER_GENERAL38 },
	{ wxT("General40"), ID_SETTER_GENERAL39 },
	{ wxT("General41"), ID_SETTER_GENERAL40 },
	{ wxT("General42"), ID_SETTER_GENERAL41 },
	{ wxT("General43"), ID_SETTER_GENERAL42 },
	{ wxT("General44"), ID_SETTER_GENERAL43 },
	{ wxT("General45"), ID_SETTER_GENERAL44 },
	{ wxT("General46"), ID_SETTER_GENERAL45 },
	{ wxT("General47"), ID_SETTER_GENERAL46 },
	{ wxT("General48"), ID_SETTER_GENERAL47 },
	{ wxT("General49"), ID_SETTER_GENERAL48 },
	{ wxT("General50"), ID_SETTER_GENERAL49 },

	{ wxT("GeneralPrev"), ID_SETTER_GENERAL_PREV },
	{ wxT("GeneralNext"), ID_SETTER_GENERAL_NEXT },

	{ wxT("PitchP1"), ID_SETTER_PITCH_P1 },
	{ wxT("PitchP10"), ID_SETTER_PITCH_P10 },
	{ wxT("PitchP100"), ID_SETTER_PITCH_P100 },
	{ wxT("PitchM1"), ID_SETTER_PITCH_M1 },
	{ wxT("PitchM10"), ID_SETTER_PITCH_M10 },
	{ wxT("PitchM100"), ID_SETTER_PITCH_M100 },
	{ wxT("TemperamentPrev"), ID_SETTER_TEMPERAMENT_PREV },
	{ wxT("TemperamentNext"), ID_SETTER_TEMPERAMENT_NEXT },
	{ wxT("TransposeDown"), ID_SETTER_TRANSPOSE_DOWN },
	{ wxT("TransposeUp"), ID_SETTER_TRANSPOSE_UP },

	{ wxT("Save"), ID_SETTER_SAVE },

	{ wxT("CrescendoA"), ID_SETTER_CRESCENDO_A },
	{ wxT("CrescendoB"), ID_SETTER_CRESCENDO_B },
	{ wxT("CrescendoC"), ID_SETTER_CRESCENDO_C },
	{ wxT("CrescendoD"), ID_SETTER_CRESCENDO_D },
	{ wxT("CrescendoPrev"), ID_SETTER_CRESCENDO_PREV },
	{ wxT("CrescendoCurrent"), ID_SETTER_CRESCENDO_CURRENT },
	{ wxT("CrescendoNext"), ID_SETTER_CRESCENDO_NEXT },
	{ wxT("Swell"), ID_SETTER_CRESCENDO_SWELL },
	{ wxT("CrescendoLabel"), ID_SETTER_CRESCENDO_LABEL },
	{ wxT("PitchLabel"), ID_SETTER_PITCH_LABEL },
	{ wxT("GeneralLabel"), ID_SETTER_GENERAL_LABEL },
	{ wxT("TemperamentLabel"), ID_SETTER_TEMPERAMENT_LABEL },
	{ wxT("TransposeLabel"), ID_SETTER_TRANSPOSE_LABEL },

};

GOrgueSetter::GOrgueSetter(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_pos(0),
	m_bank(0),
	m_crescendopos(0),
	m_crescendobank(0),
	m_framegeneral(0),
	m_general(0),
	m_crescendo(0),
	m_button(0),
	m_PosDisplay(organfile),
	m_BankDisplay(organfile),
	m_CrescendoDisplay(organfile),
	m_TransposeDisplay(organfile),
	m_swell(organfile),
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
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	for(unsigned i = 0; i < 10; i++)
		m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, false));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	/* Generals */
	for(unsigned i = 0; i < GENERALS; i++)
		m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	/* Crescendo */
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	/* Pitch */
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	m_button[ID_SETTER_PREV]->GetMidiReceiver().SetIndex(0);
	m_button[ID_SETTER_NEXT]->GetMidiReceiver().SetIndex(1);
	m_button[ID_SETTER_SET]->GetMidiReceiver().SetIndex(2);
	m_button[ID_SETTER_CURRENT]->GetMidiReceiver().SetIndex(3);
	m_button[ID_SETTER_GC]->GetMidiReceiver().SetIndex(4);
	m_button[ID_SETTER_M10]->GetMidiReceiver().SetIndex(5);
	m_button[ID_SETTER_P10]->GetMidiReceiver().SetIndex(6);
	m_button[ID_SETTER_L0]->GetMidiReceiver().SetIndex(7);
	m_button[ID_SETTER_L1]->GetMidiReceiver().SetIndex(8);
	m_button[ID_SETTER_L2]->GetMidiReceiver().SetIndex(9);
	m_button[ID_SETTER_L3]->GetMidiReceiver().SetIndex(10);
	m_button[ID_SETTER_L4]->GetMidiReceiver().SetIndex(11);
	m_button[ID_SETTER_L5]->GetMidiReceiver().SetIndex(12);
	m_button[ID_SETTER_L6]->GetMidiReceiver().SetIndex(13);
	m_button[ID_SETTER_L7]->GetMidiReceiver().SetIndex(14);
	m_button[ID_SETTER_L8]->GetMidiReceiver().SetIndex(15);
	m_button[ID_SETTER_L9]->GetMidiReceiver().SetIndex(16);
	m_button[ID_SETTER_PITCH_M1]->GetMidiReceiver().SetIndex(17);
	m_button[ID_SETTER_PITCH_P1]->GetMidiReceiver().SetIndex(18);
	m_button[ID_SETTER_PITCH_M100]->GetMidiReceiver().SetIndex(19);
	m_button[ID_SETTER_PITCH_P100]->GetMidiReceiver().SetIndex(20);
	m_button[ID_SETTER_TEMPERAMENT_PREV]->GetMidiReceiver().SetIndex(21);
	m_button[ID_SETTER_TEMPERAMENT_NEXT]->GetMidiReceiver().SetIndex(22);
	m_button[ID_SETTER_TRANSPOSE_DOWN]->GetMidiReceiver().SetIndex(23);
	m_button[ID_SETTER_TRANSPOSE_UP]->GetMidiReceiver().SetIndex(24);

	m_button[ID_SETTER_PREV]->GetKeyReceiver().SetShortcut(37);
	m_button[ID_SETTER_NEXT]->GetKeyReceiver().SetShortcut(39);
	m_button[ID_SETTER_CURRENT]->GetKeyReceiver().SetShortcut(40);

	SetSetterType(m_SetterType);
	SetCrescendoType(m_crescendobank);
}

GOrgueSetter::~GOrgueSetter()
{
}


GOGUIControl* GOrgueSetter::CreateGUIElement(GOrgueConfigReader& cfg, wxString group, GOGUIPanel* panel)
{
	unsigned element  = cfg.ReadEnum(ODFSetting, group, wxT("Type"), m_setter_element_types, sizeof(m_setter_element_types) / sizeof(m_setter_element_types[0]), true);
	if (element == ID_SETTER_LABEL)
	{
		GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_PosDisplay);
		PosDisplay->Load(cfg, group);
		return PosDisplay;
	}
	if (element == ID_SETTER_CRESCENDO_LABEL)
	{
		GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_CrescendoDisplay);
		PosDisplay->Load(cfg, group);
		return PosDisplay;
	}
	if (element == ID_SETTER_GENERAL_LABEL)
	{
		GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_BankDisplay);
		PosDisplay->Load(cfg, group);
		return PosDisplay;
	}
	if (element == ID_SETTER_PITCH_LABEL)
	{
		GOGUILabel* PosDisplay=new GOGUILabel(panel, m_organfile->GetPitchLabel());
		PosDisplay->Load(cfg, group);
		return PosDisplay;
	}
	if (element == ID_SETTER_TEMPERAMENT_LABEL)
	{
		GOGUILabel* PosDisplay=new GOGUILabel(panel, m_organfile->GetTemperamentLabel());
		PosDisplay->Load(cfg, group);
		return PosDisplay;
	}
	if (element == ID_SETTER_TRANSPOSE_LABEL)
	{
		GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_TransposeDisplay);
		PosDisplay->Load(cfg, group);
		return PosDisplay;
	}
	if (element == ID_SETTER_CRESCENDO_SWELL)
	{
		GOGUIEnclosure* enclosure = new GOGUIEnclosure(panel, &m_swell);
		enclosure->Load(cfg, group);
		return enclosure;
	}

	GOGUIButton* button = new GOGUIButton(panel, m_button[element], false);
	button->Load(cfg, group);
	return button;
}

GOGUIPanel* GOrgueSetter::CreateMasterPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_MASTER);
	panel->Init(cfg, metrics, _("Master Controls"), wxT("SetterMaster"), wxT(""));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterMaster"));
	panel->AddControl(back);

	button = new GOGUIButton(panel, m_button[ID_SETTER_PITCH_M100], false);
	button->Init(cfg, wxT("SetterMasterPitchM100"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_PITCH_M10], false);
	button->Init(cfg, wxT("SetterMasterPitchM10"), 2, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_PITCH_M1], false);
	button->Init(cfg, wxT("SetterMasterPitchM1"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_PITCH_P1], false);
	button->Init(cfg, wxT("SetterMasterPitchP1"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_PITCH_P10], false);
	button->Init(cfg, wxT("SetterMasterPitchP10"), 6, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_PITCH_P100], false);
	button->Init(cfg, wxT("SetterMasterPitchP10"), 7, 100);
	panel->AddControl(button);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, m_organfile->GetPitchLabel());
	PosDisplay->Init(cfg, wxT("SetterMasterPitch"), 230, 25);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_button[ID_SETTER_TEMPERAMENT_PREV], false);
	button->Init(cfg, wxT("SetterMasterTemperamentPrev"), 1, 101);
	panel->AddControl(button);

	PosDisplay=new GOGUILabel(panel, m_organfile->GetTemperamentLabel());
	PosDisplay->Init(cfg, wxT("SetterMasterTemperament"), 80, 80, wxEmptyString, 2);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_button[ID_SETTER_TEMPERAMENT_NEXT], false);
	button->Init(cfg, wxT("SetterMasterTemperamentNext"), 3, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SAVE], false);
	button->Init(cfg, wxT("SetterSave"), 5, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_TRANSPOSE_DOWN], false);
	button->Init(cfg, wxT("SetterMasterTransposeDown"), 1, 102);
	panel->AddControl(button);

	PosDisplay=new GOGUILabel(panel, &m_TransposeDisplay);
	PosDisplay->Init(cfg, wxT("SetterMasterTranspose"), 80, 165);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_button[ID_SETTER_TRANSPOSE_UP], false);
	button->Init(cfg, wxT("SetterMasterTransposeUp"), 3, 102);
	panel->AddControl(button);

	return panel;
}

GOGUIPanel* GOrgueSetter::CreateFloatingPanel(GOrgueConfigReader& cfg)
{
	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_FLOATING);
	panel->Init(cfg, metrics, _("Floating manuals"), wxT("SetterFloating"), wxT(""));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterFloating"));
	panel->AddControl(back);

	for (unsigned i = m_organfile->GetODFManualCount(); i <= m_organfile->GetManualAndPedalCount(); i++)
	{
		wxString group;
		group.Printf(wxT("SetterFloating%03d"), i - m_organfile->GetODFManualCount() + 1);
		GOGUIManualBackground* manual_back = new GOGUIManualBackground(panel, i - m_organfile->GetODFManualCount());
		manual_back->Init(cfg, group);
		panel->AddControl(manual_back);

		GOGUIManual* manual = new GOGUIManual(panel, m_organfile->GetManual(i), i - m_organfile->GetODFManualCount());
		manual->Init(cfg, group);
		panel->AddControl(manual);

		for(unsigned j = 0; j < 10; j++)
		{
			GOrgueDivisional* divisional = new GOrgueDivisional(m_organfile, m_organfile->GetManual(i)->GetDivisionalTemplate(), true);
			divisional->Init(cfg, wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100), i, 100 + j, wxString::Format(wxT("%d"), j + 1));
			m_organfile->GetManual(i)->AddDivisional(divisional);

			GOGUIButton* button = new GOGUIButton(panel, divisional, true);
			button->Init(cfg, wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100), j + 1, i - m_organfile->GetODFManualCount());
			panel->AddControl(button);
		}

	}

	GOrgueEnclosure* master_enc = new GOrgueEnclosure(m_organfile);
	master_enc->Init(cfg, wxT("SetterMasterVolume"), _("Master"));
	master_enc->Set(127);
	m_organfile->AddEnclosure(master_enc);
	master_enc->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("SM"))));

	GOGUIEnclosure* enclosure = new GOGUIEnclosure(panel, master_enc);
	enclosure->Init(cfg, wxT("SetterMasterVolume"));
	panel->AddControl(enclosure);

	for(unsigned i = 0; i < m_organfile->GetWindchestGroupCount(); i++)
	{
		GOrgueWindchest* windchest = m_organfile->GetWindchest(i);
		windchest->AddEnclosure(master_enc);

		GOrgueEnclosure* enc = new GOrgueEnclosure(m_organfile);
		enc->Init(cfg, wxString::Format(wxT("SetterMaster%03d"), i + 1), windchest->GetName());
		enc->Set(127);
		m_organfile->AddEnclosure(enc);
		enc->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("SM%d"), i)));
		windchest->AddEnclosure(enc);

		enclosure = new GOGUIEnclosure(panel, enc);
		enclosure->Init(cfg, wxString::Format(wxT("SetterMaster%03d"), i + 1));
		panel->AddControl(enclosure);
	}

	return panel;
}

GOGUIPanel* GOrgueSetter::CreateCouplerPanel(GOrgueConfigReader& cfg, unsigned manual_nr)
{
	GOrgueManual* manual = m_organfile->GetManual(manual_nr);

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_COUPLER);
	panel->Init(cfg, metrics, wxString::Format(_("Coupler %s"), manual->GetName().c_str()), wxString::Format(wxT("SetterCouplers%03d"), manual_nr), _("Coupler"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxString::Format(wxT("SetterCouplers%03d"), manual_nr));
	panel->AddControl(back);

	panel->GetLayoutEngine()->Update();
	for (unsigned int i = m_organfile->GetFirstManualIndex(); i < m_organfile->GetODFManualCount(); i++)
	{
		int x, y;
		GOrgueManual* dest_manual = m_organfile->GetManual(i);
		GOrgueCoupler* coupler;
		GOGUIButton* button;

		panel->GetLayoutEngine()->GetDrawstopBlitPosition(100 + i, 1, x, y);

		GOGUILabel* PosDisplay=new GOGUILabel(panel, NULL);
		PosDisplay->Init(cfg, wxString::Format(wxT("SetterCoupler%03dLabel%03d"), manual_nr, i), x, y, dest_manual->GetName());
		panel->AddControl(PosDisplay);

		coupler = new GOrgueCoupler(m_organfile, manual_nr);
		coupler->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dT16"), manual_nr, i), _("16"), false, false, -12, i, GOrgueCoupler::COUPLER_NORMAL);
		coupler->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("S%dM%dC16"), manual_nr, i)));
		manual->AddCoupler(coupler);
		button = new GOGUIButton(panel, coupler, false);
		button->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dT16"), manual_nr, i), 2, 100 + i);
		panel->AddControl(button);

		coupler = new GOrgueCoupler(m_organfile, manual_nr);
		coupler->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dT8"), manual_nr, i), manual_nr != i ? _("8") : _("U.O."), manual_nr == i, false, 0, i, GOrgueCoupler::COUPLER_NORMAL);
		coupler->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("S%dM%dC8"), manual_nr, i)));
		manual->AddCoupler(coupler);
		button = new GOGUIButton(panel, coupler, false);
		button->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dT8"), manual_nr, i), 3, 100 + i);
		panel->AddControl(button);

		coupler = new GOrgueCoupler(m_organfile, manual_nr);
		coupler->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dT4"), manual_nr, i), _("4"), false, false, 12, i, GOrgueCoupler::COUPLER_NORMAL);
		coupler->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("S%dM%dC4"), manual_nr, i)));
		manual->AddCoupler(coupler);
		button = new GOGUIButton(panel, coupler, false);
		button->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dT4"), manual_nr, i), 4, 100 + i);
		panel->AddControl(button);

		coupler = new GOrgueCoupler(m_organfile, manual_nr);
		coupler->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dBAS"), manual_nr, i), _("BAS"), false, false, 0, i, GOrgueCoupler::COUPLER_BASS);
		coupler->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("S%dM%dCB"), manual_nr, i)));
		manual->AddCoupler(coupler);
		button = new GOGUIButton(panel, coupler, false);
		button->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dBAS"), manual_nr, i), 5, 100 + i);
		panel->AddControl(button);

		coupler = new GOrgueCoupler(m_organfile, manual_nr);
		coupler->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dMEL"), manual_nr, i), _("MEL"), false, false, 0, i, GOrgueCoupler::COUPLER_MELODY);
		coupler->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("S%dM%dCM"), manual_nr, i)));
		manual->AddCoupler(coupler);
		button = new GOGUIButton(panel, coupler, false);
		button->Init(cfg, wxString::Format(wxT("SetterManual%03dCoupler%03dMEL"), manual_nr, i), 6, 100 + i);
		panel->AddControl(button);
	}

	return panel;
}


GOGUIPanel* GOrgueSetter::CreateDivisionalPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_DIVISIONALS);
	panel->Init(cfg, metrics, _("Divisionals"), wxT("SetterDivisionalPanel"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterDivisionals"));
	panel->AddControl(back);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SET], false);
	button->Init(cfg, wxT("SetterGeneralsSet"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_REGULAR], false);
	button->Init(cfg, wxT("SetterGerneralsRegular"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPE], false);
	button->Init(cfg, wxT("SetterGeneralsScope"), 4, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPED], false);
	button->Init(cfg, wxT("SetterGeneralsScoped"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_FULL], false);
	button->Init(cfg, wxT("SetterGeneralsFull"), 7, 100);
	panel->AddControl(button);

	panel->GetLayoutEngine()->Update();
	for (unsigned int i = m_organfile->GetFirstManualIndex(); i < m_organfile->GetODFManualCount(); i++)
	{
		int x, y;
		GOrgueManual* manual = m_organfile->GetManual(i);

		panel->GetLayoutEngine()->GetPushbuttonBlitPosition(100 + i, 1, x, y);

		GOGUILabel* PosDisplay=new GOGUILabel(panel, NULL);
		PosDisplay->Init(cfg, wxString::Format(wxT("SetterDivisionalLabel%03d"), i), x, y, manual->GetName());
		panel->AddControl(PosDisplay);

		for(unsigned j = 0; j < 10; j++)
		{
			GOrgueDivisional* divisional = new GOrgueDivisional(m_organfile, manual->GetDivisionalTemplate(), true);
			divisional->Init(cfg, wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100), i, 100 + j, wxString::Format(wxT("%d"), j + 1));
			manual->AddDivisional(divisional);

			button = new GOGUIButton(panel, divisional, true);
			button->Init(cfg, wxString::Format(wxT("Setter%03dDivisional%03d"), i, j + 100), j + 3, 100 + i);
			panel->AddControl(button);
		}
	}
	return panel;
}

GOGUIPanel* GOrgueSetter::CreateGeneralsPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_GENERALS);
	panel->Init(cfg, metrics, _("Generals"), wxT("SetterGeneralsPanel"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterGenerals"));
	panel->AddControl(back);

	GOGUILabel* BankDisplay=new GOGUILabel(panel, &m_BankDisplay);
	BankDisplay->Init(cfg, wxT("SetterGeneralBank"), 260, 20);
	panel->AddControl(BankDisplay);

	button = new GOGUIButton(panel, m_button[ID_SETTER_GENERAL_PREV], false);
	button->Init(cfg, wxT("SetterGerneralsPrev"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_GENERAL_NEXT], false);
	button->Init(cfg, wxT("SetterGerneralsNext"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SET], false);
	button->Init(cfg, wxT("SetterGeneralsSet"), 1, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_REGULAR], false);
	button->Init(cfg, wxT("SetterGerneralsRegular"), 3, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPE], false);
	button->Init(cfg, wxT("SetterGeneralsScope"), 4, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPED], false);
	button->Init(cfg, wxT("SetterGeneralsScoped"), 5, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_FULL], false);
	button->Init(cfg, wxT("SetterGeneralsFull"), 7, 101);
	panel->AddControl(button);

	for(unsigned i = 0; i < GENERALS; i++)
	{
		button = new GOGUIButton(panel, m_button[ID_SETTER_GENERAL00 + i], true);
		button->Init(cfg, wxString::Format(wxT("SetterGeneral%d"), i + 1), (i % 10) + 1, 100 + i / 10);
		panel->AddControl(button);
	}

	return panel;
}

GOGUIPanel* GOrgueSetter::CreateSetterPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_SETTER);
	panel->Init(cfg, metrics, _("Combination Setter"), wxT("SetterPanel"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("Setter"));
	panel->AddControl(back);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_PosDisplay);
	PosDisplay->Init(cfg, wxT("SetterCurrentPosition"), 350, 10);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CURRENT], false);
	button->Init(cfg, wxT("SetterCurrent"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_M100], false);
	button->Init(cfg, wxT("SetterM100"), 2, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_M10], false);
	button->Init(cfg, wxT("SetterM10"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_M1], false);
	button->Init(cfg, wxT("SetterM1"), 4, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_PREV], false);
	button->Init(cfg, wxT("SetterPrev"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_NEXT], false);
	button->Init(cfg, wxT("SetterNext"), 6, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_P1], false);
	button->Init(cfg, wxT("SetterP1"), 7, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_P10], false);
	button->Init(cfg, wxT("SetterP10"), 8, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_P100], false);
	button->Init(cfg, wxT("SetterP100"), 9, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_HOME], false);
	button->Init(cfg, wxT("SetterHome"), 10, 100);
	panel->AddControl(button);

	for(unsigned i = 0; i < 10; i++)
	{
		button = new GOGUIButton(panel, m_button[ID_SETTER_L0 + i], false);
		button->Init(cfg, wxString::Format(wxT("SetterL%d"), i), i + 1, 101);
		panel->AddControl(button);
	}

	button = new GOGUIButton(panel, m_button[ID_SETTER_SET], false);
	button->Init(cfg, wxT("SetterSet"), 1, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_REGULAR], false);
	button->Init(cfg, wxT("SetterRegular"), 3, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPE], false);
	button->Init(cfg, wxT("SetterScope"), 4, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPED], false);
	button->Init(cfg, wxT("SetterScoped"), 5, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_FULL], false);
	button->Init(cfg, wxT("SetterFull"), 7, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_GC], false);
	button->Init(cfg, wxT("SetterGC"), 8, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_INSERT], false);
	button->Init(cfg, wxT("SetterInsert"), 9, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_DELETE], false);
	button->Init(cfg, wxT("SetterDelete"), 10, 102);
	panel->AddControl(button);

	return panel;
}

GOGUIPanel* GOrgueSetter::CreateCrescendoPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_CRESCENDO);
	panel->Init(cfg, metrics, _("Crescendo Pedal"), wxT("SetterCrescendoPanel"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterCrescendo"));
	panel->AddControl(back);

	GOGUIEnclosure* enclosure = new GOGUIEnclosure(panel, &m_swell);
	enclosure->Init(cfg, wxT("SetterSwell"));
	panel->AddControl(enclosure);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_CrescendoDisplay);
	PosDisplay->Init(cfg, wxT("SetterCrescendoPosition"), 350, 10);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SET], false);
	button->Init(cfg, wxT("SetterCrescendoSet"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_REGULAR], false);
	button->Init(cfg, wxT("SetterCrescendoRegular"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPE], false);
	button->Init(cfg, wxT("SetterCrescendoScope"), 4, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_SCOPED], false);
	button->Init(cfg, wxT("SetterCrescendoScoped"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_FULL], false);
	button->Init(cfg, wxT("SetterCrescendoFull"), 7, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CRESCENDO_A], false);
	button->Init(cfg, wxT("SetterCrescendoA"), 1, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CRESCENDO_B], false);
	button->Init(cfg, wxT("SetterCrescendoB"), 2, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CRESCENDO_C], false);
	button->Init(cfg, wxT("SetterCrescendoC"), 3, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CRESCENDO_D], false);
	button->Init(cfg, wxT("SetterCrescendoD"), 4, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CRESCENDO_PREV], false);
	button->Init(cfg, wxT("SetterCrescendoPrev"), 6, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CRESCENDO_CURRENT], false);
	button->Init(cfg, wxT("SetterCrescendoCurrent"), 7, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_SETTER_CRESCENDO_NEXT], false);
	button->Init(cfg, wxT("SetterCrescendoNext"), 8, 101);
	panel->AddControl(button);

	return panel;
}

void GOrgueSetter::Load(GOrgueConfigReader& cfg)
{
	wxString buffer;

	m_framegeneral.resize(0);
	for (unsigned i = 0; i < FRAME_GENERALS; i++)
	{
		m_framegeneral.push_back(new GOrgueFrameGeneral(m_organfile->GetGeneralTemplate(), m_organfile, true));
		buffer.Printf(wxT("FrameGeneral%03d"), i + 1);
		m_framegeneral[i]->Load(cfg, buffer);
	}

	m_general.resize(0);
	for (unsigned i = 0; i < GENERALS * GENERAL_BANKS; i++)
	{
		m_general.push_back(new GOrgueFrameGeneral(m_organfile->GetGeneralTemplate(), m_organfile, true));
		buffer.Printf(wxT("SetterGeneral%03d"), i + 1);
		m_general[i]->Load(cfg, buffer);
	}

	m_crescendo.resize(0);
	for (unsigned i = 0; i < 4 * CRESCENDO_STEPS; i++)
	{
		m_crescendo.push_back(new GOrgueFrameGeneral(m_organfile->GetGeneralTemplate(), m_organfile, true));
		buffer.Printf(wxT("SetterCrescendo%d_%03d"), (i / CRESCENDO_STEPS) + 1, (i % CRESCENDO_STEPS)+ 1);
		m_crescendo[i]->Load(cfg, buffer);
	}

	m_button[ID_SETTER_PREV]->Init(cfg, wxT("SetterPrev"), _("Previous"));
	m_button[ID_SETTER_NEXT]->Init(cfg, wxT("SetterNext"), _("Next"));
	m_button[ID_SETTER_SET]->Init(cfg, wxT("SetterSet"), _("Set"));
	m_button[ID_SETTER_M1]->Init(cfg, wxT("SetterM1"), _("-1"));
	m_button[ID_SETTER_M10]->Init(cfg, wxT("SetterM10"), _("-10"));
	m_button[ID_SETTER_M100]->Init(cfg, wxT("SetterM100"), _("-100"));
	m_button[ID_SETTER_P1]->Init(cfg, wxT("SetterP1"), _("+1"));
	m_button[ID_SETTER_P10]->Init(cfg, wxT("SetterP10"), _("+10"));
	m_button[ID_SETTER_P100]->Init(cfg, wxT("SetterP100"), _("+100"));
	m_button[ID_SETTER_CURRENT]->Init(cfg, wxT("SetterCurrent"), _("Current"));
	m_button[ID_SETTER_HOME]->Init(cfg, wxT("SetterHome"), _("000"));
	m_button[ID_SETTER_GC]->Init(cfg, wxT("SetterGC"), _("G.C."));

	m_button[ID_SETTER_REGULAR]->Init(cfg, wxT("SetterRegular"), _("Regular"));
	m_button[ID_SETTER_SCOPE]->Init(cfg, wxT("SetterScope"), _("Scope"));
	m_button[ID_SETTER_SCOPED]->Init(cfg, wxT("SetterScoped"), _("Scoped"));
	m_button[ID_SETTER_FULL]->Init(cfg, wxT("SetterFull"), _("Full"));

	m_button[ID_SETTER_INSERT]->Init(cfg, wxT("SetterInsert"), _("Insert"));
	m_button[ID_SETTER_DELETE]->Init(cfg, wxT("SetterDelete"), _("Delete"));

	m_button[ID_SETTER_CRESCENDO_PREV]->Init(cfg, wxT("SetterCrescendoPrev"), _("<"));
	m_button[ID_SETTER_CRESCENDO_NEXT]->Init(cfg, wxT("SetterCrescendoNext"), _(">"));
	m_button[ID_SETTER_CRESCENDO_CURRENT]->Init(cfg, wxT("SetterCrescendoCurrent"), _("Current"));
	m_button[ID_SETTER_CRESCENDO_A]->Init(cfg, wxT("SetterCrescendoA"), _("A"));
	m_button[ID_SETTER_CRESCENDO_B]->Init(cfg, wxT("SetterCrescendoB"), _("B"));
	m_button[ID_SETTER_CRESCENDO_C]->Init(cfg, wxT("SetterCrescendoC"), _("C"));
	m_button[ID_SETTER_CRESCENDO_D]->Init(cfg, wxT("SetterCrescendoD"), _("D"));

	m_button[ID_SETTER_PITCH_M1]->Init(cfg, wxT("SetterPitchM1"), _("-1"));
	m_button[ID_SETTER_PITCH_M10]->Init(cfg, wxT("SetterPitchM10"), _("-10"));
	m_button[ID_SETTER_PITCH_M100]->Init(cfg, wxT("SetterPitchM100"), _("-100"));
	m_button[ID_SETTER_PITCH_P1]->Init(cfg, wxT("SetterPitchP1"), _("+1"));
	m_button[ID_SETTER_PITCH_P10]->Init(cfg, wxT("SetterPitchP10"), _("+10"));
	m_button[ID_SETTER_PITCH_P100]->Init(cfg, wxT("SetterPitchP100"), _("+100"));
	m_button[ID_SETTER_TEMPERAMENT_PREV]->Init(cfg, wxT("SetterTemperamentPrev"), _("<"));
	m_button[ID_SETTER_TEMPERAMENT_NEXT]->Init(cfg, wxT("SetterTemperamentNext"), _(">"));
	m_button[ID_SETTER_TRANSPOSE_DOWN]->Init(cfg, wxT("SetterTransposeDown"), _("-"));
	m_button[ID_SETTER_TRANSPOSE_UP]->Init(cfg, wxT("SetterTransposeUp"), _("+"));

	m_button[ID_SETTER_SAVE]->Init(cfg, wxT("SetterSave"), _("Save"));

	m_swell.Init(cfg, wxT("SetterSwell"), _("Crescendo"));

	m_PosDisplay.Init(cfg, wxT("SetterCurrentPosition"));
	m_BankDisplay.Init(cfg, wxT("SetterGeneralBank"));
	m_CrescendoDisplay.Init(cfg, wxT("SetterCrescendoPosition"));
	m_TransposeDisplay.Init(cfg, wxT("SetterTranspose"));

	for(unsigned i = 0; i < 10; i++)
	{
		wxString group;
		wxString buffer;
		buffer.Printf(_("__%d"), i);
		group.Printf(wxT("SetterL%d"), i);
		m_button[ID_SETTER_L0 + i]->Init(cfg, group, buffer);
	}

	for(unsigned i = 0; i < GENERALS; i++)
	{
		wxString group;
		wxString buffer;
		buffer.Printf(_("%d"), i+1);
		group.Printf(wxT("SetterGeneral%d"), i);
		m_button[ID_SETTER_GENERAL00 + i]->Init(cfg, group, buffer);
	}
	m_button[ID_SETTER_GENERAL_PREV]->Init(cfg, wxT("SetterGeneralPrev"), _("Prev"));
	m_button[ID_SETTER_GENERAL_NEXT]->Init(cfg, wxT("SetterGeneralNext"), _("Next"));
}

void GOrgueSetter::SetterButtonChanged(GOrgueSetterButton* button)
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
				wxTheApp->GetTopWindow()->UpdateWindowUI();
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
			case ID_SETTER_GC:
				m_organfile->Reset();
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
			case ID_SETTER_GENERAL30:
			case ID_SETTER_GENERAL31:
			case ID_SETTER_GENERAL32:
			case ID_SETTER_GENERAL33:
			case ID_SETTER_GENERAL34:
			case ID_SETTER_GENERAL35:
			case ID_SETTER_GENERAL36:
			case ID_SETTER_GENERAL37:
			case ID_SETTER_GENERAL38:
			case ID_SETTER_GENERAL39:
			case ID_SETTER_GENERAL40:
			case ID_SETTER_GENERAL41:
			case ID_SETTER_GENERAL42:
			case ID_SETTER_GENERAL43:
			case ID_SETTER_GENERAL44:
			case ID_SETTER_GENERAL45:
			case ID_SETTER_GENERAL46:
			case ID_SETTER_GENERAL47:
			case ID_SETTER_GENERAL48:
			case ID_SETTER_GENERAL49:
				m_general[i - ID_SETTER_GENERAL00 + m_bank * GENERALS ]->Push();
				ResetDisplay();
				m_button[i]->Display(true);
				break;
			case ID_SETTER_GENERAL_PREV:
			case ID_SETTER_GENERAL_NEXT:
				if (i == ID_SETTER_GENERAL_PREV && m_bank > 0)
					m_bank--;
				if (i == ID_SETTER_GENERAL_NEXT && m_bank < GENERAL_BANKS - 1)
					m_bank++;

				m_BankDisplay.SetName(wxString::Format(wxT("%c"), m_bank + wxT('A')));
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
			case ID_SETTER_CRESCENDO_A:
			case ID_SETTER_CRESCENDO_B:
			case ID_SETTER_CRESCENDO_C:
			case ID_SETTER_CRESCENDO_D:
				SetCrescendoType(i - ID_SETTER_CRESCENDO_A);
				break;

			case ID_SETTER_CRESCENDO_PREV:
				Crescendo(m_crescendopos - 1, true);
				break;
			case ID_SETTER_CRESCENDO_NEXT:
				Crescendo(m_crescendopos + 1, true);
				break;
			case ID_SETTER_CRESCENDO_CURRENT:
				m_crescendo[m_crescendopos + m_crescendobank * CRESCENDO_STEPS]->Push();
				break;

			case ID_SETTER_PITCH_M1:
				m_organfile->GetPipeConfig().ModifyTuning(-1);
				break;
			case ID_SETTER_PITCH_M10:
				m_organfile->GetPipeConfig().ModifyTuning(-10);
				break;
			case ID_SETTER_PITCH_M100:
				m_organfile->GetPipeConfig().ModifyTuning(-100);
				break;
			case ID_SETTER_PITCH_P1:
				m_organfile->GetPipeConfig().ModifyTuning(1);
				break;
			case ID_SETTER_PITCH_P10:
				m_organfile->GetPipeConfig().ModifyTuning(10);
				break;
			case ID_SETTER_PITCH_P100:
				m_organfile->GetPipeConfig().ModifyTuning(100);
				break;
			case ID_SETTER_SAVE:
				m_organfile->Save();
				break;

			case ID_SETTER_TEMPERAMENT_NEXT:
			case ID_SETTER_TEMPERAMENT_PREV:
				{
					unsigned index = m_organfile->GetSettings().GetTemperaments().GetTemperamentIndex(m_organfile->GetTemperament());
					index += m_organfile->GetSettings().GetTemperaments().GetTemperamentCount();
					if (i == ID_SETTER_TEMPERAMENT_NEXT)
						index++;
					else
						index--;
					index = index % m_organfile->GetSettings().GetTemperaments().GetTemperamentCount();
					m_organfile->SetTemperament(m_organfile->GetSettings().GetTemperaments().GetTemperamentName(index));
				}
				break;

			case ID_SETTER_TRANSPOSE_DOWN:
			case ID_SETTER_TRANSPOSE_UP:
				{
					int value = m_organfile->GetSettings().GetTranspose();
					if (i == ID_SETTER_TRANSPOSE_UP)
						value++;
					else
						value--;
					SetTranspose(value);
				}
				break;
			}
}

void GOrgueSetter::Abort()
{
	m_PosDisplay.Abort();
	m_CrescendoDisplay.Abort();
	m_swell.Abort();
}

void GOrgueSetter::PreparePlayback()
{
	wxString buffer;
	buffer.Printf(wxT("%03d"), m_pos);
	m_PosDisplay.SetName(buffer);
	m_PosDisplay.PreparePlayback();

	wxCommandEvent event(wxEVT_SETVALUE, ID_METER_FRAME_SPIN);
	event.SetInt(m_pos);
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

	buffer.Printf(wxT("%d"), m_crescendopos + 1);
	m_CrescendoDisplay.SetName(buffer);
	m_CrescendoDisplay.PreparePlayback();

	buffer.Printf(wxT("%c"), m_bank + wxT('A'));
	m_BankDisplay.SetName(buffer);
	m_BankDisplay.PreparePlayback();

	UpdateTranspose();
	m_TransposeDisplay.PreparePlayback();

	m_swell.PreparePlayback();

	for(unsigned i = 0; i < m_button.size(); i++)
		m_button[i]->PreparePlayback();
}

void GOrgueSetter::PrepareRecording()
{
	m_PosDisplay.PrepareRecording();
	m_CrescendoDisplay.PrepareRecording();
	m_BankDisplay.PrepareRecording();
	m_TransposeDisplay.PrepareRecording();
	m_swell.PrepareRecording();

	for(unsigned i = 0; i < m_button.size(); i++)
		m_button[i]->PrepareRecording();
}

void GOrgueSetter::Update()
{
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

void GOrgueSetter::SetCrescendoType(unsigned no)
{
	m_crescendobank = no;
	m_button[ID_SETTER_CRESCENDO_A]->Display(no == 0);
	m_button[ID_SETTER_CRESCENDO_B]->Display(no == 1);
	m_button[ID_SETTER_CRESCENDO_C]->Display(no == 2);
	m_button[ID_SETTER_CRESCENDO_D]->Display(no == 3);
}

void GOrgueSetter::ResetDisplay()
{
	m_button[ID_SETTER_HOME]->Display(false);
	for(unsigned i = 0; i < 10; i++)
		m_button[ID_SETTER_L0 + i]->Display(false);
	for(unsigned i = 0; i < GENERALS; i++)
		m_button[ID_SETTER_GENERAL00 + i]->Display(false);
}

void GOrgueSetter::UpdatePosition(int pos)
{
	if (pos != (int)m_pos)
		SetPosition(pos);
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
		for(unsigned i = 0; i < GENERALS; i++)
			m_button[ID_SETTER_GENERAL00 + i]->Display(false);
	}

	buffer.Printf(wxT("%03d"), m_pos);
	m_PosDisplay.SetName(buffer);
	if (pos != old_pos)
	{
		wxCommandEvent event(wxEVT_SETVALUE, ID_METER_FRAME_SPIN);
		event.SetInt(m_pos);
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
	}

}

void GOrgueSetter::Crescendo(int newpos, bool force)
{
	if (newpos < 0)
		newpos = 0;
	if (newpos > CRESCENDO_STEPS - 1)
		newpos = CRESCENDO_STEPS - 1;
	if (IsSetterActive() && !force)
		return;
	unsigned pos = newpos;
	if (pos == m_crescendopos)
		return;

	while (pos > m_crescendopos)
	{
		m_crescendo[++m_crescendopos + m_crescendobank * CRESCENDO_STEPS]->Push();
	}

	while (pos < m_crescendopos)
	{
		m_crescendo[--m_crescendopos + m_crescendobank * CRESCENDO_STEPS]->Push();
	}

	wxString buffer;
	buffer.Printf(wxT("%d"), m_crescendopos + 1);
	m_CrescendoDisplay.SetName(buffer);
}

void GOrgueSetter::ControlChanged(void* control)
{
	if (control == &m_swell)
		Crescendo(m_swell.GetValue() * CRESCENDO_STEPS / 128);
}

void GOrgueSetter::UpdateTranspose()
{
	m_TransposeDisplay.SetName(wxString::Format(wxT("%d"), m_organfile->GetSettings().GetTranspose()));
}

void GOrgueSetter::SetTranspose(int value)
{
	if (value <= -11)
		value = -11;
	if (value >= 11)
		value = 11;
	if (m_organfile->GetSettings().GetTranspose() != value)
	{
		wxCommandEvent event(wxEVT_SETVALUE, ID_METER_TRANSPOSE_SPIN);
		event.SetInt(value);
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
	}
	m_organfile->GetSettings().SetTranspose(value);
	m_organfile->AllNotesOff();
	UpdateTranspose();
}
