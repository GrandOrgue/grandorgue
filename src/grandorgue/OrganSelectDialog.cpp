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

#include "OrganSelectDialog.h"

#include "GOrgueSettings.h"
#include <wx/msgdlg.h>
#include <wx/sizer.h>

BEGIN_EVENT_TABLE(OrganSelectDialog, wxDialog)
	EVT_BUTTON(wxID_OK, OrganSelectDialog::OnOK)
	EVT_LIST_ITEM_ACTIVATED(ID_ORGANS, OrganSelectDialog::OnDoubleClick)
END_EVENT_TABLE()

OrganSelectDialog::OrganSelectDialog(wxWindow* parent, wxString title, GOrgueSettings& settings) :
	wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(700, 500)),
	m_Settings(settings)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_Organs = new wxListView(this, ID_ORGANS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	m_Organs->InsertColumn(0, _("Church"));
	m_Organs->InsertColumn(1, _("Builder"));
	m_Organs->InsertColumn(2, _("Recording"));
	m_Organs->InsertColumn(3, _("ODF Path"));
	topSizer->Add(m_Organs, 1, wxEXPAND | wxALL, 5);

	for(unsigned i = 0; i < m_Settings.GetOrganList().size(); i++)
	{
		GOrgueOrgan* o = m_Settings.GetOrganList()[i];
		m_Organs->InsertItem(i, o->GetChurchName());
		m_Organs->SetItemPtrData(i, (wxUIntPtr)o);
		m_Organs->SetItem(i, 1, o->GetOrganBuilder());
		m_Organs->SetItem(i, 2, o->GetRecordingDetail());
		m_Organs->SetItem(i, 3, o->GetODFPath());
	}

	m_Organs->SetColumnWidth(0, 150);
	m_Organs->SetColumnWidth(1, 150);
	m_Organs->SetColumnWidth(2, 250);
	m_Organs->SetColumnWidth(3, wxLIST_AUTOSIZE);


	topSizer->AddSpacer(5);
	topSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 5);

	SetSizer(topSizer);
}

void OrganSelectDialog::OnOK(wxCommandEvent& event)
{
	if (m_Organs->GetItemData(m_Organs->GetFirstSelected()) == 0)
	{
		wxMessageBox(_("Please select an organ"), _("Error"), wxOK | wxICON_ERROR, this);
		return;
	}
	EndModal(wxID_OK);
}

GOrgueOrgan* OrganSelectDialog::GetSelection()
{
	return (GOrgueOrgan*)m_Organs->GetItemData(m_Organs->GetFirstSelected());
}

void OrganSelectDialog::OnDoubleClick(wxListEvent& event)
{
	wxCommandEvent e;
	OnOK(event);
}
