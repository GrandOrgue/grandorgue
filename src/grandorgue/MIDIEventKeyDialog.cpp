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

#include "MIDIEventKeyDialog.h"
#include "GOrgueKeyConvert.h"
#include <wx/tglbtn.h>

BEGIN_EVENT_TABLE(MIDIEventKeyDialog, wxPanel)
	EVT_TOGGLEBUTTON(ID_LISTEN, MIDIEventKeyDialog::OnListenClick)
END_EVENT_TABLE()

MIDIEventKeyDialog::MIDIEventKeyDialog(wxWindow* parent, GOrgueKeyReceiver* event) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS),
	m_original(event),
	m_key(*event)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* sizer = new wxFlexGridSizer(5, 2, 5, 5);
	topSizer->Add(sizer, 0, wxALL, 6);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Shortcut:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	m_keyselect = new wxChoice(this, ID_KEY_SELECT);
	sizer->Add(m_keyselect, 1, wxEXPAND);

	sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
	m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
	sizer->Add(m_listen, 0, wxTOP, 5);
	SetSizer(topSizer);

	unsigned shortcut = m_key.GetShortcut();
	const GOShortcutKey* keys = GetShortcutKeys();
	unsigned count = GetShortcutKeyCount();
	m_keyselect->Append(_("None"), (void*)0);
	m_keyselect->SetSelection(0);
	for(unsigned i = 0; i < count; i++)
	{
		m_keyselect->Append(wxGetTranslation(keys[i].name), (void*)&keys[i]);
		if (keys[i].key_code == shortcut)
			m_keyselect->SetSelection(i + 1);
	}

	topSizer->Fit(this);
}

MIDIEventKeyDialog::~MIDIEventKeyDialog()
{
}

void MIDIEventKeyDialog::DoApply()
{
	const GOShortcutKey* key = (const GOShortcutKey*) m_keyselect->GetClientData(m_keyselect->GetSelection());
	if (!key)
		m_key.SetShortcut(0);
	else
		m_key.SetShortcut(key->key_code);
	m_original->Assign(m_key);
}

void MIDIEventKeyDialog::OnKeyDown(wxKeyEvent& event)
{
	unsigned code = WXKtoVK(event.GetKeyCode());
	if (code)
	{
		for(unsigned i = 0; i < m_keyselect->GetCount(); i++)
		{
			const GOShortcutKey* key = (const GOShortcutKey*) m_keyselect->GetClientData(i);
			if (key && key->key_code == code)
				m_keyselect->SetSelection(i);
		}

		m_listen->SetValue(false);
		m_listen->GetEventHandler()->Disconnect(wxEVT_KEY_DOWN, wxKeyEventHandler(MIDIEventKeyDialog::OnKeyDown), NULL, this);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
	else
		event.Skip();
}

void MIDIEventKeyDialog::OnListenClick(wxCommandEvent& event)
{
	if (m_listen->GetValue())
	{
		this->SetCursor(wxCursor(wxCURSOR_WAIT));
		m_listen->GetEventHandler()->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(MIDIEventKeyDialog::OnKeyDown), NULL, this);
	}
	else
	{
		m_listen->GetEventHandler()->Disconnect(wxEVT_KEY_DOWN, wxKeyEventHandler(MIDIEventKeyDialog::OnKeyDown), NULL, this);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}
