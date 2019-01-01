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

#include "GOrgueEvent.h"

#include <wx/app.h>
#include <wx/window.h>

DEFINE_LOCAL_EVENT_TYPE(wxEVT_METERS)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_LOADFILE)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_SHOWHELP)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_SETVALUE)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_WINTITLE)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_SHOWMSG)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_RENAMEFILE)

wxMsgBoxEvent::wxMsgBoxEvent(const wxString& title, const wxString& text, long style) :
	wxEvent(0, wxEVT_SHOWMSG),
	m_Title(title),
	m_Text(text),
	m_Style(style)
{
}

wxMsgBoxEvent::wxMsgBoxEvent(const wxMsgBoxEvent& e) :
	wxEvent(e),
	m_Title(e.m_Title),
	m_Text(e.m_Text),
	m_Style(e.m_Style)
{
}

wxEvent* wxMsgBoxEvent::Clone() const
{
	return new wxMsgBoxEvent(*this);
}

const wxString& wxMsgBoxEvent::getTitle()
{
	return m_Title;
}

const wxString& wxMsgBoxEvent::getText()
{
	return m_Text;
}

long wxMsgBoxEvent::getStyle()
{
	return m_Style;
}

IMPLEMENT_DYNAMIC_CLASS(wxMsgBoxEvent, wxEvent)

wxRenameFileEvent::wxRenameFileEvent(const wxString& filename, const wxString& directory, const wxString& filter) :
	wxEvent(0, wxEVT_RENAMEFILE),
	m_Filename(filename),
	m_Directory(directory),
	m_Filter(filter)
{
}

wxRenameFileEvent::wxRenameFileEvent(const wxRenameFileEvent& e) :
	wxEvent(e),
	m_Filename(e.m_Filename),
	m_Directory(e.m_Directory),
	m_Filter(e.m_Filter)
{
}

wxEvent* wxRenameFileEvent::Clone() const
{
	return new wxRenameFileEvent(*this);
}

const wxString& wxRenameFileEvent::getFilename()
{
	return m_Filename;
}

const wxString& wxRenameFileEvent::getDirectory()
{
	return m_Directory;
}

const wxString& wxRenameFileEvent::getFilter()
{
	return m_Filter;
}

IMPLEMENT_DYNAMIC_CLASS(wxRenameFileEvent, wxEvent)

void GOMessageBox(const wxString& text, const wxString title, long style, wxWindow* parent)
{
	wxMsgBoxEvent event(title, text, style);
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

void GOAskRenameFile(const wxString& file, const wxString directory, const wxString& filter)
{
	wxRenameFileEvent event(file, directory, filter);
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}
