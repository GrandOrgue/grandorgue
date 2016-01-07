/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEEVENT_H
#define GORGUEEVENT_H

#include <wx/event.h>

DECLARE_LOCAL_EVENT_TYPE(wxEVT_METERS, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_LOADFILE, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_SHOWHELP, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_SETVALUE, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_WINTITLE, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_SHOWMSG, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_RENAMEFILE, -1)

class wxMsgBoxEvent : public wxEvent {
private:
	wxString m_Title;
	wxString m_Text;
	long m_Style;

public:
	wxMsgBoxEvent(const wxString& title = wxEmptyString, const wxString& text = wxEmptyString, long style = 0);
	wxMsgBoxEvent(const wxMsgBoxEvent& e);

	const wxString& getTitle();
	const wxString& getText();
	long getStyle();

	wxEvent* Clone() const;
	DECLARE_DYNAMIC_CLASS(wxMsgBoxEvent)
};

class wxRenameFileEvent : public wxEvent {
private:
	wxString m_Filename;
	wxString m_Directory;
	wxString m_Filter;

public:
	wxRenameFileEvent(const wxString& filename = wxEmptyString, const wxString& directory = wxEmptyString, const wxString& filter = wxEmptyString);
	wxRenameFileEvent(const wxRenameFileEvent& e);

	const wxString& getFilename();
	const wxString& getDirectory();
	const wxString& getFilter();

	wxEvent* Clone() const;
	DECLARE_DYNAMIC_CLASS(wxRenameFileEvent)
};

typedef void (wxEvtHandler::*wxMsgBoxFunction)(wxMsgBoxEvent&);

typedef void (wxEvtHandler::*wxRenameFileFunction)(wxRenameFileEvent&);

#define EVT_MSGBOX(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHOWMSG, wxID_ANY, wxID_ANY, \
    (wxObjectEventFunction) (wxEventFunction) \
    wxStaticCastEvent( wxMsgBoxFunction, & fn ), (wxObject *) NULL ),

#define EVT_RENAMEFILE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_RENAMEFILE, wxID_ANY, wxID_ANY, \
    (wxObjectEventFunction) (wxEventFunction) \
    wxStaticCastEvent( wxRenameFileFunction, & fn ), (wxObject *) NULL ),

void GOMessageBox(const wxString& text, const wxString title, long style, wxWindow* parent = NULL);
void GOAskRenameFile(const wxString& file, const wxString directory, const wxString& filter);

#endif
