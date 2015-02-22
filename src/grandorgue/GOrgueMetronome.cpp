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

#include "GOrgueMetronome.h"
#include <wx/intl.h>
#include "GOGUIButton.h"
#include "GOGUIHW1Background.h"
#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUISetterDisplayMetrics.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueRank.h"
#include "GOrgueSetterButton.h"
#include "GOrgueSoundingPipe.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"

enum {
	ID_METRONOME_ON = 0,
	ID_METRONOME_MEASURE_P1,
	ID_METRONOME_MEASURE_M1,
	ID_METRONOME_BEAT_P1,
	ID_METRONOME_BEAT_M1,
	ID_METRONOME_BEAT_P10,
	ID_METRONOME_BEAT_M10,
};

GOrgueMetronome::GOrgueMetronome(GrandOrgueFile *organfile) :
	wxTimer(),
	m_organfile(organfile),
	m_button(),
	m_BPM(80),
	m_MeasureLength(4),
	m_Pos(0),
	m_Running(false),
	m_BPMDisplay(organfile),
	m_MeasureDisplay(organfile),
	m_rank(NULL),
	m_StopID(0)
{
        m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

        m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
        m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

        m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
        m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
        m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));
        m_button.push_back(new GOrgueSetterButton(m_organfile, this, true));

	m_organfile->RegisterPlaybackStateHandler(this);
}

GOrgueMetronome::~GOrgueMetronome()
{
	StopTimer();
}

void GOrgueMetronome::Load(GOrgueConfigReader& cfg)
{
	m_group = wxT("Metronome");
	m_BPM = cfg.ReadInteger(CMBSetting, m_group, wxT("BPM"), 1, 500, false, 80);
	m_MeasureLength = cfg.ReadInteger(CMBSetting, m_group, wxT("MeasureLength"), 0, 32, false, 4);

	m_button[ID_METRONOME_ON]->Init(cfg, wxT("MetronomeOn"), _("ON"));
	m_button[ID_METRONOME_MEASURE_P1]->Init(cfg, wxT("MetronomeMP1"), _("+1"));
	m_button[ID_METRONOME_MEASURE_M1]->Init(cfg, wxT("MetronomeMM1"), _("-1"));
	m_button[ID_METRONOME_BEAT_P1]->Init(cfg, wxT("MetronomeBPMP1"), _("+1"));
	m_button[ID_METRONOME_BEAT_M1]->Init(cfg, wxT("MetronomeBPMM1"), _("-1"));
	m_button[ID_METRONOME_BEAT_P10]->Init(cfg, wxT("MetronomeBPMP10"), _("+10"));
	m_button[ID_METRONOME_BEAT_M10]->Init(cfg, wxT("MetronomeBPMM10"), _("-10"));

	m_BPMDisplay.Init(cfg, wxT("MetronomeBPM"));
	m_MeasureDisplay.Init(cfg, wxT("MetronomeMeasure"));

	m_organfile->RegisterSaveableObject(this);

	GOrgueWindchest* windchest = new GOrgueWindchest(m_organfile);
	windchest->Init(cfg, wxT("MetronomeWindchest"), _("Metronome"));
	unsigned samplegroup = m_organfile->AddWindchest(windchest);

	m_rank = new GOrgueRank(m_organfile);
	m_rank->Init(cfg, wxT("MetronomSounds"), _("Metronome"), 36, samplegroup);
	m_StopID = m_rank->RegisterStop(NULL);
	m_organfile->AddRank(m_rank);

	GOrgueSoundingPipe* pipe;
	pipe = new GOrgueSoundingPipe(m_organfile, m_rank, true, samplegroup, 36, 8, 0, 100, 100, false);
	m_rank->AddPipe(pipe);
	pipe->Init(cfg, wxT("MetronomSounds"), wxT("A"), wxT("sounds\\metronome\\beat.wv"));
	pipe = new GOrgueSoundingPipe(m_organfile, m_rank, true, samplegroup, 37, 8, 0, 100, 100, false);
	m_rank->AddPipe(pipe);
	pipe->Init(cfg, wxT("MetronomSounds"), wxT("B"), wxT("sounds\\metronome\\first_beat.wv"));
}

void GOrgueMetronome::Save(GOrgueConfigWriter& cfg)
{
	cfg.WriteInteger(m_group, wxT("BPM"), m_BPM);
	cfg.WriteInteger(m_group, wxT("MeasureLength"), m_MeasureLength);
}

void GOrgueMetronome::SetterButtonChanged(GOrgueSetterButton* button)
{
	for(unsigned i = 0; i < m_button.size(); i++)
		if (m_button[i] == button)
			switch(i)
			{
			case ID_METRONOME_ON:
				if (m_Running)
					StopTimer();
				else
					StartTimer();
				break;

			case ID_METRONOME_MEASURE_P1:
				UpdateMeasure(1);
				break;

			case ID_METRONOME_MEASURE_M1:
				UpdateMeasure(-1);
				break;

			case ID_METRONOME_BEAT_P1:
				UpdateBPM(1);
				break;

			case ID_METRONOME_BEAT_M1:
				UpdateBPM(-1);
				break;

			case ID_METRONOME_BEAT_P10:
				UpdateBPM(10);
				break;

			case ID_METRONOME_BEAT_M10:
				UpdateBPM(-10);
				break;
			}
}

void GOrgueMetronome::UpdateBPM(int val)
{
	if (m_BPM + val < 1)
		m_BPM = 1;
	else
		m_BPM += val;
	if (m_BPM > 500)
		m_BPM = 500;
	UpdateState();
}

void GOrgueMetronome::UpdateMeasure(int val)
{
	if (m_MeasureLength + val < 0)
		m_MeasureLength = 0;
	else
		m_MeasureLength += val;
	if (m_MeasureLength > 32)
		m_MeasureLength = 32;
	UpdateState();
}

void GOrgueMetronome::UpdateState()
{
	m_BPMDisplay.SetName(wxString::Format(_("%d BPM"), m_BPM));
	m_MeasureDisplay.SetName(wxString::Format(_("%d"), m_MeasureLength));
	m_button[ID_METRONOME_ON]->Display(m_Running);
}

void GOrgueMetronome::StartTimer()
{
	StopTimer();
	m_Pos = 0;
	m_Running = true;
	UpdateState();
	wxTimer::Start(1, true);
}

void GOrgueMetronome::StopTimer()
{
	wxTimer::Stop();
	m_Running = false;
	UpdateState();
}

void GOrgueMetronome::Notify()
{
	unsigned type = 0;
	if (m_Pos || !m_MeasureLength)
		type = 0;
	else
		type = 1;
	m_rank->SetKey(type, 0x7f, m_StopID);
	m_rank->SetKey(type, 0, m_StopID);

	m_Pos++;
	if (m_Pos >= m_MeasureLength)
		m_Pos = 0;

	wxTimer::Start(60000 / m_BPM, true);
}

void GOrgueMetronome::AbortPlayback()
{
	StopTimer();
}

void GOrgueMetronome::PreparePlayback()
{
	m_Running = false;
	UpdateState();
}

void GOrgueMetronome::StartPlayback()
{
}

void GOrgueMetronome::PrepareRecording()
{
}

void GOrgueMetronome::CreatePanels(GOrgueConfigReader& cfg)
{
	m_organfile->AddPanel(CreateMetronomePanel(cfg));
}

GOGUIPanel* GOrgueMetronome::CreateMetronomePanel(GOrgueConfigReader& cfg)
{
	GOGUIButton* button;

	GOGUIPanel* panel = new GOGUIPanel(m_organfile);
	GOGUIDisplayMetrics* metrics = new GOGUISetterDisplayMetrics(cfg, m_organfile, GOGUI_METRONOME);
	panel->Init(cfg, metrics, _("Metronome"), wxT("Metronome"), wxT(""));

	GOGUIHW1Background* back = new GOGUIHW1Background(panel);
	back->Init(cfg, wxT("Metronome"));
	panel->AddControl(back);

	button = new GOGUIButton(panel, m_button[ID_METRONOME_ON], false);
	button->Init(cfg, wxT("MetronomeOn"), 1, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_METRONOME_MEASURE_M1], false);
	button->Init(cfg, wxT("MetronomeMM1"), 3, 100);
	panel->AddControl(button);

	GOGUILabel* PosDisplay=new GOGUILabel(panel, &m_MeasureDisplay);
	PosDisplay->Init(cfg, wxT("MetronomeMeasure"), 240, 45);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_button[ID_METRONOME_MEASURE_P1], false);
	button->Init(cfg, wxT("MetronomeMP1"), 5, 100);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_METRONOME_BEAT_M10], false);
	button->Init(cfg, wxT("MetronomeBPMM10"), 1, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_METRONOME_BEAT_M1], false);
	button->Init(cfg, wxT("MetronomeBPMM1"), 2, 101);
	panel->AddControl(button);

	PosDisplay=new GOGUILabel(panel, &m_BPMDisplay);
	PosDisplay->Init(cfg, wxT("MetronomeBPM"), 160, 115);
	panel->AddControl(PosDisplay);

	button = new GOGUIButton(panel, m_button[ID_METRONOME_BEAT_P1], false);
	button->Init(cfg, wxT("MetronomeBPMP1"), 4, 101);
	panel->AddControl(button);

	button = new GOGUIButton(panel, m_button[ID_METRONOME_BEAT_P10], false);
	button->Init(cfg, wxT("MetronomeBPMP10"), 5, 101);
	panel->AddControl(button);

	return panel;
}

GOGUIControl* GOrgueMetronome::CreateGUIElement(GOrgueConfigReader& cfg, wxString type, GOGUIPanel* panel)
{
	return NULL;
}



