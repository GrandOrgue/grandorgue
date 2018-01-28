/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#include "GOGUIMetronomePanel.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUIMetronomePanel::GOGUIMetronomePanel(GrandOrgueFile* organfile) : 
	m_organfile(organfile)
{
}

GOGUIMetronomePanel::~GOGUIMetronomePanel()
{
}

void GOGUIMetronomePanel::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateMetronomePanel(cfg));
}

GOGUIPanel* GOGUIMetronomePanel::CreateMetronomePanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_METRONOME);
	panel->Init(cfg, metrics, _("Metronome"), wxT("Metronome"), wxT(""));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("Metronome"));
	panel->AddControl(back);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MetronomeOn")), false);
	button->Init(cfg, wxT("MetronomeOn"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MetronomeMeasureM1")), false);
	button->Init(cfg, wxT("MetronomeMM1"), 3, 100);
	panel->AddControl(button);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("MetronomeMeasure")));
	PosDisplay->Init(cfg, wxT("MetronomeMeasure"), 240, 45);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MetronomeMeasureP1")), false);
	button->Init(cfg, wxT("MetronomeMP1"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MetronomeBpmM10")), false);
	button->Init(cfg, wxT("MetronomeBPMM10"), 1, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MetronomeBpmM1")), false);
	button->Init(cfg, wxT("MetronomeBPMM1"), 2, 101);
	panel->AddControl(button);

	PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("MetronomeBPM")));
	PosDisplay->Init(cfg, wxT("MetronomeBPM"), 160, 115);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MetronomeBpmP1")), false);
	button->Init(cfg, wxT("MetronomeBPMP1"), 4, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MetronomeBpmP10")), false);
	button->Init(cfg, wxT("MetronomeBPMP10"), 5, 101);
	panel->AddControl(button);

	return panel;
}
