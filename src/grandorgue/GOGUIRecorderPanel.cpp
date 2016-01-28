/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#include "GOGUIRecorderPanel.h"

#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUIRecorderPanel::GOGUIRecorderPanel(GrandOrgueFile* organfile) : 
	m_organfile(organfile)
{
}

GOGUIRecorderPanel::~GOGUIRecorderPanel()
{
}

void GOGUIRecorderPanel::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateRecorderPanel(cfg));
}

GOGUIPanel* GOGUIRecorderPanel::CreateRecorderPanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_RECORDER);
	panel->Init(cfg, metrics, _("Recorder"), wxT("Recorder"), wxT(""));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("Recorder"));
	panel->AddControl(back);

	GOGUILabel* label=new GOGUILabel(panel, NULL);
	label->Init(cfg, wxT("MidiRecorderLabel"), 1, 35, _("MIDI Recorder"));
	panel->AddControl(label);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MidiRecorderRecord")), false);
	button->Init(cfg, wxT("MidiRecorderRecord"), 2, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MidiRecorderStop")), false);
	button->Init(cfg, wxT("MidiRecorderStop"), 3, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_organfile->GetButton(wxT("MidiRecorderRecordRename")), false);
	button->Init(cfg, wxT("MidiRecorderRecordRename"), 4, 100);
	panel->AddControl(button);

	return panel;
}
