/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "SettingsOrgan.h"
#include "GOrgueSettings.h"
#include "MIDIEventDialog.h"

BEGIN_EVENT_TABLE(SettingsOrgan, wxPanel)
	EVT_LIST_ITEM_SELECTED(ID_ORGANS, SettingsOrgan::OnOrganSelected)
	EVT_BUTTON(ID_UP, SettingsOrgan::OnUp)
	EVT_BUTTON(ID_DOWN, SettingsOrgan::OnDown)
	EVT_BUTTON(ID_TOP, SettingsOrgan::OnTop)
	EVT_BUTTON(ID_DEL, SettingsOrgan::OnDel)
	EVT_BUTTON(ID_PROPERTIES, SettingsOrgan::OnProperties)
END_EVENT_TABLE()

SettingsOrgan::SettingsOrgan(GOrgueSettings& settings, GOrgueMidi& midi, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Settings(settings),
	m_midi(midi)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_Organs = new wxListView(this, ID_ORGANS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	m_Organs->InsertColumn(0, _("Church"));
	m_Organs->InsertColumn(1, _("Builder"));
	m_Organs->InsertColumn(2, _("Recording"));
	m_Organs->InsertColumn(3, _("MIDI"));
	m_Organs->InsertColumn(4, _("ODF Path"));
	topSizer->Add(m_Organs, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* buttonSizer =  new wxBoxSizer(wxHORIZONTAL);
	m_Properties = new wxButton(this, ID_PROPERTIES, _("P&roperties..."));
	m_Down = new wxButton(this, ID_DOWN, _("&Down"));
	m_Up = new wxButton(this, ID_UP, _("&Up"));
	m_Top = new wxButton(this, ID_TOP, _("&Top"));
	m_Del = new wxButton(this, ID_DEL, _("&Delete"));
	buttonSizer->Add(m_Down, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Up, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Top, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Del, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Properties, 0, wxALIGN_RIGHT | wxALL, 5);
	topSizer->Add(buttonSizer, 0, wxALL, 5);

	for(unsigned i = 0; i < m_Settings.GetOrganList().size(); i++)
	{
		GOrgueOrgan* o = m_Settings.GetOrganList()[i];
		m_Organs->InsertItem(i, o->GetChurchName());
		m_Organs->SetItemPtrData(i, (wxUIntPtr)o);
		m_Organs->SetItem(i, 1, o->GetOrganBuilder());
		m_Organs->SetItem(i, 2, o->GetRecordingDetail());
		m_Organs->SetItem(i, 3, o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No") );
		m_Organs->SetItem(i, 4, o->GetODFPath());
	}
	if (m_Organs->GetItemCount())
	{
		m_Organs->SetColumnWidth(0, 150);
		m_Organs->SetColumnWidth(1, 150);
		m_Organs->SetColumnWidth(2, 250);
		m_Organs->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
		m_Organs->SetColumnWidth(4, wxLIST_AUTOSIZE);
	}
	m_Up->Disable();
	m_Down->Disable();
	m_Top->Disable();
	m_Del->Disable();
	m_Properties->Disable();

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

void SettingsOrgan::OnOrganSelected(wxListEvent& event)
{
	long index = m_Organs->GetFirstSelected();
	m_Del->Enable();
	m_Properties->Enable();
	if (index <= 0)
	{
		m_Top->Disable();
		m_Up->Disable();
	}
	else
	{
		m_Top->Enable();
		m_Up->Enable();
	}
	if (m_Organs->GetItemCount() > 1 && index >= 0 && index + 1 < m_Organs->GetItemCount())
		m_Down->Enable();
	else
		m_Down->Disable();
}

void SettingsOrgan::MoveOrgan(long from, long to)
{
	wxListItem item;
	item.SetId(from);
	item.SetMask(-1);
	m_Organs->GetItem(item);
	item.SetId(to);
	m_Organs->InsertItem(item);
	if (to < from)
		from++;
	for(unsigned i = 0; i < 5; i++)
	{
		item.SetId(from);
		item.SetColumn(i);
		m_Organs->GetItem(item);
		item.SetId(to);
		m_Organs->SetItem(item);
	}
	m_Organs->SetItemPtrData(to, m_Organs->GetItemData(from));
	m_Organs->DeleteItem(from);
	m_Organs->Select(to);
}

void SettingsOrgan::OnUp(wxCommandEvent& event)
{
	MoveOrgan(m_Organs->GetFirstSelected(), m_Organs->GetFirstSelected() - 1);
}

void SettingsOrgan::OnDown(wxCommandEvent& event)
{
	MoveOrgan(m_Organs->GetFirstSelected(), m_Organs->GetFirstSelected() + 1);
}

void SettingsOrgan::OnTop(wxCommandEvent& event)
{
	MoveOrgan(m_Organs->GetFirstSelected(), 0);
}

void SettingsOrgan::OnDel(wxCommandEvent& event)
{
	if (wxMessageBox(wxString::Format(_("Do you want to remove %s?"), m_Organs->GetItemText(m_Organs->GetFirstSelected()).c_str()), _("Organs"), wxYES_NO | wxICON_EXCLAMATION, this) == wxYES)
	{
		m_Organs->DeleteItem(m_Organs->GetFirstSelected());
		m_Up->Disable();
		m_Down->Disable();
		m_Top->Disable();
		m_Del->Disable();
		m_Properties->Disable();
	}
}

void SettingsOrgan::OnProperties(wxCommandEvent& event)
{
	GOrgueOrgan* o = (GOrgueOrgan*)m_Organs->GetItemData(m_Organs->GetFirstSelected());
	MIDIEventDialog dlg(NULL, this, wxString::Format(_("MIDI settings for organ %s"), o->GetChurchName().c_str()), m_Settings, &o->GetMIDIReceiver(), NULL, NULL);
	dlg.RegisterMIDIListener(&m_midi);
	dlg.ShowModal();
	m_Organs->SetItem(m_Organs->GetFirstSelected(), 3, o->GetMIDIReceiver().GetEventCount() > 0 ? _("Yes") : _("No") );
}

void SettingsOrgan::Save()
{
	ptr_vector<GOrgueOrgan>& list = m_Settings.GetOrganList();
	for(unsigned i = 0; i < list.size(); i++)
	{
		bool found = false;
		for(long j = 0; j < m_Organs->GetItemCount(); j++)
			if (m_Organs->GetItemData(j) == (wxUIntPtr)list[i])
				found = true;
		if (!found)
			delete list[i];
		list[i] = 0;
	}
	list.clear();
	for(long i = 0; i < m_Organs->GetItemCount(); i++)
		list.push_back((GOrgueOrgan*)m_Organs->GetItemData(i));
}
