/*
 * GrandOrgue - a free pipe organ simulator
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

#ifndef GRANDORGUEFRAME_H
#define GRANDORGUEFRAME_H

#include "GOrgueEvent.h"
#include "GOrgueMidiListener.h"
#include "GOLock.h"
#include <wx/dcmemory.h>
#include <wx/frame.h>

class GOrgueDocument;
class GOrgueMidiEvent;
class GOrgueOrgan;
class GOrgueSettings;
class GOrgueSound;
class wxChoice;
class wxGaugeAudio;
class wxHtmlHelpController;
class wxSpinCtrl;

class GOrgueFrame: public wxFrame, protected GOrgueMidiCallback
{
private:
	GOMutex m_mutex;
	wxMenu* m_file_menu;
	wxMenu* m_audio_menu;
	wxMenu* m_panel_menu;
	wxMenu* m_favorites_menu;
	wxMenu* m_recent_menu;
	wxMenu* m_temperament_menu;
	GOrgueDocument* m_doc;
	wxHtmlHelpController* m_Help;
	wxGaugeAudio *m_SamplerUsage;
	wxGaugeAudio *m_VolumeLeft, *m_VolumeRight;
	wxSpinCtrl* m_Transpose;
	wxChoice* m_ReleaseLength;
	wxSpinCtrl* m_Polyphony;
	wxSpinCtrl* m_SetterPosition;
	wxSpinCtrl* m_Volume;
	GOrgueSound& m_Sound;
	GOrgueSettings& m_Settings;
	GOrgueMidiListener m_listener;
	wxString m_Title;
	wxString m_Label;
	bool m_MidiMonitor;

	void InitHelp();
	void UpdatePanelMenu();
	void UpdateFavoritesMenu();
	void UpdateRecentMenu();
	void UpdateTemperamentMenu();

	GOrgueDocument* GetDocument();

	void OnMeters(wxCommandEvent& event);

	void OnLoadFile(wxCommandEvent& event);
	void OnLoad(wxCommandEvent& event);
	void OnLoadFavorite(wxCommandEvent& event);
	void OnLoadRecent(wxCommandEvent& event);
	void OnInstall(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnImportSettings(wxCommandEvent& event);
	void OnImportCombinations(wxCommandEvent& event);
	void OnExport(wxCommandEvent& event);
	void OnCache(wxCommandEvent& event);
	void OnCacheDelete(wxCommandEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnRevert(wxCommandEvent& event);
	void OnProperties(wxCommandEvent& event);

	void OnEditOrgan(wxCommandEvent& event);
	void OnMidiList(wxCommandEvent& event);

	void OnAudioPanic(wxCommandEvent& event);
	void OnAudioRecord(wxCommandEvent& event);
	void OnAudioMemset(wxCommandEvent& event);
	void OnAudioState(wxCommandEvent& event);
	void OnAudioSettings(wxCommandEvent& event);
	void OnMidiRecord(wxCommandEvent& event);
	void OnMidiPlay(wxCommandEvent& event);
	void OnMidiMonitor(wxCommandEvent& event);

	void OnPreset(wxCommandEvent& event);
	void OnTemperament(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);
	void OnShowHelp(wxCommandEvent& event);

	void OnSettingsVolume(wxCommandEvent& event);
	void OnSettingsPolyphony(wxCommandEvent& event);
	void OnSettingsMemory(wxCommandEvent& event);
	void OnSettingsMemoryEnter(wxCommandEvent& event);
	void OnSettingsTranspose(wxCommandEvent& event);
	void OnSettingsReleaseLength(wxCommandEvent& event);

	void OnKeyCommand(wxKeyEvent& event);
	void OnChangeTranspose(wxCommandEvent& event);
	void OnChangeSetter(wxCommandEvent& event);
	void OnChangeVolume(wxCommandEvent& event);
	void OnPanel(wxCommandEvent& event);

	void OnSize(wxSizeEvent& event);

	void OnMenuOpen(wxMenuEvent& event);
	void OnCloseWindow(wxCloseEvent& event);

	void OnMidiEvent(const GOrgueMidiEvent& event);

	void OnUpdateLoaded(wxUpdateUIEvent& event);
	void OnSetTitle(wxCommandEvent& event);
	void OnMsgBox(wxMsgBoxEvent& event);

	bool DoClose(bool force = false);
	void Open(const GOrgueOrgan& organ, bool force = false);

	bool InstallOrganPackage(wxString name);
	void LoadLastOrgan();
	void SendLoadOrgan(const GOrgueOrgan& organ, bool force = false);

public:
	GOrgueFrame(wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type, GOrgueSound& sound);
	~GOrgueFrame(void);

	void Init(wxString filename);
	bool Close(bool force = false);

	void DoSplash(bool timeout = true);

	void SendLoadFile(wxString filename, bool force = false);

	DECLARE_EVENT_TABLE()
};

#endif
