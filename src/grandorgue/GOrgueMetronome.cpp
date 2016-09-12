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

#include "GOrgueMetronome.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueRank.h"
#include "GOrgueSetterButton.h"
#include "GOrgueSettings.h"
#include "GOrgueSoundingPipe.h"
#include "GOrgueWindchest.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

enum {
	ID_METRONOME_ON = 0,
	ID_METRONOME_MEASURE_P1,
	ID_METRONOME_MEASURE_M1,
	ID_METRONOME_BEAT_P1,
	ID_METRONOME_BEAT_M1,
	ID_METRONOME_BEAT_P10,
	ID_METRONOME_BEAT_M10,
};

const struct ElementListEntry GOrgueMetronome::m_element_types[] = {
	{ wxT("MetronomeOn"), ID_METRONOME_ON, false, false },
	{ wxT("MetronomeMeasureP1"), ID_METRONOME_MEASURE_P1, false, true },
	{ wxT("MetronomeMeasureM1"), ID_METRONOME_MEASURE_M1, false, true },
	{ wxT("MetronomeBpmP1"), ID_METRONOME_BEAT_P1, false, true },
	{ wxT("MetronomeBpmM1"), ID_METRONOME_BEAT_M1, false, true },
	{ wxT("MetronomeBpmP10"), ID_METRONOME_BEAT_P10, false, true },
	{ wxT("MetronomeBpmM10"), ID_METRONOME_BEAT_M10, false, true },
	{ wxT(""), -1, false, false },
};

const struct ElementListEntry* GOrgueMetronome::GetButtonList()
{
	return m_element_types;
}

GOrgueMetronome::GOrgueMetronome(GrandOrgueFile *organfile) :
	m_organfile(organfile),
	m_BPM(80),
	m_MeasureLength(4),
	m_Pos(0),
	m_Running(false),
	m_BPMDisplay(organfile),
	m_MeasureDisplay(organfile),
	m_rank(NULL),
	m_StopID(0)
{
	CreateButtons(m_organfile);

	m_button[ID_METRONOME_ON]->SetPreconfigIndex(25);
	m_button[ID_METRONOME_MEASURE_P1]->SetPreconfigIndex(28);
	m_button[ID_METRONOME_MEASURE_M1]->SetPreconfigIndex(29);
	m_button[ID_METRONOME_BEAT_P1]->SetPreconfigIndex(26);
	m_button[ID_METRONOME_BEAT_M1]->SetPreconfigIndex(27);

	m_organfile->RegisterPlaybackStateHandler(this);
}

GOrgueMetronome::~GOrgueMetronome()
{
	StopTimer();
}

void GOrgueMetronome::Load(GOrgueConfigReader& cfg)
{
	m_group = wxT("Metronome");
	m_BPM = cfg.ReadInteger(CMBSetting, m_group, wxT("BPM"), 1, 500, false, m_organfile->GetSettings().MetronomeBPM());
	m_MeasureLength = cfg.ReadInteger(CMBSetting, m_group, wxT("MeasureLength"), 0, 32, false, m_organfile->GetSettings().MetronomeMeasure());

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

void GOrgueMetronome::ButtonChanged(int id)
{
	switch(id)
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
	if ((int)m_BPM + val < 1)
		m_BPM = 1;
	else
		m_BPM += val;
	if (m_BPM > 500)
		m_BPM = 500;
	if (m_Running)
		m_organfile->UpdateInterval(this, 60000 / m_BPM);
	UpdateState();
}

void GOrgueMetronome::UpdateMeasure(int val)
{
	if ((int)m_MeasureLength + val < 0)
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
	m_organfile->SetTimer(0, this, 60000 / m_BPM);
}

void GOrgueMetronome::StopTimer()
{
	m_organfile->DeleteTimer(this);
	m_Running = false;
	UpdateState();
}

void GOrgueMetronome::HandleTimer()
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

GOrgueEnclosure* GOrgueMetronome::GetEnclosure(const wxString& name, bool is_panel)
{
	return NULL;
}

GOrgueLabel* GOrgueMetronome::GetLabel(const wxString& name, bool is_panel)
{
	if (is_panel)
		return NULL;

	if (name == wxT("MetronomeBPM"))
		return &m_BPMDisplay;
	if (name == wxT("MetronomeMeasure"))
		return &m_MeasureDisplay;

	return NULL;
}
