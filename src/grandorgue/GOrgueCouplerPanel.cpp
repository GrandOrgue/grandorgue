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

#include "GOrgueCouplerPanel.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOrgueCoupler.h"
#include "GOrgueManual.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrgueCouplerPanel::GOrgueCouplerPanel(GrandOrgueFile* organfile) :
	m_organfile(organfile)
{
}

GOrgueCouplerPanel::~GOrgueCouplerPanel()
{
}

void GOrgueCouplerPanel::Load(GOrgueConfigReader& cfg)
{
}

GOGUIControl* GOrgueCouplerPanel::CreateGUIElement(GOrgueConfigReader& cfg, wxString group, GOGUIPanel* panel)
{
	return NULL;
}

void GOrgueCouplerPanel::CreatePanels(GOrgueConfigReader& cfg)
{
	for(unsigned i = m_organfile->GetFirstManualIndex(); i <= m_organfile->GetManualAndPedalCount(); i++)
		m_organfile->AddPanel(CreateCouplerPanel(cfg, i));
}

GOGUIPanel* GOrgueCouplerPanel::CreateCouplerPanel(GOrgueConfigReader& cfg, unsigned manual_nr)
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
