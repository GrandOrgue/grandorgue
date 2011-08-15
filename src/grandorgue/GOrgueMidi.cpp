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

#include "GOrgueMidi.h"

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
#include <wx/config.h>

extern GrandOrgueFile* organfile;

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

typedef struct
{
	const wxChar*                              event_name;
	const long                                 default_mask;
	const MIDIListenDialog::LISTEN_DIALOG_TYPE listener_dialog_type;
} MIDI_EVENT;

static const MIDI_EVENT g_available_midi_events[NB_MIDI_EVENTS] =
{
	{wxTRANSLATE("Previous Memory"),   0xC400, MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN},
	{wxTRANSLATE("Next Memory"),       0xC401, MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN},
	{wxTRANSLATE("Enclosure 1"),       0xB001, MIDIListenDialog::LSTN_ENCLOSURE},
	{wxTRANSLATE("Enclosure 2"),       0xB101, MIDIListenDialog::LSTN_ENCLOSURE},
	{wxTRANSLATE("Enclosure 3"),       0x0000, MIDIListenDialog::LSTN_ENCLOSURE},
	{wxTRANSLATE("Enclosure 4"),       0x0000, MIDIListenDialog::LSTN_ENCLOSURE},
	{wxTRANSLATE("Enclosure 5"),       0x0000, MIDIListenDialog::LSTN_ENCLOSURE},
	{wxTRANSLATE("Enclosure 6"),       0x0000, MIDIListenDialog::LSTN_ENCLOSURE},
	{wxTRANSLATE("Manual 1 (Pedal)"),  0x9000, MIDIListenDialog::LSTN_MANUAL},
	{wxTRANSLATE("Manual 2 (Great)"),  0x9100, MIDIListenDialog::LSTN_MANUAL},
	{wxTRANSLATE("Manual 3 (Swell)"),  0x9200, MIDIListenDialog::LSTN_MANUAL},
	{wxTRANSLATE("Manual 4 (Choir)"),  0x9300, MIDIListenDialog::LSTN_MANUAL},
	{wxTRANSLATE("Manual 5 (Solo)"),   0x9500, MIDIListenDialog::LSTN_MANUAL},
	{wxTRANSLATE("Manual 6 (Echo)"),   0x9600, MIDIListenDialog::LSTN_MANUAL},
	{wxTRANSLATE("Stop Changes"),      0x9400, MIDIListenDialog::LSTN_SETTINGSDLG_STOP_CHANGE},
	{wxTRANSLATE("Memory Set"),        0x0000, MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN}
};

unsigned GOrgueMidi::NbMidiEvents()
{

	return NB_MIDI_EVENTS;

}

wxString GOrgueMidi::GetMidiEventTitle(const unsigned idx)
{

	assert(idx < NB_MIDI_EVENTS);
	return wxString(g_available_midi_events[idx].event_name);

}

wxString GOrgueMidi::GetMidiEventUserTitle(const unsigned idx)
{

	assert(idx < NB_MIDI_EVENTS);
	return wxGetTranslation(g_available_midi_events[idx].event_name);

}

MIDIListenDialog::LISTEN_DIALOG_TYPE GOrgueMidi::GetMidiEventListenDialogType(const unsigned idx)
{

	assert(idx < NB_MIDI_EVENTS);
	return g_available_midi_events[idx].listener_dialog_type;

}

GOrgueMidi::GOrgueMidi() :
	m_global_config(wxConfigBase::Get()),
	m_midi_device_map(),
	m_midi_devices(),
	m_transpose(0),
	m_listening(false),
	m_memset(false),
	m_listen_evthandler(NULL),
	m_organ_midi_events()
{

	RtMidiIn midi_dev;
	for (unsigned i = 0; i <  midi_dev.getPortCount(); i++)
	{

		MIDI_DEVICE t;
		t.midi_in = new RtMidiIn();
		t.id = 0;
		t.active = false;
		m_midi_devices.push_back(t);

		wxString name = wxString::FromAscii(t.midi_in->getPortName(i).c_str());
		name.Replace(wxT("\\"), wxT("|"));
		m_midi_device_map[name] = i;

	}

}

GOrgueMidi::~GOrgueMidi()
{

	try
	{

		/* dispose of all midi devices */
		while (m_midi_devices.size())
		{
			if (m_midi_devices.back().midi_in)
				m_midi_devices.back().midi_in->closePort();
			DELETE_AND_NULL(m_midi_devices.back().midi_in);
			m_midi_devices.pop_back();
		}

	}
	catch (RtError &e)
	{
		e.printMessage();
	}

}

void GOrgueMidi::Open()
{

	std::map<wxString, int>::iterator it2;

	for (it2 = m_midi_device_map.begin(); it2 != m_midi_device_map.end(); it2++)
		m_global_config->Read(wxT("Devices/MIDI/") + it2->first, -1);

	for (unsigned i = 0; i < NB_MIDI_EVENTS; i++)
		m_midi_events[i] = m_global_config->Read
			(wxString(wxT("MIDI/")) + g_available_midi_events[i].event_name
			,g_available_midi_events[i].default_mask
			);

	for (it2 = m_midi_device_map.begin(); it2 != m_midi_device_map.end(); it2++)
	{

		MIDI_DEVICE& this_dev = m_midi_devices[it2->second];
		int i = m_global_config->Read(wxT("Devices/MIDI/") + it2->first, 0L);
		if (i >= 0)
		{
			this_dev.id = i;
			if (!this_dev.active)
			{
				assert(this_dev.midi_in);
				this_dev.midi_in->openPort(it2->second);
				this_dev.active = true;
			}
		}
		else if (this_dev.active)
		{
			this_dev.active = false;
			this_dev.midi_in->closePort();
		}

	}

}

std::map<wxString, int>& GOrgueMidi::GetDevices()
{
	return m_midi_device_map;
}

bool GOrgueMidi::HasActiveDevice()
{

	for (unsigned i = 0; i < m_midi_devices.size(); i++)
		if (m_midi_devices[i].active)
			return true;

	return false;

}

void GOrgueMidi::SetTranspose(int transpose)
{
	m_transpose = transpose;
}

void
GOrgueMidi::ProcessMessage
	(const bool active
	,std::vector<unsigned char>& msg
	,int which
	)
{
	int i, j, k, q, Noteoffset;

    Noteoffset = 0;
    // MIDI message FF : reset
    if (msg[0] == 0xFF && active && organfile)
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
        for (unsigned l = 0; l < organfile->GetTremulantCount(); l++)
            organfile->GetTremulant(l)->Set(false);
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
	if (m_listening)
	{
		wxCommandEvent event(wxEVT_LISTENING, 0);
		event.SetInt(j);
		m_listen_evthandler->AddPendingEvent(event);
	}

	if (!active || msg.size() < 2)
		return;

	// MIDI code for controller
	if (c == 0xB0 && (msg[1] == 120 || msg[1] == 123) && organfile)
	{
        for (i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
            if (msg[0] == ((m_midi_events[organfile->GetManual(i)->GetMIDIInputNumber() + 7] | 0xB000) >> 8))
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
	if (m_midi_events[15] == j && (((j & 0xF000) == 0xC000) || m_memset ^ (bool)msg[2]))
	{
        ::wxGetApp().frame->ProcessCommand(ID_AUDIO_MEMSET);
		::wxGetApp().frame->UpdateWindowUI();
	}

    // MIDI for different organ??
	std::map<long, wxString>::iterator it;
	it = m_organ_midi_events.find(j);
	if (it != m_organ_midi_events.end())
	{
        wxCommandEvent event(wxEVT_LOADFILE, 0);
        event.SetString(it->second);
        ::wxGetApp().frame->AddPendingEvent(event);
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
		GOrgueevent = m_midi_events[i];
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
				    if (!organfile->GetManual(k)->IsDisplayed())
                        continue;
                    offset = m_midi_events[i] & 0xFF;
                    if  (offset > 127)
							offset = offset - 140;
					if (organfile->GetManual(k)->GetMIDIInputNumber() == q)
						organfile->GetManual(k)->Set(msg[1] + offset + m_transpose, msg[2] ? true : false);
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
                    for (unsigned l = 0; l < organfile->GetTremulantCount(); l++)
                        if (msg[1] == organfile->GetTremulant(l)->StopControlMIDIKeyNumber)
                            organfile->GetTremulant(l)->Set(msg[2] ? true : false);
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
                    for (unsigned l = 0; l < organfile->GetTremulantCount(); l++)
                        if (msg[1] == organfile->GetTremulant(l)->StopControlMIDIKeyNumber)
                            organfile->GetTremulant(l)->Push();
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
				if (msg[0] == (m_midi_events[organfile->GetManual(k)->GetMIDIInputNumber() + 7] ^ 0x5000) >> 8 && msg[1] == organfile->GetManual(k)->GetDivisional(j)->MIDIProgramChangeNumber - 1)
					organfile->GetManual(k)->GetDivisional(j)->Push();
		for (j = 0; j < organfile->GetGeneralCount(); j++)
			if (msg[1] == organfile->GetGeneral(j)->MIDIProgramChangeNumber - 1)
				organfile->GetGeneral(j)->Push();
		for (j = 0; j < organfile->GetNumberOfReversiblePistons(); j++)
			if (msg[1] == organfile->GetPiston(j)->MIDIProgramChangeNumber - 1)
				organfile->GetPiston(j)->Push();
	}
}


bool GOrgueMidi::HasListener()
{

	return (m_listen_evthandler) && (m_listening);

}

void GOrgueMidi::SetListener(wxEvtHandler* event_handler)
{

	m_listening = (event_handler != NULL);
	m_listen_evthandler = event_handler;

}

void GOrgueMidi::UpdateOrganMIDI()
{

	long count = m_global_config->Read(wxT("OrganMIDI/Count"), 0L);
	for (long i = 0; i < count; i++)
	{
		wxString itemstr = wxT("OrganMIDI/Organ") + wxString::Format(wxT("%ld"), i);
		long j = m_global_config->Read(itemstr + wxT(".midi"), 0L);
		wxString file = m_global_config->Read(itemstr + wxT(".file"));
		m_organ_midi_events.insert(std::pair<long, wxString>(j, file));
	}

}

bool GOrgueMidi::SetterActive()
{

	return m_memset;

}

int GOrgueMidi::GetMidiEventByChannel(int channel)
{

	return m_midi_events[channel];

}

int GOrgueMidi::GetStopMidiEvent()
{

	return m_midi_events[14];

}

int GOrgueMidi::GetManualMidiEvent(int manual_nb)
{

	return m_midi_events[7 + manual_nb];

}

void GOrgueMidi::ProcessMessages(const bool audio_active)
{

	/* Process any MIDI messages */
	std::vector<unsigned char> msg;
	for (unsigned j = 0; j < m_midi_devices.size(); j++)
	{

		if (!m_midi_devices[j].active)
			continue;

		for(;;)
		{
			m_midi_devices[j].midi_in->getMessage(&msg);
			if (msg.empty())
				break;
			ProcessMessage(audio_active, msg, j);
		}

	}

}

void GOrgueMidi::ToggleSetter()
{

	m_memset = !m_memset;

}
