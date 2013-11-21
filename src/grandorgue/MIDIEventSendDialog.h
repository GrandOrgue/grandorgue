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

#ifndef MIDIEVENTSENDDIALOG_H_
#define MIDIEVENTSENDDIALOG_H_

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "GOrgueMidiSender.h"

class GOrgueSettings;

class MIDIEventSendDialog : public wxPanel
{
private:
	GOrgueSettings& m_Settings;
	GOrgueMidiSender* m_original;
	GOrgueMidiSenderData m_midi;
	wxChoice *m_eventno, *m_eventtype, *m_channel, *m_device;
	wxSpinCtrl *m_key;
	wxSpinCtrl *m_LowValue;
	wxSpinCtrl *m_HighValue;
	wxButton* m_new, *m_delete;
	int m_current;

	void StoreEvent();
	void LoadEvent();

	void OnNewClick(wxCommandEvent& event);
	void OnDeleteClick(wxCommandEvent& event);
	void OnEventChange(wxCommandEvent& event);
	void OnTypeChange(wxCommandEvent& event);

protected:
	enum {
		ID_EVENT_NO = 200,
		ID_EVENT_NEW,
		ID_EVENT_DELETE,
		ID_DEVICE,
		ID_EVENT,
		ID_CHANNEL,
		ID_KEY,
		ID_LOW_VALUE,
		ID_HIGH_VALUE
	};

public:
	MIDIEventSendDialog (wxWindow* parent, GOrgueMidiSender* event, GOrgueSettings& settings);
	~MIDIEventSendDialog();

	void DoApply();

	DECLARE_EVENT_TABLE()
};

#endif
