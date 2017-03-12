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

#include "GOGUIFloatingPanel.h"

#include "GOGUIEnclosure.h"
#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOrgueDivisional.h"
#include "GOrgueEnclosure.h"
#include "GOrgueManual.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUIFloatingPanel::GOGUIFloatingPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile)
{
}

GOGUIFloatingPanel::~GOGUIFloatingPanel()
{
}

void GOGUIFloatingPanel::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateFloatingPanel(cfg));
}

GOGUIPanel* GOGUIFloatingPanel::CreateFloatingPanel(GOrgueConfigReader& cfg)
{
	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_SETTER_FLOATING);
	panel->Init(cfg, metrics, _("Coupler manuals and volume"), wxT("SetterFloating"), wxT(""));

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
	master_enc->Init(cfg, wxT("SetterMasterVolume"), _("Master"), 127);
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
		enc->Init(cfg, wxString::Format(wxT("SetterMaster%03d"), i + 1), windchest->GetName(), 127);
		m_organfile->AddEnclosure(enc);
		enc->SetElementID(m_organfile->GetRecorderElementID(wxString::Format(wxT("SM%d"), i)));
		windchest->AddEnclosure(enc);

		enclosure = new GOGUIEnclosure(panel, enc);
		enclosure->Init(cfg, wxString::Format(wxT("SetterMaster%03d"), i + 1));
		panel->AddControl(enclosure);
	}

	return panel;
}
