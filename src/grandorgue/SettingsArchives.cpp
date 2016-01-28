/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#include "SettingsArchives.h"

#include "GOrgueArchiveFile.h"
#include "GOrgueOrgan.h"
#include "GOrgueSettings.h"
#include "SettingsOrgan.h"
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

BEGIN_EVENT_TABLE(SettingsArchives, wxPanel)
	EVT_LIST_ITEM_SELECTED(ID_ARCHIVES, SettingsArchives::OnArchiveSelected)
	EVT_BUTTON(ID_DEL, SettingsArchives::OnDel)
END_EVENT_TABLE()

SettingsArchives::SettingsArchives(GOrgueSettings& settings, SettingsOrgan& organs, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Settings(settings),
	m_Organs(organs)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_Archives = new wxListView(this, ID_ARCHIVES, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	m_Archives->InsertColumn(0, _("Name"));
	m_Archives->InsertColumn(1, _("Path"));
	m_Archives->InsertColumn(2, _("ID"));
	m_Archives->InsertColumn(3, _("Info"));
	topSizer->Add(m_Archives, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* buttonSizer =  new wxBoxSizer(wxHORIZONTAL);
	m_Del = new wxButton(this, ID_DEL, _("&Delete"));
	buttonSizer->Add(m_Del, 0, wxALIGN_LEFT | wxALL, 5);
	topSizer->Add(buttonSizer, 0, wxALL, 5);

	for(unsigned i = 0; i < m_Settings.GetArchiveList().size(); i++)
	{
		GOrgueArchiveFile* a = m_Settings.GetArchiveList()[i];
		wxString title = a->GetName();
		wxString info = wxEmptyString;
		if (!a->IsUsable(m_Settings))
		    title = _("MISSING - ") + title;
		else if (!a->IsComplete(m_Settings))
		{
		    title = _("INCOMPLETE - ") + title;
		    for(unsigned i = 0; i < a->GetDependencies().size(); i++)
			    if (!m_Settings.GetArchiveByID(a->GetDependencies()[i], true))
				    info += wxString::Format(_("requires '%s' "), a->GetDependencyTitles()[i]);
		}
		m_Archives->InsertItem(i, title);
		m_Archives->SetItemPtrData(i, (wxUIntPtr)a);
		m_Archives->SetItem(i, 1, a->GetPath());
		m_Archives->SetItem(i, 2, a->GetID());
		m_Archives->SetItem(i, 3, info);
	}

	if (m_Archives->GetItemCount())
	{
		m_Archives->SetColumnWidth(0, wxLIST_AUTOSIZE);
		m_Archives->SetColumnWidth(1, wxLIST_AUTOSIZE);
		m_Archives->SetColumnWidth(2, wxLIST_AUTOSIZE);
		m_Archives->SetColumnWidth(3, wxLIST_AUTOSIZE);
	}
	m_Del->Disable();

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

void SettingsArchives::OnArchiveSelected(wxListEvent& event)
{
	m_Del->Enable();
}

void SettingsArchives::OnDel(wxCommandEvent& event)
{
	GOrgueArchiveFile* a = (GOrgueArchiveFile*)m_Archives->GetItemData(m_Archives->GetFirstSelected());

	for(long i = 0; i < m_Archives->GetItemCount(); i++)
	{
		GOrgueArchiveFile* a1 = (GOrgueArchiveFile*)m_Archives->GetItemData(i);
		if (a == a1)
			continue;
		if (!a1->IsUsable(m_Settings))
			continue;
		if (a->GetID() == a1->GetID())
		{
			m_Archives->DeleteItem(m_Archives->GetFirstSelected());
			m_Del->Disable();
			return;
		}
	}

	std::vector<GOrgueOrgan*> organ_list = m_Organs.GetOrgans();
	for(unsigned i = 0; i < organ_list.size(); i++)
		if (organ_list[i]->GetArchiveID() == a->GetID())
		{
			wxMessageBox(wxString::Format(_("'%s' is still used by the organ '%s'"), a->GetName().c_str(), organ_list[i]->GetChurchName()), _("Delete package"), wxOK | wxICON_ERROR, this);
			return;
		}
	for(long i = 0; i < m_Archives->GetItemCount(); i++)
	{
		GOrgueArchiveFile* a1 = (GOrgueArchiveFile*)m_Archives->GetItemData(i);
		if (a == a1)
			continue;
		for(unsigned j = 0; j < a1->GetDependencies().size(); j++)
		{
			if (a->GetID() == a1->GetDependencies()[j])
			{
				wxMessageBox(wxString::Format(_("'%s' is still used by the package '%s'"), a->GetName().c_str(), a1->GetName()), _("Delete package"), wxOK | wxICON_ERROR, this);
				return;
			}
		}
	}
	m_Archives->DeleteItem(m_Archives->GetFirstSelected());
	m_Del->Disable();
}

void SettingsArchives::Save()
{
	ptr_vector<GOrgueArchiveFile>& list = m_Settings.GetArchiveList();
	for(unsigned i = 0; i < list.size(); i++)
	{
		bool found = false;
		for(long j = 0; j < m_Archives->GetItemCount(); j++)
			if (m_Archives->GetItemData(j) == (wxUIntPtr)list[i])
				found = true;
		if (!found)
			delete list[i];
		list[i] = 0;
	}
	list.clear();
	for(long i = 0; i < m_Archives->GetItemCount(); i++)
		list.push_back((GOrgueArchiveFile*)m_Archives->GetItemData(i));
}
