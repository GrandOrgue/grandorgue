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
#include "GOrgueManual.h"

#include <algorithm>
#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrguePipe.h"
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "IniFileConfig.h"
#include "MIDIListenDialog.h"

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;
extern const char* s_MIDIMessages[];

GOrgueManual::GOrgueManual() :
	m_ManualNumber(0),
	m_MIDI(),
	NumberOfLogicalKeys(0),
	FirstAccessibleKeyLogicalKeyNumber(0),
	FirstAccessibleKeyMIDINoteNumber(0),
	NumberOfAccessibleKeys(0),
	MIDIInputNumber(0),
	NumberOfStops(0),
	NumberOfCouplers(0),
	NumberOfDivisionals(0),
	NumberOfTremulants(0),
	tremulant(),
	Name(),
	stop(),
	coupler(NULL),
	divisional(NULL),
	Displayed(false),
	m_Width(0),
	m_Height(0),
	m_X(0),
	m_Y(0),
	m_KeysY(0),
	m_PistonY(0),
	DispKeyColourInverted(false),
	DispKeyColourWooden(false)
{

}

void GOrgueManual::Load(IniFileConfig& cfg, const char* group, GOrgueDisplayMetrics* displayMetrics, int manualNumber)
{

	Name                                = cfg.ReadString (group, "Name", 32);
	NumberOfLogicalKeys                 = cfg.ReadInteger(group, "NumberOfLogicalKeys", 1, 192);
	FirstAccessibleKeyLogicalKeyNumber  = cfg.ReadInteger(group, "FirstAccessibleKeyLogicalKeyNumber", 1, NumberOfLogicalKeys);
	FirstAccessibleKeyMIDINoteNumber    = cfg.ReadInteger(group, "FirstAccessibleKeyMIDINoteNumber", 0, 127);
	NumberOfAccessibleKeys              = cfg.ReadInteger(group, "NumberOfAccessibleKeys", 0, 85);
	MIDIInputNumber                     = cfg.ReadInteger(group, "MIDIInputNumber", 1, 6);
	Displayed                           = cfg.ReadBoolean(group, "Displayed");
	DispKeyColourInverted               = cfg.ReadBoolean(group, "DispKeyColourInverted");
	DispKeyColourWooden                 = cfg.ReadBoolean(group, "DispKeyColourWooden", false);
	NumberOfStops                       = cfg.ReadInteger(group, "NumberOfStops", 0, 64);
	NumberOfCouplers                    = cfg.ReadInteger(group, "NumberOfCouplers", 0, 16, false);
	NumberOfDivisionals                 = cfg.ReadInteger(group, "NumberOfDivisionals", 0, 32, false);
	NumberOfTremulants                  = cfg.ReadInteger(group, "NumberOfTremulants", 0, 10, false);

	m_ManualNumber = manualNumber;

	char buffer[64];

	for (unsigned i = 0; i < NumberOfStops; i++)
	{
		stop.push_back(new GOrgueStop());
		sprintf(buffer, "Stop%03d", i + 1);
		sprintf(buffer, "Stop%03d", cfg.ReadInteger( group, buffer, 1, 448));
		stop[i]->m_ManualNumber = m_ManualNumber;
		stop[i]->Load(cfg, buffer, displayMetrics);
	}

	coupler = new GOrgueCoupler[NumberOfCouplers];
	for (unsigned i = 0; i < NumberOfCouplers; i++)
	{
		sprintf(buffer, "Coupler%03d", i + 1);
		sprintf(buffer, "Coupler%03d", cfg.ReadInteger( group, buffer, 1, 64));
		coupler[i].Load(cfg, buffer, organfile->GetFirstManualIndex(), organfile->GetManualAndPedalCount(), displayMetrics);
	}

	divisional = new GOrgueDivisional[NumberOfDivisionals];
	for (unsigned i = 0; i < NumberOfDivisionals; i++)
	{
		sprintf(buffer, "Divisional%03d", i + 1);
		sprintf(buffer, "Divisional%03d", cfg.ReadInteger( group, buffer, 1, 224));
		divisional[i].Load(cfg, buffer, m_ManualNumber, i, displayMetrics);
	}

	for (unsigned i = 0; i < NumberOfTremulants; i++)
	{
		sprintf(buffer, "Tremulant%03d", i + 1);
		tremulant[i] = cfg.ReadInteger( group, buffer, 1, organfile->GetTremulantCount());
	}

	for (unsigned i = 0; i < NumberOfStops; i++)
	{
		if (!stop[i]->m_auto)
			continue;
		for (unsigned j = 0; j < NumberOfStops; j++)
		{
			if (i == j)
				continue;
			if (stop[j]->FirstAccessiblePipeLogicalKeyNumber < stop[i]->FirstAccessiblePipeLogicalKeyNumber + stop[i]->NumberOfAccessiblePipes &&
				stop[j]->FirstAccessiblePipeLogicalKeyNumber + stop[j]->NumberOfAccessiblePipes > stop[i]->FirstAccessiblePipeLogicalKeyNumber)
			{
				stop[i]->m_auto = stop[j]->m_auto = false;
				break;
			}
		}
	}
}

void GOrgueManual::Set(int note, bool on, bool pretend, int depth, GOrgueCoupler* prev)
{
	int j;

	// test polyphony?
#if 0
	for (i = 0; i < organfile->m_NumberOfPipes; i++)
		organfile->m_pipe[i]->Set(on);
	return;
#endif

	if (depth > 32)
	{
		::wxLogFatalError("Infinite recursive coupling detected!");
		return;
	}

	note -= FirstAccessibleKeyMIDINoteNumber;
	bool outofrange = note < 0 || note >= NumberOfAccessibleKeys;

	if (!depth && outofrange)
		return;

	if (!outofrange && !pretend)
	{
		if (depth)
		{
			if (!(m_MIDI[note] >> 1) && !on)
				return;
			m_MIDI[note] += on ? 2 : -2;
		}
		else
		{
			if ((m_MIDI[note] & 1) ^ !on)
				return;
			m_MIDI[note]  = (m_MIDI[note] & 0xFFFFFFFE) | (on ? 1 : 0);
		}
	}

	bool unisonoff = false;
	for (unsigned i = 0; i < NumberOfCouplers; i++)
	{
		if (!coupler[i].DefaultToEngaged)
			continue;
		if (coupler[i].UnisonOff && (!depth || (prev && prev->CoupleToSubsequentUnisonIntermanualCouplers)))
		{
			unisonoff = true;
			continue;
		}
		j = coupler[i].DestinationManual;
		if (
				(!depth)
				||
				(
					(prev)
					&&
					(
						(j == m_ManualNumber && coupler[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntramanualCouplers)
						||
						(j == m_ManualNumber && coupler[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntramanualCouplers)
						||
						(j != m_ManualNumber && coupler[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntermanualCouplers)
						||
						(j != m_ManualNumber && coupler[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntermanualCouplers)
					)
				)
			)
		{
			organfile->GetManual(j)->Set(note + FirstAccessibleKeyMIDINoteNumber + coupler[i].DestinationKeyshift, on, false, depth + 1, coupler + i);
		}
	}

	if (!unisonoff)
	{
		for (unsigned i = 0; i < NumberOfStops; i++)
		{
			if (!stop[i]->DefaultToEngaged)
				continue;
			j = note - (stop[i]->FirstAccessiblePipeLogicalKeyNumber - 1);
			if (j < 0 || j >= stop[i]->NumberOfAccessiblePipes)
				continue;
			j += stop[i]->FirstAccessiblePipeLogicalPipeNumber - 1;

			organfile->GetPipe(stop[i]->pipe[j])->Set(on);
		}
	}

	if (!outofrange)
	{
		wxCommandEvent event(wxEVT_NOTEONOFF, 0);
		event.SetInt(m_ManualNumber);
		event.SetExtraLong(note);
		::wxGetApp().frame->AddPendingEvent(event);
	}
}


void GOrgueManual::MIDI(void)
{
	int index = MIDIInputNumber + 7;
	MIDIListenDialog dlg(::wxGetApp().frame, _(s_MIDIMessages[index]), g_sound->i_midiEvents[index], 1);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxConfigBase::Get()->Write(wxString("MIDI/") + s_MIDIMessages[index], dlg.GetEvent());
		g_sound->ResetSound(organfile);
	}
}


template<class T>
struct delete_functor
{
	void operator()(T* p)
	{
		delete p;
	}
};

GOrgueManual::~GOrgueManual(void)
{
	std::for_each(stop.begin(),stop.end(),delete_functor<GOrgueStop>());
	if (coupler)
		delete[] coupler;
	if (divisional)
		delete[] divisional;
}

int GOrgueManual::GetMIDIInputNumber()
{
	return MIDIInputNumber;
}

int GOrgueManual::GetLogicalKeyCount()
{
	return NumberOfLogicalKeys;
}

int GOrgueManual::GetNumberOfAccessibleKeys()
{
	return NumberOfAccessibleKeys;
}

/* TODO: I suspect this could be made private or into something better... */
int GOrgueManual::GetFirstAccessibleKeyMIDINoteNumber()
{
	return FirstAccessibleKeyMIDINoteNumber;
}

int GOrgueManual::GetStopCount()
{
	return NumberOfStops;
}

GOrgueStop* GOrgueManual::GetStop(unsigned index)
{
	assert(index < NumberOfStops);
	return stop[index];
}

int GOrgueManual::GetCouplerCount()
{
	return NumberOfCouplers;
}

GOrgueCoupler* GOrgueManual::GetCoupler(unsigned index)
{
	assert(index < NumberOfCouplers);
	return &coupler[index];
}

int GOrgueManual::GetDivisionalCount()
{
	return NumberOfDivisionals;
}

GOrgueDivisional* GOrgueManual::GetDivisional(unsigned index)
{
	assert(index < NumberOfDivisionals);
	return &divisional[index];
}

int GOrgueManual::GetTremulantCount()
{
	return NumberOfTremulants;
}

GOrgueTremulant* GOrgueManual::GetTremulant(unsigned index)
{
	/* FIXME: Figure out what's going on here - then document and fix it. */
	assert(index < NumberOfTremulants);
	return organfile->GetTremulant(tremulant[index] - 1);
}

void GOrgueManual::AllNotesOff()
{

	/* TODO: I'm not sure if these are allowed to be merged into one loop. */
	for (int j = 0; j < GetNumberOfAccessibleKeys(); j++)
        m_MIDI[j] = 0;
	for (int j = 0; j < GetNumberOfAccessibleKeys(); j++)
	{
		wxCommandEvent event(wxEVT_NOTEONOFF, 0);
		event.SetInt(m_ManualNumber);
		event.SetExtraLong(j);
		::wxGetApp().frame->AddPendingEvent(event);
	}
}

/* TODO: figure out what this thing does and document it */
void GOrgueManual::MIDIPretend(bool on)
{
	for (int j = 0; j < NumberOfLogicalKeys; j++)
		if (m_MIDI[j] & 1)
			Set(j + FirstAccessibleKeyMIDINoteNumber, on, true);
}

bool GOrgueManual::IsKeyDown(int midiNoteNumber)
{
	if (midiNoteNumber < FirstAccessibleKeyMIDINoteNumber)
		return false;
	if (midiNoteNumber > FirstAccessibleKeyMIDINoteNumber + NumberOfLogicalKeys - 1)
		return false;
	return m_MIDI[midiNoteNumber - FirstAccessibleKeyMIDINoteNumber];
}

