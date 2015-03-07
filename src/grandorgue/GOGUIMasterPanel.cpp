/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include "GOGUIMasterPanel.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUIMasterPanel::GOGUIMasterPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile)
{
}

GOGUIMasterPanel:: ~GOGUIMasterPanel()
{
}

void GOGUIMasterPanel::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateMasterPanel(cfg));
}

GOGUIPanel* GOGUIMasterPanel::CreateMasterPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_MASTER);
	panel->Init(cfg, metrics, _("Master Controls"), wxT("SetterMaster"), wxT(""));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterMaster"));
	panel->AddControl(back);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchM100")), false);
	button->Init(cfg, wxT("SetterMasterPitchM100"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchM10")), false);
	button->Init(cfg, wxT("SetterMasterPitchM10"), 2, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchM1")), false);
	button->Init(cfg, wxT("SetterMasterPitchM1"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchP1")), false);
	button->Init(cfg, wxT("SetterMasterPitchP1"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchP10")), false);
	button->Init(cfg, wxT("SetterMasterPitchP10"), 6, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("PitchP100")), false);
	button->Init(cfg, wxT("SetterMasterPitchP10"), 7, 100);
	panel->AddControl(button);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("PitchLabel")));
	PosDisplay->Init(cfg, wxT("SetterMasterPitch"), 230, 35);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("TemperamentPrev")), false);
	button->Init(cfg, wxT("SetterMasterTemperamentPrev"), 1, 101);
	panel->AddControl(button);

	PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("TemperamentLabel")));
	PosDisplay->Init(cfg, wxT("SetterMasterTemperament"), 80, 90, wxEmptyString, 2);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("TemperamentNext")), false);
	button->Init(cfg, wxT("SetterMasterTemperamentNext"), 3, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Save")), false);
	button->Init(cfg, wxT("SetterSave"), 5, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("OnState")), true);
	button->Init(cfg, wxT("SetterOn"), 1, 100, 4);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("TransposeDown")), false);
	button->Init(cfg, wxT("SetterMasterTransposeDown"), 1, 102);
	panel->AddControl(button);

	PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("TransposeLabel")));
	PosDisplay->Init(cfg, wxT("SetterMasterTranspose"), 80, 175);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("TransposeUp")), false);
	button->Init(cfg, wxT("SetterMasterTransposeUp"), 3, 102);
	panel->AddControl(button);

	PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("OrganNameLabel")));
	PosDisplay->Init(cfg, wxT("SetterMasterName"), 180, 230, wxEmptyString, 5);
	panel->AddControl(PosDisplay);

	return panel;
}

