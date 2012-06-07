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

#ifndef SETTINGSMIDIMESSAGE_H
#define SETTINGSMIDIMESSAGE_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include "MIDIListenDialog.h"

class GOrgueSettings;

class SettingsMidiMessage : public wxPanel
{
	enum {
		ID_EVENTS,
		ID_PROPERTIES,
	};

private:
	GOrgueSettings& m_Settings;
	wxListView* m_Events;
	wxButton* m_Properties;

	unsigned GetEventCount();
	MIDIListenDialog::LISTEN_DIALOG_TYPE GetEventType(unsigned index);
	wxString GetEventName(unsigned index);
	int GetEventData(unsigned index);
	void SetEventData(unsigned index, int event);
	void UpdateMessages(unsigned i);

public:
	SettingsMidiMessage(GOrgueSettings& settings, wxWindow* parent);

	void OnEventsClick(wxListEvent& event);
	void OnEventsDoubleClick(wxListEvent& event);
	void OnProperties(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif

