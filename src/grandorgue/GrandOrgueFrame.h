/*
 * GrandOrgue - a free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GRANDORGUEFRAME_H
#define GRANDORGUEFRAME_H

#include <wx/docview.h>
#include <wx/dcmemory.h>

class wxHtmlHelpController;
class GOrgueMeter;

class GOrgueFrame: public wxDocParentFrame
{
	DECLARE_CLASS(GOrgueFrame)
	wxMenu* m_panel_menu;
	wxHtmlHelpController* m_Help;

public:
    GOrgueFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type);
    ~GOrgueFrame(void);

	void OnMeters(wxCommandEvent& event);

    void OnLoadFile(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnLoad(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnCache(wxCommandEvent& event);
	void OnCacheDelete(wxCommandEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnRevert(wxCommandEvent& event);
	void OnProperties(wxCommandEvent& event);

	void OnAudioPanic(wxCommandEvent& event);
	void OnAudioRecord(wxCommandEvent& event);
	void OnAudioMemset(wxCommandEvent& event);
	void OnAudioSettings(wxCommandEvent& event);

	void OnHelp(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnShowHelp(wxCommandEvent& event);

	//Added by Graham Goode Nov 2009
	// Added to allow for the mouse-over event creating a description of the new icons
	// that were formerly just text
	void OnSettingsVolume(wxCommandEvent& event);
	void OnSettingsPolyphony(wxCommandEvent& event);
	void OnSettingsMemory(wxCommandEvent& event);
	void OnSettingsTranspose(wxCommandEvent& event);
	//
	void OnKeyCommand(wxKeyEvent& event);
	void ChangeSetter(unsigned position);
	void OnUpdatePanelMenu(wxUpdateUIEvent& event);
	void OnPanel(wxCommandEvent& event);

	void OnSize(wxSizeEvent& event);

	void OnMenuOpen(wxMenuEvent& event);

	void DoSplash(bool timeout = true);

	void OnUpdateLoaded(wxUpdateUIEvent& event);

	GOrgueMeter* m_meters[4];

	DECLARE_EVENT_TABLE()
protected:
	void AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString = wxEmptyString);
	void AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString, const wxBitmap& toolbarImage, wxItemKind kind = wxITEM_NORMAL);
	wxString GetDocumentDirectory();
	wxString GetOrganDirectory();
	void InitHelp();
};

#endif
