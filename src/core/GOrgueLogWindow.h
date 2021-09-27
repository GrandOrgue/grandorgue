/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUELOGWINDOW_H
#define GORGUELOGWINDOW_H

#include <wx/frame.h>
#include <wx/log.h>

class wxListCtrl;

DECLARE_LOCAL_EVENT_TYPE(wxEVT_ADD_LOG_MESSAGE, -1)

class GOrgueLogWindow : public wxFrame
{
private:
	wxListCtrl* m_List;

	void OnLog(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
	void OnPopup(wxContextMenuEvent& event);
	void OnCloseWindow(wxCloseEvent& event);

public:
	GOrgueLogWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);
	~GOrgueLogWindow();

	void LogMsg(wxLogLevel level, const wxString& msg, time_t time);

	DECLARE_EVENT_TABLE()
};

#endif
