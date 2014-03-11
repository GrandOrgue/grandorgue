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

#include "GOrgueMidiMerger.h"

#include "GOrgueMidiEvent.h"
#include <string.h>

GOrgueMidiMerger::GOrgueMidiMerger()
{
	Clear();
}

void GOrgueMidiMerger::Clear()
{
	memset(m_BankMsb, 0, sizeof(m_BankMsb));
	memset(m_BankLsb, 0, sizeof(m_BankLsb));
	memset(m_RpnMsb, 0, sizeof(m_RpnMsb));
	memset(m_RpnLsb, 0, sizeof(m_RpnLsb));
	memset(m_NrpnMsb, 0, sizeof(m_NrpnMsb));
	memset(m_NrpnLsb, 0, sizeof(m_NrpnLsb));
	m_Rpn = false;
}

bool GOrgueMidiMerger::Process(GOrgueMidiEvent& e)
{
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_BANK_SELECT_MSB)
	{
		m_BankMsb[e.GetChannel() - 1] = e.GetValue();
		return false;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_BANK_SELECT_LSB)
	{
		m_BankLsb[e.GetChannel() - 1] = e.GetValue();
		return false;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_RPN_LSB)
	{
		m_RpnLsb[e.GetChannel() - 1] = e.GetValue();
		m_Rpn = true;
		return false;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_RPN_MSB)
	{
		m_RpnMsb[e.GetChannel() - 1] = e.GetValue();
		m_Rpn = true;
		return false;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_NRPN_LSB)
	{
		m_NrpnLsb[e.GetChannel() - 1] = e.GetValue();
		m_Rpn = false;
		return false;
	}
	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_NRPN_MSB)
	{
		m_NrpnMsb[e.GetChannel() - 1] = e.GetValue();
		m_Rpn = false;
		return false;
	}
	if (e.GetMidiType() == MIDI_PGM_CHANGE)
		e.SetKey(((e.GetKey() - 1) | (m_BankLsb[e.GetChannel() - 1] << 7) | (m_BankMsb[e.GetChannel() - 1] << 14)) + 1);

	if (e.GetMidiType() == MIDI_CTRL_CHANGE && e.GetKey() == MIDI_CTRL_DATA_ENTRY)
	{
		if (m_Rpn)
		{
			e.SetMidiType(MIDI_RPN);
			e.SetKey((m_RpnLsb[e.GetChannel() - 1] << 0) | (m_RpnMsb[e.GetChannel() - 1] << 7));
		}
		else
		{
			e.SetMidiType(MIDI_NRPN);
			e.SetKey((m_NrpnLsb[e.GetChannel() - 1] << 0) | (m_NrpnMsb[e.GetChannel() - 1] << 7));
		}
	}

	return true;
}
