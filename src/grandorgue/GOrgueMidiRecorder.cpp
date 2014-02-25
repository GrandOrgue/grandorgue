/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueMidiRecorder.h"

#include "GOrgueMidiEvent.h"
#include "GrandOrgueFile.h"

GOrgueMidiRecorder::GOrgueMidiRecorder(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_OutputDevice(0)
{
	Clear();
}

GOrgueMidiRecorder::~GOrgueMidiRecorder()
{
}

void GOrgueMidiRecorder::SetOutputDevice(unsigned device_id)
{
	m_OutputDevice = device_id;
}

void GOrgueMidiRecorder::Clear()
{
	m_Mappings.clear();
	m_NextChannel = 1;
	m_NextNRPN = 0;

	if (!m_OutputDevice)
		return;
	GOrgueMidiEvent e;
	e.SetMidiType(MIDI_SYSEX_GO_CLEAR);
	e.SetDevice(m_OutputDevice);
	m_organfile->SendMidiMessage(e);
}

void GOrgueMidiRecorder::SendMidiRecorderMessage(GOrgueMidiEvent& e)
{
	if (!m_OutputDevice)
		return;
	if (e.GetDevice() >= m_Mappings.size())
		m_Mappings.resize(e.GetDevice() + 1);

	if (e.GetDevice() != m_Mappings[e.GetDevice()].elementID)
	{
		if (e.GetMidiType() == MIDI_NRPN)
		{
			if (m_NextNRPN >= 1 << 18)
				return;
			m_Mappings[e.GetDevice()].elementID = e.GetDevice();
			m_Mappings[e.GetDevice()].channel = 1 + m_NextNRPN / (1 << 14);
			m_Mappings[e.GetDevice()].key = m_NextNRPN % (1 << 14);
			m_NextNRPN++;
		}
		else
		{
			if (m_NextChannel > 16)
				return;
			m_Mappings[e.GetDevice()].elementID = e.GetDevice();
			m_Mappings[e.GetDevice()].channel = m_NextChannel;
			m_Mappings[e.GetDevice()].key = 0;
			m_NextChannel++;
		}
		GOrgueMidiEvent e1;
		e1.SetMidiType(MIDI_SYSEX_GO_SETUP);
		e1.SetDevice(m_OutputDevice);
		e1.SetKey(m_Mappings[e.GetDevice()].elementID);
		e1.SetChannel(m_Mappings[e.GetDevice()].channel);
		e1.SetValue(m_Mappings[e.GetDevice()].key);
		m_organfile->SendMidiMessage(e1);
	}
	e.SetChannel(m_Mappings[e.GetDevice()].channel);
	if (e.GetMidiType() == MIDI_NRPN)
		e.SetKey(m_Mappings[e.GetDevice()].key);

	e.SetDevice(m_OutputDevice);
	m_organfile->SendMidiMessage(e);
}
