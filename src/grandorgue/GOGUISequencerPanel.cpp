/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#include "GOGUISequencerPanel.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUISequencerPanel::GOGUISequencerPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile)
{
}

GOGUISequencerPanel:: ~GOGUISequencerPanel()
{
}

void GOGUISequencerPanel::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateSequencerPanel(cfg));
}

GOGUIPanel* GOGUISequencerPanel::CreateSequencerPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_SETTER);
	panel->Init(cfg, metrics, _("Combination Setter"), wxT("SetterPanel"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("Setter"));
	panel->AddControl(back);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("Label")));
	PosDisplay->Init(cfg, wxT("SetterCurrentPosition"), 350, 10);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Current")), false);
	button->Init(cfg, wxT("SetterCurrent"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("M100")), false);
	button->Init(cfg, wxT("SetterM100"), 2, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("M10")), false);
	button->Init(cfg, wxT("SetterM10"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("M1")), false);
	button->Init(cfg, wxT("SetterM1"), 4, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Prev")), false);
	button->Init(cfg, wxT("SetterPrev"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Next")), false);
	button->Init(cfg, wxT("SetterNext"), 6, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("P1")), false);
	button->Init(cfg, wxT("SetterP1"), 7, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("P10")), false);
	button->Init(cfg, wxT("SetterP10"), 8, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("P100")), false);
	button->Init(cfg, wxT("SetterP100"), 9, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Home")), false);
	button->Init(cfg, wxT("SetterHome"), 10, 100);
	panel->AddControl(button);

	for(unsigned i = 0; i < 10; i++)
	{
		button = new GOGUIButton(panel, m_organfile->GetButton(wxString::Format(wxT("L%d"), i)), false);
		button->Init(cfg, wxString::Format(wxT("SetterL%d"), i), i + 1, 101);
		panel->AddControl(button);
	}

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Set")), false);
	button->Init(cfg, wxT("SetterSet"), 1, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Regular")), false);
	button->Init(cfg, wxT("SetterRegular"), 3, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scope")), false);
	button->Init(cfg, wxT("SetterScope"), 4, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scoped")), false);
	button->Init(cfg, wxT("SetterScoped"), 5, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Full")), false);
	button->Init(cfg, wxT("SetterFull"), 7, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("GC")), false);
	button->Init(cfg, wxT("SetterGC"), 8, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Insert")), false);
	button->Init(cfg, wxT("SetterInsert"), 9, 102);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Delete")), false);
	button->Init(cfg, wxT("SetterDelete"), 10, 102);
	panel->AddControl(button);

	return panel;
}
