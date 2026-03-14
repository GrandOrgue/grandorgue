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
#include "frames/GOToolbarWindow.h"
#include "gui/dialogs/GOMidiObjectstDialog.h"
#include "gui/dialogs/midi-event/GOMidiEventDialog.h"
#include "gui/dialogs/organ-settings/GOOrganSettingsDialog.h"
#include "gui/frames/GOStopsWindow.h"
#include "gui/panels/GOGUIPanel.h"
#include "gui/panels/GOGUIPanelView.h"
#include "gui/size/GOResizable.h"
#include "midi/events/GOMidiEvent.h"
#include "sound/GOSoundSystem.h"
#include "threading/GOMutexLocker.h"

#include "GOEvent.h"
#include "GOOrgan.h"
#include "GOOrganController.h"
#include "go_ids.h"

GOGuiOrgan::GOGuiOrgan(GOResizable *pMainWindow, GOSoundSystem *sound)
  : p_MainWindow(pMainWindow),
    m_sound(*sound),
    m_OrganFileReady(false),
    m_OrganController(NULL),
    m_listener() {
  m_listener.Register(&m_sound.GetMidi());
}

GOGuiOrgan::~GOGuiOrgan() {
  m_listener.SetCallback(NULL);
  CloseOrgan();
}

bool GOGuiOrgan::IsModified() const {
  return m_OrganController && m_OrganController->IsOrganModified();
}

GOOrganController *GOGuiOrgan::LoadOrgan(
  GOProgressDialog *dlg,
  const GOOrgan &organ,
  const wxString &cmb,
  bool isGuiOnly) {
  wxBusyCursor busy;
  GOConfig &cfg = m_sound.GetSettings();

  CloseOrgan();
  m_OrganController = new GOOrganController(cfg, true);
  wxString error = m_OrganController->Load(dlg, organ, cmb, isGuiOnly);

  if (error.IsEmpty()) {
    cfg.AddOrgan(m_OrganController->GetOrganInfo());
    cfg.Flush();
    {
      wxCommandEvent event(wxEVT_SETVALUE, ID_METER_AUDIO_SPIN);
      event.SetInt(m_OrganController->GetVolume());
      wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

      m_sound.GetEngine().SetVolume(m_OrganController->GetVolume());
    }

    wxCommandEvent event(wxEVT_WINTITLE, 0);
    event.SetString(m_OrganController->GetOrganName());
    wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);

    for (unsigned i = 0; i < m_OrganController->GetPanelCount(); i++)
      if (m_OrganController->GetPanel(i)->InitialOpenWindow())
        ShowPanel(i);

    const GOLogicalRect &mRect(
      m_OrganController->GetMainWindowData()->GetWindowRect());

    if (!mRect.IsEmpty() && p_MainWindow)
      p_MainWindow->SetPosSize(mRect);

    m_sound.AssignOrganFile(m_OrganController);
    m_OrganFileReady = true;
    m_listener.SetCallback(this);
    if (!cmb.IsEmpty())
      m_OrganController->SetOrganModified();
  } else {
    if (error != wxT("!")) {
      wxLogError(wxT("%s\n"), error.c_str());
      GOMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
    }
    CloseOrgan(); // also clears m_OrganController
  }
  return m_OrganController;
}

bool GOGuiOrgan::UpdateCache(GOProgressDialog *dlg, bool compress) {
  if (!m_OrganController)
    return false;
  return m_OrganController->UpdateCache(dlg, compress);
}

void GOGuiOrgan::ShowPanel(unsigned id) {
  GOGUIPanel *panel = m_OrganController->GetPanel(id);

  if (!showWindow(GOGuiOrgan::PANEL, panel)) {
    registerWindow(
      GOGuiOrgan::PANEL, panel, GOGUIPanelView::createWithFrame(this, panel));
  }
}

void GOGuiOrgan::SyncState() {
  if (p_MainWindow)
    m_OrganController->GetMainWindowData()->SetWindowRect(
      p_MainWindow->GetPosSize());
  for (unsigned i = 0; i < m_OrganController->GetPanelCount(); i++)
    m_OrganController->GetPanel(i)->SetInitialOpenWindow(false);
  GODocumentBase::SyncState();
}

bool GOGuiOrgan::Save() {
  SyncState();
  return m_OrganController->Save();
}

bool GOGuiOrgan::Export(const wxString &cmb) {
  SyncState();
  return m_OrganController->Export(cmb);
}

void GOGuiOrgan::CloseOrgan() {
  m_listener.SetCallback(NULL);
  m_sound.AssignOrganFile(NULL);
  // m_sound.CloseSound();
  CloseWindows();
  wxTheApp->ProcessPendingEvents();

  m_OrganFileReady = false;
  GOMutexLocker locker(m_lock);
  if (m_OrganController) {
    delete m_OrganController;
    m_OrganController = 0;
  }

  wxCommandEvent event(wxEVT_WINTITLE, 0);
  event.SetString(wxEmptyString);
  wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

void GOGuiOrgan::OnMidiEvent(const GOMidiEvent &event) {
  GOMutexLocker locker(m_lock);

  if (!m_OrganFileReady)
    return;

  if (m_OrganController)
    m_OrganController->ProcessMidi(event);
}

void GOGuiOrgan::ShowOrganSettingsDialog() {
  if (!showWindow(GOGuiOrgan::ORGAN_DIALOG, NULL) && m_OrganController) {
    registerWindow(
      GOGuiOrgan::ORGAN_DIALOG,
      NULL,
      new GOOrganSettingsDialog(*m_OrganController, this, nullptr));
  }
}

void GOGuiOrgan::ShowMidiList() {
  if (!showWindow(GOGuiOrgan::MIDI_LIST, NULL) && m_OrganController) {
    registerWindow(
      GOGuiOrgan::MIDI_LIST,
      nullptr,
      new GOMidiObjectsDialog(this, nullptr, *m_OrganController));
  }
}

void GOGuiOrgan::ShowStops() {
  if (!showWindow(GOGuiOrgan::STOPS, NULL) && m_OrganController) {
    auto stopsWindow = new GOStopsWindow(
      this,
      nullptr,
      m_OrganController->GetStopWindowSizeKeeper(),
      *m_OrganController);

    registerWindow(
      GOGuiOrgan::STOPS,
      stopsWindow, // Otherwise GOStopsWindow::SyncState() wont be called
      stopsWindow);
  }
}

void GOGuiOrgan::ShowMIDIEventDialog(
  GOMidiObject &obj, GOMidiDialogListener *pDialogListener) {
  if (!showWindow(GOGuiOrgan::MIDI_EVENT, &obj) && m_OrganController) {
    const wxString title = wxString::Format(
      _("MIDI-Settings for %s - %s"), obj.GetMidiTypeName(), obj.GetName());
    const wxString dialogSelector
      = wxString::Format(wxT("%s.%s"), obj.GetMidiTypeCode(), obj.GetName());

    GOMidiEventDialog *dlg = new GOMidiEventDialog(
      this,
      NULL,
      title,
      m_OrganController->GetSettings(),
      dialogSelector,
      obj,
      true,
      pDialogListener);
    dlg->RegisterMIDIListener(m_OrganController->GetMidi());
    dlg->SetModificationListener(m_OrganController);
    registerWindow(GOGuiOrgan::MIDI_EVENT, &obj, dlg);
  }
}