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
 * MA 02111-1307, USA.
 */

#include "SettingsOrgan.h"
#include "GOrgueSettings.h"
#include "MIDIListenDialog.h"

BEGIN_EVENT_TABLE(SettingsOrgan, wxPanel)
	EVT_LIST_ITEM_SELECTED(ID_ORGANS, SettingsOrgan::OnEventsClick)
	EVT_BUTTON(ID_ADD, SettingsOrgan::OnAdd)
	EVT_BUTTON(ID_DEL, SettingsOrgan::OnDel)
	EVT_BUTTON(ID_PROPERTIES, SettingsOrgan::OnProperties)
END_EVENT_TABLE()

SettingsOrgan::SettingsOrgan(GOrgueSettings& settings, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Settings(settings)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_Events = new wxListView(this, ID_ORGANS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	m_Events->InsertColumn(0, _("Organ"));
	m_Events->InsertColumn(1, _("Event"));
	m_Events->InsertColumn(2, _("Channel"));
	m_Events->InsertColumn(3, _("Data"));
	topSizer->Add(m_Events, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* buttonSizer =  new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddSpacer(5);
	m_Properties = new wxButton(this, ID_PROPERTIES, _("P&roperties..."));
	m_Properties->Disable();
	m_Add = new wxButton(this, ID_ADD, _("&Add"));
	m_Del = new wxButton(this, ID_DEL, _("&Delete"));
	m_Del->Disable();

	buttonSizer->Add(m_Properties, 0, wxALIGN_RIGHT | wxALL, 5);
	buttonSizer->Add(m_Add, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Del, 0, wxALIGN_LEFT | wxALL, 5);
	topSizer->Add(buttonSizer, 0, wxALL, 5);

	std::map<long, wxString>::const_iterator it = m_Settings.GetOrganList().begin();
	for(; it != m_Settings.GetOrganList().end(); it++)
	{
		unsigned i = m_Events->GetItemCount();
		m_Events->InsertItem(i, it->second);
		m_Events->SetItemData(i, it->first);
		UpdateOrganMessages(i);
	}
	if (m_Events->GetItemCount())
	{
		m_Events->SetColumnWidth(0, wxLIST_AUTOSIZE);
		m_Events->SetColumnWidth(1, wxLIST_AUTOSIZE);
		m_Events->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
		m_Events->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	}

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

void SettingsOrgan::OnEventsClick(wxListEvent& event)
{
	m_Del->Enable();
	m_Properties->Enable();
}

void SettingsOrgan::OnAdd(wxCommandEvent& event)
{
	wxFileDialog dlg(this, _("Choose a file"), wxT(""), wxT(""), _("*.organ"), wxFD_FILE_MUST_EXIST | wxFD_OPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_Events->InsertItem(m_Events->GetItemCount(), dlg.GetPath());
		m_Events->SetColumnWidth(0, wxLIST_AUTOSIZE);
	}
}

void SettingsOrgan::OnDel(wxCommandEvent& event)
{
	int index = m_Events->GetFirstSelected();
	if (index != -1)
		m_Events->DeleteItem(index);
	if (!m_Events->GetItemCount())
	{
		m_Del->Disable();
		m_Properties->Disable();
	}

}

void SettingsOrgan::OnProperties(wxCommandEvent& event)
{
	int index = m_Events->GetFirstSelected();
	if (index == -1)
		return;

	MIDIListenDialog dlg
		(this
		,m_Events->GetItemText(index)
		,MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN
		,m_Events->GetItemData(index)
		);

	if (dlg.ShowModal() == wxID_OK)
	{
		m_Events->SetItemData(index, dlg.GetEvent());
		UpdateOrganMessages(index);
		m_Events->SetColumnWidth(1, wxLIST_AUTOSIZE);
		m_Events->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
		m_Events->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	}

}

void SettingsOrgan::UpdateOrganMessages(int i)
{
	int j = m_Events->GetItemData(i);
	m_Events->SetItem(i, 1, MIDIListenDialog::GetEventTitle(j, MIDIListenDialog::LSTN_SETTINGSDLG_MEMORY_OR_ORGAN));
	m_Events->SetItem(i, 2, MIDIListenDialog::GetEventChannelString(j));
	// data
	wxString temp;
	int offset = (j & 0xF000) == 0xC000 ? 1 : 0;
	temp << ((j & 0x7F) + offset);
	m_Events->SetItem(i, 3, temp);
	m_Events->SetItemData(i, j);
}

void SettingsOrgan::Save()
{
	std::map<long, wxString> list;
	for (int i=0; i < m_Events->GetItemCount(); i++)
		list.insert(std::pair<long, wxString>(m_Events->GetItemData(i), m_Events->GetItemText(i)));

	m_Settings.SetOrganList(list);
}
