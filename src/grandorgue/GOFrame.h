/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOFRAME_H
#define GOFRAME_H

#include <wx/dcmemory.h>
#include <wx/frame.h>

#include <vector>

#include "help/GOHelpRequestor.h"
#include "midi/GOMidiCallback.h"
#include "midi/GOMidiListener.h"
#include "threading/GOMutex.h"

#include "GOEvent.h"
#include "GOResizable.h"

class GOApp;
class GOAudioGauge;
class GOConfig;
class GODocument;
class GOMidiEvent;
class GOOrgan;
class GOOrganController;
class GOProgressDialog;
class GOSound;
class wxChoice;
class wxHtmlHelpController;
class wxSpinCtrl;
class wxToolBar;
class wxToolBarToolBase;

class GOFrame : public wxFrame,
                private GOHelpRequestor,
                public GOResizable,
                protected GOMidiCallback {
private:
  GOApp &m_App;
  GOMutex m_mutex;
  wxMenu *m_file_menu;
  wxMenu *m_audio_menu;
  wxMenu *m_panel_menu;
  wxMenu *m_favorites_menu;
  wxMenu *m_recent_menu;
  wxMenu *m_temperament_menu;
  GODocument *m_doc;
  wxToolBar *m_ToolBar;
  GOAudioGauge *m_SamplerUsage;
  wxControl *m_VolumeControl;
  wxToolBarToolBase *m_VolumeControlTool;
  std::vector<GOAudioGauge *> m_VolumeGauge;
  wxSpinCtrl *m_Transpose;
  wxChoice *m_ReleaseLength;
  wxSpinCtrl *m_Polyphony;
  wxSpinCtrl *m_SetterPosition;
  wxSpinCtrl *m_Volume;
  GOSound &m_Sound;
  GOConfig &m_config;
  GOMidiListener m_listener;
  wxString m_Title;
  wxString m_Label;
  bool m_MidiMonitor;
  bool m_isMeterReady;

  // to avoid event processing when the settings dialog is open
  bool m_InSettings;
  wxEventType m_AfterSettingsEventType;
  int m_AfterSettingsEventId;
  GOOrgan *p_AfterSettingsEventOrgan;

  void UpdateReleaseLength();
  void UpdatePanelMenu();
  void UpdateFavoritesMenu();
  void UpdateRecentMenu();
  void UpdateTemperamentMenu();
  bool AdjustVolumeControlWithSettings();
  void UpdateSize();
  void UpdateVolumeControlWithSettings();

  // Returns the current open organ controller or nullptr
  GOOrganController *GetOrganController() const;

  bool LoadOrgan(const GOOrgan &organ, const wxString &cmb = wxEmptyString);

  void OnMeters(wxCommandEvent &event);
  void OnLoadFile(wxCommandEvent &event);
  void OnLoad(wxCommandEvent &event);
  void OnLoadFavorite(wxCommandEvent &event);
  void OnLoadRecent(wxCommandEvent &event);
  void OnInstall(wxCommandEvent &event);
  void OnOpen(wxCommandEvent &event);
  void OnSave(wxCommandEvent &event);
  void OnMenuClose(wxCommandEvent &event);
  void OnExit(wxCommandEvent &event);
  void OnImportSettings(wxCommandEvent &event);
  void OnImportCombinations(wxCommandEvent &event);
  void OnExport(wxCommandEvent &event);
  void OnCache(wxCommandEvent &event);
  void OnCacheDelete(wxCommandEvent &event);
  void OnReload(wxCommandEvent &event);
  void OnRevert(wxCommandEvent &event);
  void OnProperties(wxCommandEvent &event);

  void OnEditOrgan(wxCommandEvent &event);
  void OnMidiList(wxCommandEvent &event);

  void OnAudioPanic(wxCommandEvent &event);
  void OnAudioMemset(wxCommandEvent &event);
  void OnAudioState(wxCommandEvent &event);

  void SetEventAfterSettings(
    wxEventType eventType, int eventId, GOOrgan *pOrganFile = NULL);

  void OnSettings(wxCommandEvent &event);
  void OnMidiLoad(wxCommandEvent &event);
  void OnMidiMonitor(wxCommandEvent &event);

  void OnPreset(wxCommandEvent &event);
  void OnTemperament(wxCommandEvent &event);
  void OnHelp(wxCommandEvent &event);
  void OnHelpAbout(wxCommandEvent &event);

  void OnSettingsVolume(wxCommandEvent &event);
  void OnSettingsPolyphony(wxCommandEvent &event);
  void OnSettingsMemory(wxCommandEvent &event);
  void OnSettingsMemoryEnter(wxCommandEvent &event);
  void OnSettingsTranspose(wxCommandEvent &event);
  void OnSettingsReleaseLength(wxCommandEvent &event);

  void OnKeyCommand(wxKeyEvent &event);
  void OnChangeTranspose(wxCommandEvent &event);
  void OnChangeSetter(wxCommandEvent &event);
  void OnChangeVolume(wxCommandEvent &event);
  void OnPanel(wxCommandEvent &event);

  void OnSize(wxSizeEvent &event);

  void OnMenuOpen(wxMenuEvent &event);
  void OnCloseWindow(wxCloseEvent &event);

  void OnMidiEvent(const GOMidiEvent &event);

  void OnUpdateLoaded(wxUpdateUIEvent &event);
  void OnSetTitle(wxCommandEvent &event);
  void OnMsgBox(wxMsgBoxEvent &event);
  void OnRenameFile(wxRenameFileEvent &event);

  bool CloseOrgan(bool isForce = false);
  bool CloseProgram(bool isForce = false);
  void Open(const GOOrgan &organ);

  bool InstallOrganPackage(wxString name);
  void LoadLastOrgan();
  void LoadFirstOrgan();
  void SendLoadOrgan(const GOOrgan &organ);

public:
  GOFrame(
    GOApp &app,
    wxFrame *frame,
    wxWindowID id,
    const wxString &title,
    const wxPoint &pos,
    const wxSize &size,
    const long type,
    GOSound &sound);
  virtual ~GOFrame(void);

  void Init(wxString filename);

  void DoSplash(bool timeout = true);

  void SendLoadFile(wxString filename);

  virtual GOLogicalRect GetPosSize() const override;
  virtual void SetPosSize(const GOLogicalRect &rect) override;

  DECLARE_EVENT_TABLE()
};

#endif
