/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include "SettingsMidiMessage.h"
#include "GOrgueSettings.h"

BEGIN_EVENT_TABLE(SettingsMidiMessage, wxPanel)
	EVT_LIST_ITEM_SELECTED(ID_EVENTS, SettingsMidiMessage::OnEventsClick)
	EVT_LIST_ITEM_ACTIVATED(ID_EVENTS, SettingsMidiMessage::OnEventsDoubleClick)
	EVT_BUTTON(ID_PROPERTIES, SettingsMidiMessage::OnProperties)
END_EVENT_TABLE()

SettingsMidiMessage::SettingsMidiMessage(GOrgueSettings& settings, GOrgueMidi& midi, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Settings(settings),
	m_midi(midi)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_Events = new wxListView(this, ID_EVENTS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	m_Events->InsertColumn(0, _("Action"));
	m_Events->InsertColumn(1, _("MIDI Event"));
	m_Events->InsertColumn(2, _("Channel"));
	m_Events->InsertColumn(3, _("Data/Offset"));
	topSizer->Add(m_Events, 1, wxEXPAND | wxALL, 5);
	m_Properties = new wxButton(this, ID_PROPERTIES, _("P&roperties..."));
	m_Properties->Disable();
	topSizer->Add(m_Properties, 0, wxALIGN_RIGHT | wxALL, 5);

	for (unsigned i = 0; i < GetEventCount(); i++)
	{
		m_Events->InsertItem(i, GetEventName(i));
		UpdateMessages(i);
	}

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);

	m_Events->SetColumnWidth(0, 150);
	m_Events->SetColumnWidth(1, 300);
	m_Events->SetColumnWidth(2, 62);
	m_Events->SetColumnWidth(3, 100);
}

unsigned SettingsMidiMessage::GetEventCount()
{
	return m_Settings.GetSetterCount() + m_Settings.GetEnclosureCount() +
		m_Settings.GetManualCount() + 1;
}

MIDIListenDialog::LISTEN_DIALOG_TYPE SettingsMidiMessage::GetEventType(unsigned index)
{
	if (index < m_Settings.GetSetterCount())
		return MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN;
	index -= m_Settings.GetSetterCount();

	if (index < m_Settings.GetEnclosureCount())
		return MIDIListenDialog::LSTN_ENCLOSURE;
	index -= m_Settings.GetEnclosureCount();

	if (index < m_Settings.GetManualCount())
		return MIDIListenDialog::LSTN_MANUAL;
	index -= m_Settings.GetManualCount();

	return MIDIListenDialog::LSTN_SETTINGSDLG_STOP_CHANGE;
}

wxString SettingsMidiMessage::GetEventName(unsigned index)
{
	if (index < m_Settings.GetSetterCount())
		return m_Settings.GetSetterTitle(index);
	index -= m_Settings.GetSetterCount();

	if (index < m_Settings.GetEnclosureCount())
		return m_Settings.GetEnclosureTitle(index);
	index -= m_Settings.GetEnclosureCount();

	if (index < m_Settings.GetManualCount())
		return m_Settings.GetManualTitle(index);
	index -= m_Settings.GetManualCount();

	return m_Settings.GetStopChangeTitle();
}

int SettingsMidiMessage::GetEventData(unsigned index)
{
	if (index < m_Settings.GetSetterCount())
		return m_Settings.GetSetterEvent(index);
	index -= m_Settings.GetSetterCount();

	if (index < m_Settings.GetEnclosureCount())
		return m_Settings.GetEnclosureEvent(index);
	index -= m_Settings.GetEnclosureCount();

	if (index < m_Settings.GetManualCount())
		return m_Settings.GetManualEvent(index);
	index -= m_Settings.GetManualCount();

	return m_Settings.GetStopChangeEvent();
}

void SettingsMidiMessage::SetEventData(unsigned index, int event)
{
	if (index < m_Settings.GetSetterCount())
	{
		m_Settings.SetSetterEvent(index, event);
		return;
	}
	index -= m_Settings.GetSetterCount();

	if (index < m_Settings.GetEnclosureCount())
	{
		m_Settings.SetEnclosureEvent(index, event);
		return;
	}
	index -= m_Settings.GetEnclosureCount();

	if (index < m_Settings.GetManualCount())
	{
		m_Settings.SetManualEvent(index, event);
		return;
	}
	index -= m_Settings.GetManualCount();

	m_Settings.SetStopChangeEvent(event);
}

void SettingsMidiMessage::UpdateMessages(unsigned i)
{
	MIDIListenDialog::LISTEN_DIALOG_TYPE type = GetEventType(i);
	int j = GetEventData(i);
	m_Events->SetItem(i, 1, MIDIListenDialog::GetEventTitle(j, type));
	m_Events->SetItem(i, 2, MIDIListenDialog::GetEventChannelString(j));

	if (type != MIDIListenDialog::LSTN_SETTINGSDLG_STOP_CHANGE && j)
	{
		int offset = (j & 0xF000) == 0xC000 ? 1 : 0;
		wxString temp;
		if (type == 1 ) // when type == 1 (keyboards) data contained in lower byte correspond to note offset
		{
			if ((j&0xFF)>0x7F) offset = -140; // negative numbers are coded as 128=-12 140=0
		}
		temp << ((j & 0xFF) + offset );
		m_Events->SetItem(i, 3, temp);
	}

	m_Events->SetItemData(i, j);
}

void SettingsMidiMessage::OnEventsClick(wxListEvent& event)
{
	m_Properties->Enable();
}

void SettingsMidiMessage::OnEventsDoubleClick(wxListEvent& event)
{
	m_Properties->Enable();
	int index = m_Events->GetFirstSelected();

	MIDIListenDialog dlg(this, m_Events->GetItemText(index), m_midi, GetEventType(index), m_Events->GetItemData(index));

	if (dlg.ShowModal() == wxID_OK)
	{
		SetEventData(index, dlg.GetEvent());
		UpdateMessages(index);
	}
}

void SettingsMidiMessage::OnProperties(wxCommandEvent& event)
{
	wxListEvent listevent;
	OnEventsDoubleClick(listevent);
}

