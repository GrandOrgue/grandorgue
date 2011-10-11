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

#include "GOrgueEvent.h"
#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GrandOrgueFile.h"
#include "RtMidi.h"
#include <vector>
#include <wx/config.h>

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
	m_listen_evthandler(NULL),
	m_organ_midi_events(),
	m_organfile(NULL)
{
	UpdateDevices();
}

void GOrgueMidi::UpdateDevices()
{
	try
	{
		RtMidiIn midi_dev;
		for (unsigned i = 0; i <  midi_dev.getPortCount(); i++)
		{
			wxString name = wxString::FromAscii(midi_dev.getPortName(i).c_str());
			if (!m_midi_device_map.count(name))
			{
				MIDI_DEVICE *t = new MIDI_DEVICE;
				t->midi_in = new RtMidiIn();
				t->channel_shift = 0;
				t->midi = this;
				t->rtmidi_port_no = i;
				t->active = false;
				t->name = name;
				t->midi_in->setCallback(&MIDICallback, t);
				m_midi_devices.push_back(t);
			
				name.Replace(wxT("\\"), wxT("|"));
				m_midi_device_map[name] = m_midi_devices.size() - 1;
			}
			else
				m_midi_devices[m_midi_device_map[name]]->rtmidi_port_no = i;
		}
	}
	catch (RtError &e)
	{
		e.printMessage();
	}
}


GOrgueMidi::~GOrgueMidi()
{
	try
	{
		/* dispose of all midi devices */
		for (unsigned i = 0; i < m_midi_devices.size(); i++)
		{
			if (m_midi_devices[i]->midi_in)
				m_midi_devices[i]->midi_in->closePort();
			DELETE_AND_NULL(m_midi_devices[i]->midi_in);
		}
	}
	catch (RtError &e)
	{
		e.printMessage();
	}

}

void GOrgueMidi::Open()
{
	UpdateDevices();

	for (unsigned i = 0; i < NB_MIDI_EVENTS; i++)
		m_midi_events[i] = m_global_config->Read
			(wxString(wxT("MIDI/")) + g_available_midi_events[i].event_name
			,g_available_midi_events[i].default_mask
			);

	for (unsigned i = 0; i < m_midi_devices.size(); i++)
	{
		MIDI_DEVICE& this_dev = *m_midi_devices[i];
		memset(this_dev.bank_msb, 0, sizeof(this_dev.bank_msb));
		memset(this_dev.bank_lsb, 0, sizeof(this_dev.bank_lsb));
		int channel_shift = m_global_config->Read(wxT("Devices/MIDI/") + this_dev.name, 0L);
		if (channel_shift >= 0)
		{
			try
			{
				assert(this_dev.midi_in);
				this_dev.channel_shift = channel_shift;
				this_dev.midi_in->closePort();
				this_dev.midi_in->openPort(this_dev.rtmidi_port_no);
				this_dev.active = true;
			}
			catch (RtError &e)
			{
				e.printMessage();
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

void GOrgueMidi::SetOrganFile(GrandOrgueFile* organfile)
{
	wxCriticalSectionLocker locker(m_lock);

	m_organfile = organfile;
}

bool GOrgueMidi::HasActiveDevice()
{
	for (unsigned i = 0; i < m_midi_devices.size(); i++)
		if (m_midi_devices[i]->active)
			return true;

	return false;
}

int GOrgueMidi::GetTranspose()
{
	return m_transpose;
}

void GOrgueMidi::SetTranspose(int transpose)
{
	m_transpose = transpose;
}

void GOrgueMidi::ProcessMessage(std::vector<unsigned char>& msg, MIDI_DEVICE* device)
{
	wxCriticalSectionLocker locker(m_lock);
	GOrgueMidiEvent e;
	e.FromMidi(msg);
	if (e.GetMidiType() == MIDI_NONE)
		return;
	e.SetDevice(device->name);

	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_BANK_SELECT_MSB)
	{
		device->bank_msb[e.GetChannel() - 1] = e.GetValue();
		return;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_BANK_SELECT_LSB)
	{
		device->bank_lsb[e.GetChannel() - 1] = e.GetValue();
		return;
	}
	if (e.GetMidiType() == MIDI_PGM_CHANGE)
		e.SetKey(((e.GetKey() - 1) | (device->bank_lsb[e.GetChannel() - 1] << 7) | (device->bank_msb[e.GetChannel() - 1] << 14)) + 1);

	/* Compat stuff */
	if (e.GetChannel() != -1)
		e.SetChannel(((e.GetChannel() - 1 + device->channel_shift) & 0x0F) + 1);

	int j;

	if (e.GetMidiType() == MIDI_RESET && m_organfile)
	{
		m_organfile->Reset();
		return;
	}

	register unsigned char c = msg[0] & 0xF0; // c = MIDI message code without chanel number in c

	if (c == 0x80 && msg.size() == 3) // if MIDI code for note OFF and 2 data bytes
	{
		c ^= 0x10; // c = 0x90 , MIDI code note ON
		msg[2] = 0; // set velocity to zero
	}
	msg[0] = c | ((msg[0] + device->channel_shift) & 0x0F); // msg[0] = MIDI code + channel from device + channel offset

	j = msg[0] << 8; // j = channel in higher byte
	if (msg.size() > 1) // if midi meesage has data
		j |= msg[1]; // j = channel in higher byte and first byte of MIDI data in lower byte
	if (m_listening)
	{
		wxCommandEvent event(wxEVT_LISTENING, 0);
		event.SetInt(j);
		m_listen_evthandler->AddPendingEvent(event);
		m_listen_evthandler->AddPendingEvent(e);
	}

	if (m_organfile)
		m_organfile->ProcessMidi(e);

	// MIDI for different organ??
	std::map<long, wxString>::iterator it;
	it = m_organ_midi_events.find(j);
	if (it != m_organ_midi_events.end())
	{
		wxCommandEvent event(wxEVT_LOADFILE, 0);
		event.SetString(it->second);
		wxTheApp->GetTopWindow()->AddPendingEvent(event);
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

void GOrgueMidi::MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData)
{
	MIDI_DEVICE* m_dev = (MIDI_DEVICE*)userData;
	if (!m_dev->active)
		return;
	wxMutexGuiEnter();
	m_dev->midi->ProcessMessage(*msg, m_dev);
	wxMutexGuiLeave();
}
