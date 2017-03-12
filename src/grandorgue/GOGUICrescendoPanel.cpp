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

#include "GOGUICrescendoPanel.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUIEnclosure.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUICrescendoPanel::GOGUICrescendoPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile)
{
}

GOGUICrescendoPanel:: ~GOGUICrescendoPanel()
{
}

void GOGUICrescendoPanel::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateCrescendoPanel(cfg));
}

GOGUIPanel* GOGUICrescendoPanel::CreateCrescendoPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_CRESCENDO);
	panel->Init(cfg, metrics, _("Crescendo Pedal"), wxT("SetterCrescendoPanel"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterCrescendo"));
	panel->AddControl(back);

	GOGUIEnclosure* enclosure = new GOGUIEnclosure(panel, m_organfile->GetEnclosure(wxT("Swell")));
	enclosure->Init(cfg, wxT("SetterSwell"));
	panel->AddControl(enclosure);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("CrescendoLabel")));
	PosDisplay->Init(cfg, wxT("SetterCrescendoPosition"), 350, 10);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Set")), false);
	button->Init(cfg, wxT("SetterCrescendoSet"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Regular")), false);
	button->Init(cfg, wxT("SetterCrescendoRegular"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scope")), false);
	button->Init(cfg, wxT("SetterCrescendoScope"), 4, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scoped")), false);
	button->Init(cfg, wxT("SetterCrescendoScoped"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Full")), false);
	button->Init(cfg, wxT("SetterCrescendoFull"), 7, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("CrescendoA")), false);
	button->Init(cfg, wxT("SetterCrescendoA"), 1, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("CrescendoB")), false);
	button->Init(cfg, wxT("SetterCrescendoB"), 2, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("CrescendoC")), false);
	button->Init(cfg, wxT("SetterCrescendoC"), 3, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("CrescendoD")), false);
	button->Init(cfg, wxT("SetterCrescendoD"), 4, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("CrescendoPrev")), false);
	button->Init(cfg, wxT("SetterCrescendoPrev"), 6, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("CrescendoCurrent")), false);
	button->Init(cfg, wxT("SetterCrescendoCurrent"), 7, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("CrescendoNext")), false);
	button->Init(cfg, wxT("SetterCrescendoNext"), 8, 101);
	panel->AddControl(button);

	return panel;
}
