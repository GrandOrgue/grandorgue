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

#include "GOrgueSound.h"

#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueEnclosure.h"
#include "GOrgueGeneral.h"
#include "GOrgueMeter.h"
#include "GOrguePiston.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GrandOrgue.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueFrame.h"
#include "GrandOrgueID.h"
#include <vector>

extern GrandOrgueFile* organfile;

void
GOrgueSound::MIDICallback
	(std::vector<unsigned char>& msg
	,int which)
{
	int i, j, k, q, Noteoffset;

    Noteoffset = 0;
    // MIDI message FF : reset
    if (msg[0] == 0xFF && b_active && organfile)
	{
        for (k = organfile->GetFirstManualIndex(); k <= organfile->GetManualAndPedalCount(); k++)
        {
            for (j = 0; j < organfile->GetManual(k)->GetStopCount(); j++)
                organfile->GetManual(k)->GetStop(j)->Set(false);
            for (j = 0; j < organfile->GetManual(k)->GetCouplerCount(); j++)
                organfile->GetManual(k)->GetCoupler(j)->Set(false);
            for (j = 0; j < organfile->GetManual(k)->GetDivisionalCount(); j++)
            {
                GOrgueDivisional *divisional = organfile->GetManual(k)->GetDivisional(j);
                if (divisional->DispImageNum & 2)
                {
                    divisional->DispImageNum &= 1;
                    wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
                    event.SetClientData(divisional);
                    ::wxGetApp().frame->AddPendingEvent(event);
                }
            }

        }
        for (j = 0; j < organfile->GetTremulantCount(); j++)
            organfile->GetTremulant(j)->Set(false);
        for (j = 0; j < organfile->GetDivisionalCouplerCount(); j++)
            organfile->GetDivisionalCoupler(j)->Set(false);

        for (k = 0; k < organfile->GetGeneralCount(); k++)
        {
            GOrgueGeneral *general = organfile->GetGeneral(k);
            if (general->DispImageNum & 2)
            {
                general->DispImageNum &= 1;
                wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
                event.SetClientData(general);
                ::wxGetApp().frame->AddPendingEvent(event);
            }
        }

        return;
	}

	register unsigned char c = msg[0] & 0xF0; // c = MIDI message code without chanel number in c

	if (c == 0x80 && msg.size() == 3) // if MIDI code for note OFF and 2 data bytes
	{
		c ^= 0x10; // c = 0x90 , MIDI code note ON
		msg[2] = 0; // set velocity to zero
	}
	msg[0] = c | ((msg[0] + m_midi_devices[which].id) & 0x0F); // msg[0] = MIDI code + channel from device + channel offset

	j = msg[0] << 8; // j = channel in higher byte
	if (msg.size() > 1) // if midi meesage has data
		j |= msg[1]; // j = channel in higher byte and first byte of MIDI data in lower byte
	if (b_listening)
	{
		wxCommandEvent event(wxEVT_LISTENING, 0);
		event.SetInt(j);
		listen_evthandler->AddPendingEvent(event);
	}

	if (!b_active || msg.size() < 2)
		return;

	// MIDI code for controller
	if (c == 0xB0 && (msg[1] == 120 || msg[1] == 123) && organfile)
	{
        for (i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
            if (msg[0] == ((i_midiEvents[organfile->GetManual(i)->GetMIDIInputNumber() + 7] | 0xB000) >> 8))
            {
/* TODO: this code is not equivalent to the old code but seems to be the right
 * thing to do given the context... (midi controller change/all notes off) */
#if 0
            	for (j = 0; j < organfile->GetManual(i)->GetNumberOfAccessibleKeys(); j++)
                    if (organfile->GetManual(i)->m_MIDI[j] == 1)
                        organfile->GetManual(i)->Set(j + organfile->GetManual(i)->FirstAccessibleKeyMIDINoteNumber, false);
#else
            	organfile->GetManual(i)->AllNotesOff();
#endif
            }
        return;
	}

	// MIDI code for memory set
	if (i_midiEvents[15] == j && (((j & 0xF000) == 0xC000) || b_memset ^ (bool)msg[2]))
	{
        ::wxGetApp().frame->ProcessCommand(ID_AUDIO_MEMSET);
		::wxGetApp().frame->UpdateWindowUI();
	}

    // MIDI for different organ??
	std::map<long, wxString>::iterator it;
	it=organmidiEvents.find(j);
	if (it!=organmidiEvents.end()) {
//	    wxMessageDialog dlg(::wxGetApp().frame, it->second);
//	    dlg.ShowModal();
        wxCommandEvent event(wxEVT_LOADFILE, 0);
        event.SetString(it->second);
        ::wxGetApp().frame->AddPendingEvent(event);
//        ::wxGetApp().frame->ProcessCommand(wxEVT_LOADFILE);
//        organfile->Load(it->second);
	}

	// For the 16 MIDI devices
	int GOrgueevent, offset;
	for (i = 0; i < 15; i++)
	{
		// ??
		if (i == 8 || (i == 2 && msg.size() < 3))
		{
			j &= 0xFF00;
			if (i != 8)
                i = 14;
		}
		GOrgueevent = i_midiEvents[i];
		if (i>=8 && i<=13) GOrgueevent &= 0xFF00;
  //      if ((gOrgueSoundInstance->i_midiEvents[i] & 0xFF00) == j )
        if (GOrgueevent == j)
		{
			if (i < 2)
			{
				if (msg.size() > 2 && !msg[2])
					continue;
				GOrgueMeter* meter = ::wxGetApp().frame->m_meters[2];
				meter->SetValue(meter->GetValue() + (i ? 1 : -1));
			}
			else if (i < 8)
			{
				q = i - 1;
				for (unsigned l = 0; l < organfile->GetEnclosureCount(); l++)
					if (organfile->GetEnclosure(l)->GetMIDIInputNumber() == q)
						organfile->GetEnclosure(l)->Set(msg[2]);
			}
			else if (i < 14)
			{
				q = i - 7;
				for (k = organfile->GetFirstManualIndex(); k <= organfile->GetManualAndPedalCount(); k++)
				{
				    if (!organfile->GetManual(k)->Displayed)
                        continue;
                    offset = i_midiEvents[i] & 0xFF;
                    if  (offset > 127)
							offset = offset - 140;
					if (organfile->GetManual(k)->GetMIDIInputNumber() == q)
						organfile->GetManual(k)->Set(msg[1]+offset+transpose, msg[2] ? true : false);
				}
			}
			else
			{
			    if ((c == 0x90 || c == 0xB0) && msg.size() == 3)
			    {
                    for (k = organfile->GetFirstManualIndex(); k <= organfile->GetManualAndPedalCount(); k++)
                    {
                        for (j = 0; j < organfile->GetManual(k)->GetStopCount(); j++)
                            if (msg[1] == organfile->GetManual(k)->GetStop(j)->StopControlMIDIKeyNumber)
                                organfile->GetManual(k)->GetStop(j)->Set(msg[2] ? true : false);
                        for (j = 0; j < organfile->GetManual(k)->GetCouplerCount(); j++)
                            if (msg[1] == organfile->GetManual(k)->GetCoupler(j)->StopControlMIDIKeyNumber)
                                organfile->GetManual(k)->GetCoupler(j)->Set(msg[2] ? true : false);
                    }
                    for (j = 0; j < organfile->GetTremulantCount(); j++)
                        if (msg[1] == organfile->GetTremulant(j)->StopControlMIDIKeyNumber)
                            organfile->GetTremulant(j)->Set(msg[2] ? true : false);
                    for (j = 0; j < organfile->GetDivisionalCouplerCount(); j++)
                        if (msg[1] == organfile->GetDivisionalCoupler(j)->StopControlMIDIKeyNumber)
                            organfile->GetDivisionalCoupler(j)->Set(msg[2] ? true : false);
			    }
			    else if (c == 0xC0 && msg.size() == 2)
			    {
                    for (k = organfile->GetFirstManualIndex(); k <= organfile->GetManualAndPedalCount(); k++)
                    {
                        for (j = 0; j < organfile->GetManual(k)->GetStopCount(); j++)
                            if (msg[1] == organfile->GetManual(k)->GetStop(j)->StopControlMIDIKeyNumber)
                                organfile->GetManual(k)->GetStop(j)->Push();
                        for (j = 0; j < organfile->GetManual(k)->GetCouplerCount(); j++)
                            if (msg[1] == organfile->GetManual(k)->GetCoupler(j)->StopControlMIDIKeyNumber)
                                organfile->GetManual(k)->GetCoupler(j)->Push();
                    }
                    for (j = 0; j < organfile->GetTremulantCount(); j++)
                        if (msg[1] == organfile->GetTremulant(j)->StopControlMIDIKeyNumber)
                            organfile->GetTremulant(j)->Push();
                    for (j = 0; j < organfile->GetDivisionalCouplerCount(); j++)
                        if (msg[1] == organfile->GetDivisionalCoupler(j)->StopControlMIDIKeyNumber)
                            organfile->GetDivisionalCoupler(j)->Push();
			    }
			}
		}
	}

	if (c == 0xC0)
	{
		for (k = organfile->GetFirstManualIndex(); k <= organfile->GetManualAndPedalCount(); k++)
			for (j = 0; j < organfile->GetManual(k)->GetDivisionalCount(); j++)
				if (msg[0] == (i_midiEvents[organfile->GetManual(k)->GetMIDIInputNumber() + 7] ^ 0x5000) >> 8 && msg[1] == organfile->GetManual(k)->GetDivisional(j)->MIDIProgramChangeNumber - 1)
					organfile->GetManual(k)->GetDivisional(j)->Push();
		for (j = 0; j < organfile->GetGeneralCount(); j++)
			if (msg[1] == organfile->GetGeneral(j)->MIDIProgramChangeNumber - 1)
				organfile->GetGeneral(j)->Push();
		for (j = 0; j < organfile->GetNumberOfReversiblePistons(); j++)
			if (msg[1] == organfile->GetPiston(j)->MIDIProgramChangeNumber - 1)
				organfile->GetPiston(j)->Push();
	}
}

