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

#include "SettingsTemperaments.h"

#include "GOrgueLimits.h"
#include "GOrgueTemperamentUser.h"
#include "GOrgueTime.h"
#include "GOrgueSettings.h"
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/stopwatch.h>

BEGIN_EVENT_TABLE(SettingsTemperaments, wxPanel)
	EVT_GRID_CMD_SELECT_CELL(ID_LIST, SettingsTemperaments::OnListSelected)
	EVT_BUTTON(ID_ADD, SettingsTemperaments::OnAdd)
	EVT_BUTTON(ID_DEL, SettingsTemperaments::OnDel)
END_EVENT_TABLE()

SettingsTemperaments::SettingsTemperaments(GOrgueSettings& settings, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Temperaments(settings.GetTemperaments()),
	m_Ptrs()
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->AddSpacer(5);

	m_List = new wxGrid(this, ID_LIST);
	m_List->CreateGrid(0, 14);
	topSizer->Add(m_List, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* buttonSizer =  new wxBoxSizer(wxHORIZONTAL);
	m_Add = new wxButton(this, ID_ADD, _("&Add"));
	m_Del = new wxButton(this, ID_DEL, _("&Delete"));
	buttonSizer->Add(m_Add, 0, wxALIGN_LEFT | wxALL, 5);
	buttonSizer->Add(m_Del, 0, wxALIGN_LEFT | wxALL, 5);
	topSizer->Add(buttonSizer, 0, wxALL, 5);
	
	m_List->SetColLabelValue(0, _("Group"));
	m_List->SetColLabelValue(1, _("Name"));
	m_List->SetColLabelValue(2, _("c"));
	m_List->SetColLabelValue(3, _("c#"));
	m_List->SetColLabelValue(4, _("d"));
	m_List->SetColLabelValue(5, _("d#"));
	m_List->SetColLabelValue(6, _("e"));
	m_List->SetColLabelValue(7, _("f"));
	m_List->SetColLabelValue(8, _("f#"));
	m_List->SetColLabelValue(9, _("g"));
	m_List->SetColLabelValue(10, _("g#"));
	m_List->SetColLabelValue(11, _("a"));
	m_List->SetColLabelValue(12, _("a#"));
	m_List->SetColLabelValue(13, _("b"));

	m_List->SetColMinimalWidth(0, 75);
	m_List->SetColMinimalWidth(1, 150);

	for(unsigned i = 0; i < 12; i++)
	{
		m_List->SetColFormatFloat(2 + i, 8, 3);
		m_List->SetColMinimalWidth(2 + i, 50);
	}

	m_List->SetDefaultEditor(new wxGridCellFloatEditor(8, 3));

	for(unsigned i = 0; i < m_Temperaments.GetUserTemperaments().size(); i++)
	{
		GOrgueTemperamentUser* t = m_Temperaments.GetUserTemperaments()[i];

		unsigned row = m_List->GetNumberRows();
		m_Ptrs.push_back(t);
		m_List->AppendRows(1);
		m_List->SetCellValue(row, 0, t->GetGroup());
		m_List->SetCellEditor(row, 0, new wxGridCellTextEditor());
		m_List->SetCellValue(row, 1, t->GetTitle());
		m_List->SetCellEditor(row, 1, new wxGridCellTextEditor());
		for(unsigned i = 0; i < 12; i++)
			m_List->SetCellValue(row, 2 + i, wxString::Format(wxT("%.03f"), t->GetOffset(i)));
	}

	m_List->EnableEditing(true);
	m_List->AutoSizeColumns(false);
	m_List->SetRowLabelSize(0);

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);

	Update();
}

void SettingsTemperaments::OnAdd(wxCommandEvent& event)
{
	unsigned row = m_List->GetNumberRows();
	m_Ptrs.push_back(NULL);
	m_List->AppendRows(1);
	m_List->SetCellValue(row, 0, _("User"));
	m_List->SetCellEditor(row, 0, new wxGridCellTextEditor());
	m_List->SetCellValue(row, 1, _("User Temperament"));
	m_List->SetCellEditor(row, 1, new wxGridCellTextEditor());
	for(unsigned i = 0; i < 12; i++)
		m_List->SetCellValue(row, 2 + i, wxString::Format(wxT("%.03f"), 0.0));
	Update();
}

void SettingsTemperaments::OnDel(wxCommandEvent& event)
{
	unsigned row = m_List->GetGridCursorRow();
	m_List->DeleteRows(row, 1);
	m_Ptrs.erase(m_Ptrs.begin() + row);
	Update();
}

void SettingsTemperaments::OnListSelected(wxGridEvent& event)
{
	event.Skip();
	Update();
}

void SettingsTemperaments::Update()
{
	if (m_Ptrs.size() > 0 && m_List->GetGridCursorRow() >= 0)
		m_Del->Enable();
	else
		m_Del->Disable();
	if (m_Ptrs.size() < MAX_TEMPERAMENTS)
		m_Add->Enable();
	else
		m_Add->Disable();
}

void SettingsTemperaments::Save()
{
	ptr_vector<GOrgueTemperamentUser>& list = m_Temperaments.GetUserTemperaments();
	for(unsigned i = 0; i < list.size(); i++)
	{
		bool found = false;
		for(unsigned j = 0; j < m_Ptrs.size(); j++)
			if (m_Ptrs[j] == list[i])
				found = true;

		if (!found)
			delete list[i];
		list[i] = 0;
	}
	list.clear();
	long time = wxGetUTCTime();
	for(unsigned i = 0; i < m_Ptrs.size(); i++)
	{
		if (!m_Ptrs[i])
			m_Ptrs[i] = new GOrgueTemperamentUser(wxString::Format(wxT("UserTemperament%d-%d"), time, i), m_List->GetCellValue(i, 1), m_List->GetCellValue(i, 0));

		m_Ptrs[i]->SetTitle(m_List->GetCellValue(i, 1));
		m_Ptrs[i]->SetGroup(m_List->GetCellValue(i, 0));
		for(unsigned j = 0; j < 12; j++)
			m_Ptrs[i]->SetOffset(j, wxAtof(m_List->GetCellValue(i, 2 + j)));

		list.push_back(m_Ptrs[i]);
	}
}
