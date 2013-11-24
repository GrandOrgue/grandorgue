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

#include "GOrgueLogWindow.h"
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>

DEFINE_LOCAL_EVENT_TYPE(wxEVT_ADD_LOG_MESSAGE)

BEGIN_EVENT_TABLE(GOrgueLogWindow, wxFrame)
	EVT_COMMAND(0, wxEVT_ADD_LOG_MESSAGE, GOrgueLogWindow::OnLog)
	EVT_CLOSE(GOrgueLogWindow::OnCloseWindow)
	EVT_CONTEXT_MENU(GOrgueLogWindow::OnPopup)
	EVT_MENU(wxID_COPY, GOrgueLogWindow::OnCopy)
END_EVENT_TABLE()

GOrgueLogWindow::GOrgueLogWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) :
	wxFrame(parent, id, title, pos, size, style)
{
	m_List = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
	m_List->InsertColumn(0, wxT("Message"));
	m_List->InsertColumn(1, wxT("Time"));

	const int img_size = 16;
	wxImageList* imgs = new wxImageList(img_size, img_size);
	imgs->Add(wxArtProvider::GetBitmap(wxART_ERROR, wxART_MESSAGE_BOX, wxSize(img_size, img_size)));
	imgs->Add(wxArtProvider::GetBitmap(wxART_WARNING, wxART_MESSAGE_BOX, wxSize(img_size, img_size)));
	imgs->Add(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_MESSAGE_BOX, wxSize(img_size, img_size)));
	m_List->AssignImageList(imgs, wxIMAGE_LIST_SMALL);

	m_List->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_List->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

GOrgueLogWindow::~GOrgueLogWindow()
{
}

void GOrgueLogWindow::OnCopy(wxCommandEvent& event)
{
	wxString text;
	for(long i = m_List->GetItemCount() - 1; i >= 0; i--)
	{
		wxListItem i1, i2;
		i1.SetId(i);
		i1.SetColumn(1);
		i2.SetId(i);
		i2.SetColumn(0);
		i1.SetMask(wxLIST_MASK_TEXT);
		i2.SetMask(wxLIST_MASK_TEXT);
		m_List->GetItem(i1);
		m_List->GetItem(i2);
		text += wxString::Format(_("%s: %s\n"), i1.GetText().c_str(), i2.GetText().c_str());
	}

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(text));
		wxTheClipboard->Close();
	}
}

void GOrgueLogWindow::OnPopup(wxContextMenuEvent& event)
{
	wxMenu popup;
	popup.Append(wxID_COPY, _("C&opy"), wxEmptyString, wxITEM_NORMAL);
	PopupMenu(&popup);
}

void GOrgueLogWindow::OnLog(wxCommandEvent& event)
{
	wxDateTime time((time_t)event.GetTimestamp());
	wxString timestr = time.Format();

	long line = m_List->InsertItem(0, event.GetString(), event.GetInt());
	m_List->SetItem(line, 1, timestr);

	m_List->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_List->SetColumnWidth(1, wxLIST_AUTOSIZE);

	if (!IsShown())
		Show();
}

void GOrgueLogWindow::OnCloseWindow(wxCloseEvent& event)
{
	Show(false);
	m_List->DeleteAllItems();
}

void GOrgueLogWindow::LogMsg(wxLogLevel level, const wxString& msg, time_t timestamp)
{
	static unsigned count = 0;
	int l;
	switch(level)
	{
	case wxLOG_FatalError:
	case wxLOG_Error:
		l = 0;
		break;
	case wxLOG_Warning:
		l = 1;
		break;
	default:
		l = 2;
	}
	wxCommandEvent e(wxEVT_ADD_LOG_MESSAGE, 0);
	e.SetString(msg);
	e.SetInt(l);
	e.SetTimestamp(timestamp);
	GetEventHandler()->AddPendingEvent(e);
	count++;
	if ((count % 100)==0)
		wxTheApp->Yield(true);
}
