/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueStop.h"
#include "GOrguePipe.h"
#include "GrandOrgueFile.h"
#include "GOrgueSound.h"
#include "GOrgueDisplayMetrics.h"

/* TODO: This should not be... */
extern GrandOrgueFile* organfile;

GOrgueStop::GOrgueStop() :
	GOrgueDrawstop(),
	m_Pipes(0),
	m_ManualNumber(0),
	Percussive(false),
	m_Auto(false),
	AmplitudeLevel(0),
	NumberOfLogicalPipes(0),
	FirstAccessiblePipeLogicalPipeNumber(0),
	FirstAccessiblePipeLogicalKeyNumber(0),
	NumberOfAccessiblePipes(0),
	WindchestGroup(0)
{
}

void GOrgueStop::Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics)
{
	AmplitudeLevel = cfg.ReadInteger(group, wxT("AmplitudeLevel"), 0, 1000);
	NumberOfLogicalPipes = cfg.ReadInteger(group, wxT("NumberOfLogicalPipes"), 1, 192);
	FirstAccessiblePipeLogicalPipeNumber = cfg.ReadInteger(group, wxT("FirstAccessiblePipeLogicalPipeNumber"), 1, NumberOfLogicalPipes);
	FirstAccessiblePipeLogicalKeyNumber = cfg.ReadInteger(group, wxT("FirstAccessiblePipeLogicalKeyNumber"), 1,  128);
	NumberOfAccessiblePipes = cfg.ReadInteger(group, wxT("NumberOfAccessiblePipes"), 1, NumberOfLogicalPipes);
	WindchestGroup = cfg.ReadInteger(group, wxT("WindchestGroup"), 1, organfile->GetWinchestGroupCount());
	Percussive = cfg.ReadBoolean(group, wxT("Percussive"));

	WindchestGroup += organfile->GetTremulantCount();    // we would + 1 but it already has it: clever!

	int i;
	wxString buffer;

	m_Pipes.clear();
	for (i = 0; i < NumberOfLogicalPipes; i++)
	{
		buffer.Printf(wxT("Pipe%03d"), i + 1);
		wxString file = cfg.ReadString(group, buffer);

		m_Pipes.push_back(new GOrguePipe(file, Percussive, WindchestGroup, organfile->GetAmplitude() * AmplitudeLevel));
	}

	m_Auto = NumberOfLogicalPipes == 1;

	GOrgueDrawstop::Load(cfg, group, displayMetrics);
}

void GOrgueStop::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
	GOrgueDrawstop::Save(cfg, prefix, group);
}

bool GOrgueStop::Set(bool on)
{
	if (DefaultToEngaged == on)
		return on;
	DefaultToEngaged = on;
	organfile->MIDIPretend(true);
	DefaultToEngaged = !on;
	organfile->MIDIPretend(false);

	bool retval = GOrgueDrawstop::Set(on);

	/* m_auto seems to state that if a stop only has 1 note, the note isn't
	 * actually controlled by a manual, but will be on if the stop is on and
	 * off if the stop is off... */
	if (m_Auto)
	{
		GOrgueManual* manual = organfile->GetManual(m_ManualNumber);
		manual->Set(manual->GetFirstAccessibleKeyMIDINoteNumber() + FirstAccessiblePipeLogicalKeyNumber - 1, on);
	}

	return retval;
}

GOrguePipe* GOrgueStop::GetPipe(unsigned index)
{
	return m_Pipes[index];
}

GOrgueStop::~GOrgueStop(void)
{
	while (m_Pipes.size())
	{
		if (m_Pipes.back())
			delete m_Pipes.back();
		m_Pipes.pop_back();
	}
}

unsigned GOrgueStop::GetPipeCount()
{
	return m_Pipes.size();
}
