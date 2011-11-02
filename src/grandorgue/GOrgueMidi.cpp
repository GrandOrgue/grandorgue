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
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"
#include "RtMidi.h"
#include <vector>

#define DELETE_AND_NULL(x) do { if (x) { delete x; x = NULL; } } while (0)

GOrgueMidi::GOrgueMidi(GOrgueSettings& settings) :
	m_Settings(settings),
	m_midi_device_map(),
	m_midi_devices(),
	m_listening(false),
	m_listen_evthandler(NULL),
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
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}
}


GOrgueMidi::~GOrgueMidi()
{
	try
	{
		/* dispose of all midi devices */
		for (unsigned i = 0; i < m_midi_devices.size(); i++)
		{
			wxMutexGuiLeave();
			{
				wxCriticalSectionLocker locker(m_CloseLock);
				m_midi_devices[i]->active = false;
			}
			wxMutexGuiEnter();

			if (m_midi_devices[i]->midi_in)
				m_midi_devices[i]->midi_in->closePort();
			DELETE_AND_NULL(m_midi_devices[i]->midi_in);
		}
	}
	catch (RtError &e)
	{
		wxString error = wxString::FromAscii(e.getMessage().c_str());
		wxLogError(_("RtMidi error: %s"), error.c_str());
	}

}

void GOrgueMidi::Open()
{
	UpdateDevices();

	for (unsigned i = 0; i < m_midi_devices.size(); i++)
	{
		MIDI_DEVICE& this_dev = *m_midi_devices[i];
		memset(this_dev.bank_msb, 0, sizeof(this_dev.bank_msb));
		memset(this_dev.bank_lsb, 0, sizeof(this_dev.bank_lsb));
		int channel_shift = m_Settings.GetMidiDeviceChannelShift(this_dev.name);
		if (channel_shift >= 0)
		{
			try
			{
				wxMutexGuiLeave();
				{
					wxCriticalSectionLocker locker(m_CloseLock);
					this_dev.active = false;
				}
				wxMutexGuiEnter();

				assert(this_dev.midi_in);
				this_dev.channel_shift = channel_shift;
				this_dev.midi_in->closePort();
				this_dev.midi_in->openPort(this_dev.rtmidi_port_no);
				this_dev.active = true;
			}
			catch (RtError &e)
			{
				wxString error = wxString::FromAscii(e.getMessage().c_str());
				wxLogError(_("RtMidi error: %s"), error.c_str());
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

void GOrgueMidi::ProcessMessage(std::vector<unsigned char>& msg, MIDI_DEVICE* device)
{
	/* To avoid deadlocks while closing - must be the first one */
	wxCriticalSectionLocker close_locker(m_CloseLock);
	if (!device->active)
		return;

	/* Aquire first GUI lock and then MIDI state lock */
	wxMutexGuiLocker gui_lock;
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

	if (e.GetMidiType() == MIDI_RESET && m_organfile)
	{
		m_organfile->Reset();
		return;
	}

	if (m_listening)
		m_listen_evthandler->AddPendingEvent(e);

	if (m_organfile)
		m_organfile->ProcessMidi(e);

	int j = e.GetEventCode();
	if (j == -1)
		return;
	// MIDI for different organ??
	std::map<long, wxString>::const_iterator it = m_Settings.GetOrganList().find(j);
	if (it != m_Settings.GetOrganList().end())
	{
		wxCommandEvent event(wxEVT_LOADFILE, 0);
		event.SetString(it->second);
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
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

void GOrgueMidi::MIDICallback (double timeStamp, std::vector<unsigned char>* msg, void* userData)
{
	MIDI_DEVICE* m_dev = (MIDI_DEVICE*)userData;
	m_dev->midi->ProcessMessage(*msg, m_dev);
}
