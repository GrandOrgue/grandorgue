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

#ifndef MIDIEVENTKEYDIALOG_H_
#define MIDIEVENTKEYDIALOG_H_

#include "GOrgueKeyReceiver.h"
#include <wx/panel.h>

class wxChoice;
class wxToggleButton;

class MIDIEventKeyDialog : public wxPanel
{

private:
	GOrgueKeyReceiver* m_original;
	GOrgueKeyReceiverData m_key;
	wxChoice *m_keyselect;
	wxToggleButton* m_listen;

	void OnKeyDown(wxKeyEvent& event);
	void OnListenClick(wxCommandEvent& event);

protected:
	enum {
		ID_KEY_SELECT = 200,
		ID_LISTEN,
	};
public:
	MIDIEventKeyDialog (wxWindow* parent, GOrgueKeyReceiver* event);
	~MIDIEventKeyDialog();

	void DoApply();

	DECLARE_EVENT_TABLE()
};

#endif
