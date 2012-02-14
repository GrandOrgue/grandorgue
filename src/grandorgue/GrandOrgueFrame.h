/*
 * GrandOrgue - a free pipe organ simulator
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
 * MA 02111-1307, USA.
 */

#ifndef GRANDORGUEFRAME_H
#define GRANDORGUEFRAME_H

#include <vector>
#include <wx/docview.h>
#include <wx/dcmemory.h>
#include "GrandOrgueDef.h"

class wxHtmlHelpController;
class wxGaugeAudio;
class wxSpinCtrl;
class GOrgueMidiEvent;
class GOrgueSettings;
class GOrgueSound;

class GOrgueFrame: public wxDocParentFrame
{
	DECLARE_CLASS(GOrgueFrame)
	wxMenu* m_panel_menu;
	wxHtmlHelpController* m_Help;
	wxGaugeAudio *m_SamplerUsage;
	wxGaugeAudio *m_VolumeLeft, *m_VolumeRight;
	wxSpinCtrl* m_Transpose;
	wxSpinCtrl* m_Polyphony;
	wxSpinCtrl* m_SetterPosition;
	wxSpinCtrl* m_Volume;
	std::vector<wxString> m_TemperamentNames;
	GOrgueSound& m_Sound;
	GOrgueSettings& m_Settings;

public:
	GOrgueFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type, GOrgueSound& sound);
	~GOrgueFrame(void);

	void Init();

	void OnMeters(wxCommandEvent& event);

	void OnLoadFile(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnImportSettings(wxCommandEvent& event);
	void OnImportCombinations(wxCommandEvent& event);
	void OnExport(wxCommandEvent& event);
	void OnCache(wxCommandEvent& event);
	void OnCacheDelete(wxCommandEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnRevert(wxCommandEvent& event);
	void OnProperties(wxCommandEvent& event);

	void OnEditOrgan(wxCommandEvent& event);

	void OnAudioPanic(wxCommandEvent& event);
	void OnAudioRecord(wxCommandEvent& event);
	void OnAudioMemset(wxCommandEvent& event);
	void OnAudioSettings(wxCommandEvent& event);

	void OnPreset(wxCommandEvent& event);
	void OnTemperament(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnShowHelp(wxCommandEvent& event);

	void OnSettingsVolume(wxCommandEvent& event);
	void OnSettingsPolyphony(wxCommandEvent& event);
	void OnSettingsMemory(wxCommandEvent& event);
	void OnSettingsTranspose(wxCommandEvent& event);

	void OnKeyCommand(wxKeyEvent& event);
	void OnChangeSetter(wxCommandEvent& event);
	void OnChangeVolume(wxCommandEvent& event);
	void OnPanel(wxCommandEvent& event);

	void OnSize(wxSizeEvent& event);

	void OnMenuOpen(wxMenuEvent& event);

	void OnMidiEvent(GOrgueMidiEvent& event);

	void DoSplash(bool timeout = true);

	void OnUpdateLoaded(wxUpdateUIEvent& event);

	DECLARE_EVENT_TABLE()
protected:
	void UpdatePanelMenu();
	void AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString = wxEmptyString);
	void AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString, const wxBitmap& toolbarImage, wxItemKind kind = wxITEM_NORMAL);
	void InitHelp();
};

#endif
