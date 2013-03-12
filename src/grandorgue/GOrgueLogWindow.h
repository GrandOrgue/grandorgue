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

#ifndef GORGUELOGWINDOW_H
#define GORGUELOGWINDOW_H

#include <wx/wx.h>

class wxListCtrl;

DECLARE_LOCAL_EVENT_TYPE(wxEVT_ADD_LOG_MESSAGE, -1)

class GOrgueLogWindow : public wxFrame
{
private:
	wxListCtrl* m_List;

	void OnLog(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);

public:
	GOrgueLogWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);
	~GOrgueLogWindow();

	void LogMsg(wxLogLevel level, const wxString& msg, time_t time);

	DECLARE_EVENT_TABLE()
};

#endif
