/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGuiOrgan.h"

#include <wx/app.h>

#include "config/GOConfig.h"
#include "document-base/GODocumentView.h"
#include "frames/GOAppWindow.h"
#include "gui/dialogs/GOMidiObjectstDialog.h"
#include "gui/dialogs/midi-event/GOMidiEventDialog.h"
#include "gui/dialogs/organ-settings/GOOrganSettingsDialog.h"
#include "gui/frames/GOMainWindowData.h"
#include "gui/frames/GOStopsWindow.h"
#include "gui/panels/GOGUIBankedGeneralsPanel.h"
#include "gui/panels/GOGUICouplerManualsAndVolumePanel.h"
#include "gui/panels/GOGUICouplerPanel.h"
#include "gui/panels/GOGUICrescendoPanel.h"
#include "gui/panels/GOGUIDivisionalsPanel.h"
#include "gui/panels/GOGUIMasterPanel.h"
#include "gui/panels/GOGUIMetronomePanel.h"
#include "gui/panels/GOGUIPanel.h"
#include "gui/panels/GOGUIPanelView.h"
#include "gui/panels/GOGUIRecorderPanel.h"
#include "gui/panels/GOGUISequencerPanel.h"
#include "gui/size/GOResizable.h"
#include "loader/GOProgressMonitor.h"
#include "midi/GOMidiSystem.h"
#include "midi/events/GOMidiEvent.h"
#include "threading/GOMutexLocker.h"

#include "GOEvent.h"
#include "GOOrgan.h"
#include "go_ids.h"

static const wxString WX_ORGAN = wxT("Organ");

GOGuiOrgan::GOGuiOrgan(
  GOResizable *pMainWindow, GOConfig &config, GOMidiSystem &midi)
  : p_MainWindow(pMainWindow),
    r_config(config),
    r_MidiSystem(midi),
    m_OrganFileReady(false),
    m_listener() {
  m_listener.Register(&r_MidiSystem);
}

GOGuiOrgan::~GOGuiOrgan() {
  m_listener.SetCallback(NULL);
  CloseOrgan();
}

/*
 * lifecycle-hook bodies
 */

void GOGuiOrgan::LoadOrganGui(GOConfigReader &cfg) {
  m_IsOrganGuiLoaded = true;
  mp_ImageCache.emplace(mp_controller->GetFileStore());

  unsigned nPanels = cfg.ReadInteger(
    ODFSetting, WX_ORGAN, wxT("NumberOfPanels"), 0, 100, false);

  m_PanelCreators.push_back(
    new GOGUICouplerPanel(*this, mp_controller->GetVirtualCouplers()));
  m_PanelCreators.push_back(new GOGUICouplerManualsAndVolumePanel(*this));
  m_PanelCreators.push_back(new GOGUIMetronomePanel(*this));
  m_PanelCreators.push_back(new GOGUICrescendoPanel(*this));
  m_PanelCreators.push_back(new GOGUIDivisionalsPanel(*this));
  m_PanelCreators.push_back(new GOGUIBankedGeneralsPanel(*this));
  m_PanelCreators.push_back(new GOGUISequencerPanel(*this));
  m_PanelCreators.push_back(new GOGUIMasterPanel(*this));
  m_PanelCreators.push_back(new GOGUIRecorderPanel(*this));

  mp_controller->GetPitchLabel()->Load(
    cfg, wxT("SetterMasterPitch"), _("organ pitch"));
  mp_controller->GetTemperamentLabel()->Load(
    cfg, wxT("SetterMasterTemperament"), _("temperament"));
  mp_MainWindowData.emplace(*mp_controller, wxT("MainWindow"));
  mp_MainWindowData->Load(cfg);

  m_panels.resize(0);
  m_panels.push_back(
    new GOGUIPanel(&*mp_controller, *mp_ImageCache, m_MouseState));
  m_panels[0]->Load(cfg, wxT(""));

  wxString buffer;

  for (unsigned i = 0; i < nPanels; i++) {
    buffer.Printf(wxT("Panel%03d"), i + 1);
    m_panels.push_back(
      new GOGUIPanel(&*mp_controller, *mp_ImageCache, m_MouseState));
    m_panels[i + 1]->Load(cfg, buffer);
  }

  m_StopWindowSizeKeeper.Load(cfg, wxT("Stops"));

  for (GOGUIPanelCreator *pPanelCreator : m_PanelCreators)
    pPanelCreator->CreatePanels(cfg);

  for (GOGUIPanel *pPanel : m_panels)
    pPanel->Layout();
}

void GOGuiOrgan::SyncState() {
  if (p_MainWindow)
    mp_MainWindowData->SetWindowRect(p_MainWindow->GetPosSize());
  for (GOGUIPanel *pPanel : m_panels)
    pPanel->SetInitialOpenWindow(false);
  GODocumentBase::SyncState();
}

void GOGuiOrgan::SaveOrganGui(GOConfigWriter &cfg) {
  m_StopWindowSizeKeeper.Save(cfg);
}

void GOGuiOrgan::ClearOrganGui() {
  if (m_IsOrganGuiLoaded) {
    m_panels.clear();
    m_PanelCreators.clear();
    mp_MainWindowData.reset();
    mp_ImageCache.reset();
    m_IsOrganGuiLoaded = false;
  }
}

void GOGuiOrgan::CloseOrgan() {
  m_listener.SetCallback(NULL);
  // m_sound.CloseSound();
  CloseWindows();
  wxTheApp->ProcessPendingEvents();

  m_OrganFileReady = false;
  GOMutexLocker locker(m_lock);

  mp_controller.reset();

  wxCommandEvent event(wxEVT_WINTITLE, 0);
  event.SetString(wxEmptyString);
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

/*
 * the rest - unrelated to the load/save lifecycle
 */

void GOGuiOrgan::OnMidiEvent(const GOMidiEvent &event) {
  GOMutexLocker locker(m_lock);

  if (!m_OrganFileReady)
    return;

  if (mp_controller)
    mp_controller->ProcessMidi(event);
}

/*
 * lifecycle methods
 */

GOOrganController *GOGuiOrgan::LoadOrgan(
  const GOOrgan &organ,
  const wxString &cmb,
  bool isGuiOnly,
  GOProgressMonitor &monitor) {
  wxBusyCursor busy;
  GOConfig &cfg = r_config;

  CloseOrgan();
  mp_controller.emplace(cfg, *this);

  wxString error = mp_controller->Load(organ, cmb, isGuiOnly, monitor);

  if (error.IsEmpty()) {
    cfg.AddOrgan(mp_controller->GetOrganInfo());
    cfg.Flush();
    {
      wxCommandEvent event(wxEVT_SETVALUE, ID_METER_AUDIO_SPIN);
      event.SetInt(mp_controller->GetGain());
      wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
    }

    wxCommandEvent event(wxEVT_WINTITLE, 0);
    event.SetString(mp_controller->GetOrganName());
    wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

    for (unsigned i = 0; i < GetPanelCount(); i++)
      if (GetPanel(i)->InitialOpenWindow())
        ShowPanel(i);

    const GOLogicalRect &mRect(GetMainWindowData()->GetWindowRect());

    if (!mRect.IsEmpty() && p_MainWindow)
      p_MainWindow->SetPosSize(mRect);

    m_OrganFileReady = true;
    m_listener.SetCallback(this);
    if (!cmb.IsEmpty())
      mp_controller->SetOrganModified();
  } else {
    if (error != wxT("!")) {
      wxLogError(wxT("%s\n"), error.c_str());
      GOMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
    }
    CloseOrgan(); // also clears mp_controller
  }
  return mp_controller ? &*mp_controller : nullptr;
}

bool GOGuiOrgan::UpdateCache(bool compress, GOProgressMonitor &monitor) {
  if (!mp_controller)
    return false;
  return mp_controller->UpdateCache(compress, monitor);
}

/*
 * the rest, callee-first
 */

void GOGuiOrgan::ShowPanel(unsigned id) {
  GOGUIPanel *panel = GetPanel(id);

  if (!showWindow(GOGuiOrgan::PANEL, panel)) {
    registerWindow(
      GOGuiOrgan::PANEL, panel, GOGUIPanelView::createWithFrame(this, panel));
  }
}

void GOGuiOrgan::ShowOrganSettingsDialog() {
  if (!showWindow(GOGuiOrgan::ORGAN_DIALOG, NULL) && mp_controller) {
    registerWindow(
      GOGuiOrgan::ORGAN_DIALOG,
      NULL,
      new GOOrganSettingsDialog(*mp_controller, this, nullptr));
  }
}

void GOGuiOrgan::ShowMidiList() {
  if (!showWindow(GOGuiOrgan::MIDI_LIST, NULL) && mp_controller) {
    registerWindow(
      GOGuiOrgan::MIDI_LIST,
      nullptr,
      new GOMidiObjectsDialog(this, nullptr, *mp_controller));
  }
}

void GOGuiOrgan::ShowStops() {
  if (!showWindow(GOGuiOrgan::STOPS, NULL) && mp_controller) {
    auto stopsWindow = new GOStopsWindow(
      this, nullptr, GetStopWindowSizeKeeper(), *mp_controller);

    registerWindow(
      GOGuiOrgan::STOPS,
      stopsWindow, // Otherwise GOStopsWindow::SyncState() wont be called
      stopsWindow);
  }
}

void GOGuiOrgan::ShowMIDIEventDialog(
  GOMidiObject &obj, GOMidiDialogListener *pDialogListener) {
  if (!showWindow(GOGuiOrgan::MIDI_EVENT, &obj) && mp_controller) {
    const wxString title = wxString::Format(
      _("MIDI-Settings for %s - %s"), obj.GetMidiTypeName(), obj.GetName());
    const wxString dialogSelector
      = wxString::Format(wxT("%s.%s"), obj.GetMidiTypeCode(), obj.GetName());

    GOMidiEventDialog *dlg = new GOMidiEventDialog(
      this,
      NULL,
      title,
      mp_controller->GetSettings(),
      dialogSelector,
      obj,
      true,
      pDialogListener);
    dlg->RegisterMIDIListener(mp_controller->GetMidi());
    dlg->SetModificationListener(&*mp_controller);
    registerWindow(GOGuiOrgan::MIDI_EVENT, &obj, dlg);
  }
}