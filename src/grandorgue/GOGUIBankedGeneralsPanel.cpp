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

#include "GOGUIBankedGeneralsPanel.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUIBankedGeneralsPanel::GOGUIBankedGeneralsPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile)
{
}

GOGUIBankedGeneralsPanel:: ~GOGUIBankedGeneralsPanel()
{
}

void GOGUIBankedGeneralsPanel::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateBankedGeneralsPanel(cfg));
}

GOGUIPanel* GOGUIBankedGeneralsPanel::CreateBankedGeneralsPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_GENERALS);
	panel->Init(cfg, metrics, _("Generals"), wxT("SetterGeneralsPanel"));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("SetterGenerals"));
	panel->AddControl(back);

	GOGUILabel* BankDisplay=new GOGUILabel(panel, m_organfile->GetLabel(wxT("GeneralLabel")));
	BankDisplay->Init(cfg, wxT("SetterGeneralBank"), 260, 20);
	panel->AddControl(BankDisplay);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("GeneralPrev")), false);
	button->Init(cfg, wxT("SetterGerneralsPrev"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("GeneralNext")), false);
	button->Init(cfg, wxT("SetterGerneralsNext"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Set")), false);
	button->Init(cfg, wxT("SetterGeneralsSet"), 1, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Regular")), false);
	button->Init(cfg, wxT("SetterGerneralsRegular"), 3, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scope")), false);
	button->Init(cfg, wxT("SetterGeneralsScope"), 4, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Scoped")), false);
	button->Init(cfg, wxT("SetterGeneralsScoped"), 5, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("Full")), false);
	button->Init(cfg, wxT("SetterGeneralsFull"), 7, 101);
	panel->AddControl(button);

	for(unsigned i = 0; i < 50; i++)
	{
		button = new GOGUIButton(panel, m_organfile->GetButton(wxString::Format(wxT("General%02d"), i + 1)), true);
		button->Init(cfg, wxString::Format(wxT("SetterGeneral%d"), i + 1), (i % 10) + 1, 100 + i / 10);
		panel->AddControl(button);
	}

	return panel;
}
