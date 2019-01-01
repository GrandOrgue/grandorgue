/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#ifndef MIDILIST_H
#define MIDILIST_H

#include "GOrgueView.h"
#include <wx/dialog.h>
#include <vector>

class GOrgueEventDistributor;
class wxButton;
class wxListEvent;
class wxListView;

class MIDIList : public wxDialog, public GOrgueView
{
private:
	wxListView* m_Objects;
	wxButton* m_Edit;
	wxButton* m_Status;
	std::vector<wxButton*> m_Buttons;

	enum {
		ID_LIST = 200,
		ID_EDIT,
		ID_STATUS,
		ID_BUTTON,
		ID_BUTTON_LAST = ID_BUTTON + 2,
	};

	void OnObjectClick(wxListEvent& event);
	void OnObjectDoubleClick(wxListEvent& event);
	void OnEdit(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnStatus(wxCommandEvent& event);
	void OnButton(wxCommandEvent& event);

public:
	MIDIList (GOrgueDocumentBase* doc, wxWindow* parent, GOrgueEventDistributor* midi_elements);
	~MIDIList();

	DECLARE_EVENT_TABLE()
};

#endif
