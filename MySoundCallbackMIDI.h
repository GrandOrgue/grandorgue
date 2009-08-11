/*
 * GrandOrgue - Copyright (C) 2009 JLDER - free pipe organ simulator based on MyOrgan Copyright (C) 2006 Kloria Publishing LLC
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

#pragma once

void MySound::MIDIPretend(bool on)
{
	for (int i = organfile->FirstManual; i <= organfile->NumberOfManuals; i++)
		for (int j = 0; j < organfile->manual[i].NumberOfLogicalKeys; j++)
			if (organfile->manual[i].m_MIDI[j] & 1)
				organfile->manual[i].Set(j + organfile->manual[i].FirstAccessibleKeyMIDINoteNumber, on, true);
}

void MIDIAllNotesOff()
{
	int i, j, k;

	if (!organfile)
		return;

	for (i = organfile->FirstManual; i <= organfile->NumberOfManuals; i++)
	{
		for (j = 0; j < organfile->manual[i].NumberOfAccessibleKeys; j++)
            organfile->manual[i].m_MIDI[j] = 0;
		for (j = 0; j < organfile->manual[i].NumberOfAccessibleKeys; j++)
		{
			wxCommandEvent event(wxEVT_NOTEONOFF, 0);
			event.SetInt(i);
			event.SetExtraLong(j);
			::wxGetApp().frame->AddPendingEvent(event);
		}
		for (j = 0; j < organfile->manual[i].NumberOfStops; j++)
		{
			for (k = 0; k < organfile->manual[i].stop[j].NumberOfLogicalPipes; k++)
			{
				register MyPipe* pipe = organfile->pipe[organfile->manual[i].stop[j].pipe[k]];
				if (pipe->instances > -1)
					pipe->instances = 0;
				pipe->sampler = 0;
			}
			if (organfile->manual[i].stop[j].m_auto)
                organfile->manual[i].stop[j].Set(false);
		}
	}
}

void MySoundCallbackMIDI(std::vector<unsigned char>& msg, int which)
{
	register int i, j, k, q, Noteoffset;

    Noteoffset = 0;
    // MIDI message FF : reset
    if (msg[0] == 0xFF && g_sound->b_active && organfile)
	{
        for (k = organfile->FirstManual; k <= organfile->NumberOfManuals; k++)
        {
            for (j = 0; j < organfile->manual[k].NumberOfStops; j++)
                organfile->manual[k].stop[j].Set(false);
            for (j = 0; j < organfile->manual[k].NumberOfCouplers; j++)
                organfile->manual[k].coupler[j].Set(false);
            for (j = 0; j < organfile->manual[k].NumberOfDivisionals; j++)
            {
                MyDivisional *divisional = organfile->manual[k].divisional + j;
                if (divisional->DispImageNum & 2)
                {
                    divisional->DispImageNum &= 1;
                    wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
                    event.SetClientData(divisional);
                    ::wxGetApp().frame->AddPendingEvent(event);
                }
            }

        }
        for (j = 0; j < organfile->NumberOfTremulants; j++)
            organfile->tremulant[j].Set(false);
        for (j = 0; j < organfile->NumberOfDivisionalCouplers; j++)
            organfile->divisionalcoupler[j].Set(false);

        for (k = 0; k < organfile->NumberOfGenerals; k++)
        {
            MyGeneral *general = organfile->general + k;
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
	msg[0] = c | ((msg[0] + g_sound->i_midiDevices[which]) & 0x0F); // msg[0] = MIDI code + channel from device + channel offset

	j = msg[0] << 8; // j = channel in higher byte
	if (msg.size() > 1) // if midi meesage has data
		j |= msg[1]; // j = channel in higher byte and first byte of MIDI data in lower byte
	if (g_sound->b_listening)
	{
		wxCommandEvent event(wxEVT_LISTENING, 0);
		event.SetInt(j);
		g_sound->listen_evthandler->AddPendingEvent(event);
	}

	if (!g_sound->b_active || msg.size() < 2)
		return;
    // MIDI code for controller
	if (c == 0xB0 && (msg[1] == 120 || msg[1] == 123) && organfile)
	{
        for (i = organfile->FirstManual; i <= organfile->NumberOfManuals; i++)
            if (msg[0] == ((g_sound->i_midiEvents[organfile->manual[i].MIDIInputNumber + 7] | 0xB000) >> 8))
                for (j = 0; j < organfile->manual[i].NumberOfAccessibleKeys; j++)
                    if (organfile->manual[i].m_MIDI[j] == 1)
                        organfile->manual[i].Set(j + organfile->manual[i].FirstAccessibleKeyMIDINoteNumber, false);
        return;
	}

	// MIDI code for memory set
	if (g_sound->i_midiEvents[15] == j && (((j & 0xF000) == 0xC000) || g_sound->b_memset ^ (bool)msg[2]))
	{
        ::wxGetApp().frame->ProcessCommand(ID_AUDIO_MEMSET);
		::wxGetApp().frame->UpdateWindowUI();
	}

    // MIDI for different organ??
	std::map<long, wxString>::iterator it;
	it=g_sound->organmidiEvents.find(j);
	if (it!=g_sound->organmidiEvents.end()) {
//	    wxMessageDialog dlg(::wxGetApp().frame, it->second);
//	    dlg.ShowModal();
        wxCommandEvent event(wxEVT_LOADFILE, 0);
        event.SetString(it->second);
        ::wxGetApp().frame->AddPendingEvent(event);
//        ::wxGetApp().frame->ProcessCommand(wxEVT_LOADFILE);
//        organfile->Load(it->second);
	}

	// For the 16 MIDI devices
	int myevent, offset;
	for (i = 0; i < 15; i++)
	{
		// ??
		if (i == 8 || (i == 2 && msg.size() < 3))
		{
			j &= 0xFF00;
			if (i != 8)
                i = 14;
		}
		myevent = g_sound->i_midiEvents[i];
		if (i>=8 && i<=13) myevent &= 0xFF00;
  //      if ((g_sound->i_midiEvents[i] & 0xFF00) == j )
        if (myevent == j)
		{
			if (i < 2)
			{
				if (msg.size() > 2 && !msg[2])
					continue;
				MyMeter* meter = ::wxGetApp().frame->m_meters[2];
				meter->SetValue(meter->GetValue() + (i ? 1 : -1));
			}
			else if (i < 8)
			{
				q = i - 1;
				for (k = 0; k < organfile->NumberOfEnclosures; k++)
					if (organfile->enclosure[k].MIDIInputNumber == q)
						organfile->enclosure[k].Set(msg[2]);
			}
			else if (i < 14)
			{
				q = i - 7;
				for (k = organfile->FirstManual; k <= organfile->NumberOfManuals; k++)
				{
				    if (!organfile->manual[k].Displayed)
                        continue;
                    offset = g_sound->i_midiEvents[i] & 0xFF;
                    if  (offset > 127) offset = offset - 140;
					if (organfile->manual[k].MIDIInputNumber == q)
						organfile->manual[k].Set(msg[1]+offset+g_sound->transpose, msg[2] ? true : false);
				}
			}
			else
			{
			    if ((c == 0x90 || c == 0xB0) && msg.size() == 3)
			    {
                    for (k = organfile->FirstManual; k <= organfile->NumberOfManuals; k++)
                    {
                        for (j = 0; j < organfile->manual[k].NumberOfStops; j++)
                            if (msg[1] == organfile->manual[k].stop[j].StopControlMIDIKeyNumber)
                                organfile->manual[k].stop[j].Set(msg[2] ? true : false);
                        for (j = 0; j < organfile->manual[k].NumberOfCouplers; j++)
                            if (msg[1] == organfile->manual[k].coupler[j].StopControlMIDIKeyNumber)
                                organfile->manual[k].coupler[j].Set(msg[2] ? true : false);
                    }
                    for (j = 0; j < organfile->NumberOfTremulants; j++)
                        if (msg[1] == organfile->tremulant[j].StopControlMIDIKeyNumber)
                            organfile->tremulant[j].Set(msg[2] ? true : false);
                    for (j = 0; j < organfile->NumberOfDivisionalCouplers; j++)
                        if (msg[1] == organfile->divisionalcoupler[j].StopControlMIDIKeyNumber)
                            organfile->divisionalcoupler[j].Set(msg[2] ? true : false);
			    }
			    else if (c == 0xC0 && msg.size() == 2)
			    {
                    for (k = organfile->FirstManual; k <= organfile->NumberOfManuals; k++)
                    {
                        for (j = 0; j < organfile->manual[k].NumberOfStops; j++)
                            if (msg[1] == organfile->manual[k].stop[j].StopControlMIDIKeyNumber)
                                organfile->manual[k].stop[j].Push();
                        for (j = 0; j < organfile->manual[k].NumberOfCouplers; j++)
                            if (msg[1] == organfile->manual[k].coupler[j].StopControlMIDIKeyNumber)
                                organfile->manual[k].coupler[j].Push();
                    }
                    for (j = 0; j < organfile->NumberOfTremulants; j++)
                        if (msg[1] == organfile->tremulant[j].StopControlMIDIKeyNumber)
                            organfile->tremulant[j].Push();
                    for (j = 0; j < organfile->NumberOfDivisionalCouplers; j++)
                        if (msg[1] == organfile->divisionalcoupler[j].StopControlMIDIKeyNumber)
                            organfile->divisionalcoupler[j].Push();
			    }
			}
		}
	}

	if (c == 0xC0)
	{
		for (k = organfile->FirstManual; k <= organfile->NumberOfManuals; k++)
			for (j = 0; j < organfile->manual[k].NumberOfDivisionals; j++)
				if (msg[0] == (g_sound->i_midiEvents[organfile->manual[k].MIDIInputNumber + 7] ^ 0x5000) >> 8 && msg[1] == organfile->manual[k].divisional[j].MIDIProgramChangeNumber - 1)
					organfile->manual[k].divisional[j].Push();
		for (j = 0; j < organfile->NumberOfGenerals; j++)
			if (msg[1] == organfile->general[j].MIDIProgramChangeNumber - 1)
				organfile->general[j].Push();
		for (j = 0; j < organfile->NumberOfReversiblePistons; j++)
			if (msg[1] == organfile->piston[j].MIDIProgramChangeNumber - 1)
				organfile->piston[j].Push();
	}
}
