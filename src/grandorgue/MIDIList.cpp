/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#include "MIDIList.h"

#include "GOrgueEvent.h"
#include "GrandOrgueFile.h"
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

BEGIN_EVENT_TABLE(MIDIList, wxDialog)
        EVT_LIST_ITEM_SELECTED(ID_LIST, MIDIList::OnObjectClick)
        EVT_LIST_ITEM_ACTIVATED(ID_LIST, MIDIList::OnObjectDoubleClick)
        EVT_BUTTON(ID_STATUS, MIDIList::OnStatus)
        EVT_BUTTON(ID_EDIT, MIDIList::OnEdit)
        EVT_BUTTON(wxID_OK, MIDIList::OnOK)
        EVT_COMMAND_RANGE(ID_BUTTON, ID_BUTTON_LAST, wxEVT_BUTTON, MIDIList::OnButton)
END_EVENT_TABLE()

MIDIList::MIDIList (GOrgueDocumentBase* doc, wxWindow* parent, GrandOrgueFile* organfile) :
	wxDialog(parent, wxID_ANY, _("MIDI Objects"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	GOrgueView(doc, this)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_Objects = new wxListView(this, ID_LIST, wxDefaultPosition, wxSize(300,600), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	m_Objects->InsertColumn(0, _("Type"));
	m_Objects->InsertColumn(1, _("Element"));
	topSizer->Add(m_Objects, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	for(unsigned id = ID_BUTTON; id <= ID_BUTTON_LAST; id++)
	{
		wxButton* button = new wxButton(this, id, wxEmptyString);
		sizer->Add(button, 0, id == ID_BUTTON ? wxRESERVE_SPACE_EVEN_IF_HIDDEN : 0);
		button->Hide();
		m_Buttons.push_back(button);
	}
	topSizer->Add(sizer, 0, wxALIGN_LEFT | wxALL, 1);

	wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
	m_Edit = new wxButton(this, ID_EDIT, _("C&onfigure..."));
	m_Edit->Disable();
	buttons->Add(m_Edit);
	m_Status = new wxButton(this, ID_STATUS, _("&Status"));
	m_Status->Disable();
	buttons->Add(m_Status);
	wxButton* close = new wxButton(this, wxID_OK, _("&Close"));
	buttons->Add(close);
	topSizer->Add(buttons, 0, wxALIGN_RIGHT | wxALL, 1);

	for(unsigned i = 0; i < organfile->GetMidiConfiguratorCount(); i++)
	{
		GOrgueMidiConfigurator* obj = organfile->GetMidiConfigurator(i);
		m_Objects->InsertItem(i, obj->GetMidiType());
		m_Objects->SetItemPtrData(i, (wxUIntPtr)obj);
		m_Objects->SetItem(i, 1, obj->GetMidiName());
	}

	m_Objects->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_Objects->SetColumnWidth(1, wxLIST_AUTOSIZE);

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

MIDIList::~MIDIList()
{
}

void MIDIList::OnButton(wxCommandEvent& event)
{
	GOrgueMidiConfigurator* obj = (GOrgueMidiConfigurator*)m_Objects->GetItemData(m_Objects->GetFirstSelected());
	obj->TriggerElementActions(event.GetId() - ID_BUTTON);
}

void MIDIList::OnStatus(wxCommandEvent& event)
{
	GOrgueMidiConfigurator* obj = (GOrgueMidiConfigurator*)m_Objects->GetItemData(m_Objects->GetFirstSelected());
	wxString status = obj->GetElementStatus();
	GOMessageBox(wxString::Format(_("Status: %s"), status), obj->GetMidiType() + _(" ") + obj->GetMidiName(), wxOK);
}

void MIDIList::OnOK(wxCommandEvent& event)
{
	Destroy();
}

void MIDIList::OnObjectClick(wxListEvent& event)
{
	m_Edit->Enable();
	m_Status->Enable();
	GOrgueMidiConfigurator* obj = (GOrgueMidiConfigurator*)m_Objects->GetItemData(m_Objects->GetFirstSelected());
	std::vector<wxString> actions = obj->GetElementActions();
	for(unsigned i = 0; i < m_Buttons.size(); i++)
		if (i < actions.size())
		{
			m_Buttons[i]->SetLabel(actions[i]);
			m_Buttons[i]->Show();
		}
		else
			m_Buttons[i]->Hide();
	Layout();
}

void MIDIList::OnObjectDoubleClick(wxListEvent& event)
{
	GOrgueMidiConfigurator* obj = (GOrgueMidiConfigurator*)m_Objects->GetItemData(m_Objects->GetFirstSelected());
	obj->ShowConfigDialog();
}

void MIDIList::OnEdit(wxCommandEvent& event)
{
	wxListEvent listevent;
	OnObjectDoubleClick(listevent);

}

