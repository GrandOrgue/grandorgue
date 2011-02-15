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
#include "IniFileConfig.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrgueSound.h"
#include "OrganFile.h"
#include "MIDIListenDialog.h"
#include "GOrgueDivisional.h"
#include <algorithm>

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;
extern const char* s_MIDIMessages[];

void GOrgueManual::Load(IniFileConfig& cfg, const char* group)
{
  Name								 = cfg.ReadString( group,"Name",   32);
  NumberOfLogicalKeys				 = cfg.ReadInteger( group,"NumberOfLogicalKeys",    1,  192);
  FirstAccessibleKeyLogicalKeyNumber = cfg.ReadInteger( group,"FirstAccessibleKeyLogicalKeyNumber",    1, NumberOfLogicalKeys);
  FirstAccessibleKeyMIDINoteNumber	 = cfg.ReadInteger( group,"FirstAccessibleKeyMIDINoteNumber",    0,  127);	// spec says 128
  NumberOfAccessibleKeys			 = cfg.ReadInteger( group,"NumberOfAccessibleKeys",    0,   85);
  MIDIInputNumber					 = cfg.ReadInteger( group,"MIDIInputNumber",    1,    6);
  Displayed							 = cfg.ReadBoolean( group,"Displayed");
  DispKeyColourInverted				 = cfg.ReadBoolean( group,"DispKeyColourInverted");
  DispKeyColourWooden				 = cfg.ReadBoolean( group,"DispKeyColourWooden",false);
  NumberOfStops						 = cfg.ReadInteger( group,"NumberOfStops",    0,   64);
  NumberOfCouplers					 = cfg.ReadInteger( group,"NumberOfCouplers",    0,   16, false);
  NumberOfDivisionals				 = cfg.ReadInteger( group,"NumberOfDivisionals",    0,   32, false);
  NumberOfTremulants				 = cfg.ReadInteger( group,"NumberOfTremulants",    0,   10, false);

  int i, j;
  char buffer[64];

  for (i = 0; i < NumberOfStops; i++)
	{
	  stop.push_back(new GOrgueStop());

	  sprintf(buffer, "Stop%03d", i + 1);
	  sprintf(buffer, "Stop%03d", cfg.ReadInteger( group, buffer, 1, 448));
	  stop[i]->m_ManualNumber = m_ManualNumber;
	  stop[i]->Load(cfg, buffer);
	}

  coupler = new GOrgueCoupler[NumberOfCouplers];
  for (i = 0; i < NumberOfCouplers; i++)
	{
	  sprintf(buffer, "Coupler%03d", i + 1);
	  sprintf(buffer, "Coupler%03d", cfg.ReadInteger( group, buffer, 1, 64));
	  coupler[i].Load(cfg, buffer);
	}

  divisional = new GOrgueDivisional[NumberOfDivisionals];
  for (i = 0; i < NumberOfDivisionals; i++)
	{
	  sprintf(buffer, "Divisional%03d", i + 1);
	  sprintf(buffer, "Divisional%03d", cfg.ReadInteger( group, buffer, 1, 224));
	  divisional[i].m_ManualNumber = m_ManualNumber;
	  divisional[i].m_DivisionalNumber = i;
	  divisional[i].Load(cfg, buffer);
	}

  for (i = 0; i < NumberOfTremulants; i++)
	{
	  sprintf(buffer, "Tremulant%03d", i + 1);
	  tremulant[i] = cfg.ReadInteger( group, buffer, 1, organfile->m_NumberOfTremulants);
	}

  for (i = 0; i < NumberOfStops; i++)
	{
	  if (!stop[i]->m_auto)
		continue;
	  for (j = 0; j < NumberOfStops; j++)
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
  int i, j;

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
  for (i = 0; i < NumberOfCouplers; i++)
	{
	  if (!coupler[i].DefaultToEngaged)
		continue;
	  if (coupler[i].UnisonOff && (!depth || (prev && prev->CoupleToSubsequentUnisonIntermanualCouplers)))
		{
		  unisonoff = true;
		  continue;
		}
	  j = coupler[i].DestinationManual;
	  if (!depth || (prev && (
							  (j == m_ManualNumber && coupler[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntramanualCouplers) ||
							  (j == m_ManualNumber && coupler[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntramanualCouplers) ||
							  (j != m_ManualNumber && coupler[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntermanualCouplers) ||
							  (j != m_ManualNumber && coupler[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntermanualCouplers)
							  )))
		{
		  organfile->m_manual[j].Set(note + FirstAccessibleKeyMIDINoteNumber + coupler[i].DestinationKeyshift, on, false, depth + 1, coupler + i);
		}
	}

  if (!unisonoff)
	{
	  for (i = 0; i < NumberOfStops; i++)
		{
		  if (!stop[i]->DefaultToEngaged)
			continue;
		  j = note - (stop[i]->FirstAccessiblePipeLogicalKeyNumber - 1);
		  if (j < 0 || j >= stop[i]->NumberOfAccessiblePipes)
			continue;
		  j += stop[i]->FirstAccessiblePipeLogicalPipeNumber - 1;

		  organfile->m_pipe[stop[i]->pipe[j]]->Set(on);
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
	  g_sound->ResetSound();
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
