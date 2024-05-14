/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOFrame.h"

#include <wx/choice.h>
#include <wx/display.h>
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/platinfo.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/splash.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>

#include <algorithm>

#include "archive/GOArchiveManager.h"
#include "combinations/GOSetter.h"
#include "config/GOConfig.h"
#include "dialogs/GONewReleaseDialog.h"
#include "dialogs/GOProgressDialog.h"
#include "dialogs/GOSelectOrganDialog.h"
#include "dialogs/GOSplash.h"
#include "dialogs/settings/GOSettingsDialog.h"
#include "dialogs/settings/GOSettingsReason.h"
#include "files/GOStdFileName.h"
#include "gui/GOGUIPanel.h"
#include "gui/primitives/go_gui_utils.h"
#include "help/GOHelpRequestor.h"
#include "loader/cache/GOCacheCleaner.h"
#include "midi/GOMidi.h"
#include "midi/GOMidiEvent.h"
#include "size/GOLogicalRect.h"
#include "sound/GOSound.h"
#include "temperaments/GOTemperament.h"
#include "threading/GOMutexLocker.h"
#include "wxcontrols/GOAudioGauge.h"

#include "GOApp.h"
#include "GODocument.h"
#include "GOEvent.h"
#include "GOOrgan.h"
#include "GOOrganController.h"
#include "GOPath.h"
#include "GOProperties.h"
#include "Images.h"
#include "go_defs.h"
#include "go_ids.h"
#include "go_limits.h"

BEGIN_EVENT_TABLE(GOFrame, wxFrame)
EVT_MSGBOX(GOFrame::OnMsgBox)
EVT_RENAMEFILE(GOFrame::OnRenameFile)
EVT_CLOSE(GOFrame::OnCloseWindow)
EVT_CHAR_HOOK(GOFrame::OnKeyCommand)
EVT_COMMAND(0, wxEVT_METERS, GOFrame::OnMeters)
EVT_COMMAND(0, wxEVT_LOADFILE, GOFrame::OnLoadFile)
EVT_MENU_OPEN(GOFrame::OnMenuOpen)
EVT_MENU(ID_FILE_OPEN, GOFrame::OnOpen)
EVT_MENU(ID_FILE_LOAD, GOFrame::OnLoad)
EVT_MENU(ID_FILE_INSTALL, GOFrame::OnInstall)
EVT_MENU_RANGE(ID_LOAD_FAV_FIRST, ID_LOAD_FAV_LAST, GOFrame::OnLoadFavorite)
EVT_MENU_RANGE(ID_LOAD_LRU_FIRST, ID_LOAD_LRU_LAST, GOFrame::OnLoadRecent)
EVT_MENU(ID_FILE_SAVE, GOFrame::OnSave)
EVT_MENU(ID_FILE_CLOSE, GOFrame::OnMenuClose)
EVT_MENU(ID_FILE_EXIT, GOFrame::OnExit)
EVT_MENU(ID_FILE_RELOAD, GOFrame::OnReload)
EVT_MENU(ID_FILE_REVERT, GOFrame::OnRevert)
EVT_MENU(ID_FILE_PROPERTIES, GOFrame::OnProperties)
EVT_MENU(ID_FILE_IMPORT_COMBINATIONS, GOFrame::OnImportCombinations)
EVT_MENU(ID_FILE_EXPORT_COMBINATIONS, GOFrame::OnExportCombinations)
EVT_MENU(ID_FILE_IMPORT_SETTINGS, GOFrame::OnImportSettings)
EVT_MENU(ID_FILE_EXPORT, GOFrame::OnExport)
EVT_MENU(ID_FILE_CACHE, GOFrame::OnCache)
EVT_MENU(ID_FILE_CACHE_DELETE, GOFrame::OnCacheDelete)
EVT_MENU(ID_ORGAN_EDIT, GOFrame::OnOrganSettings)
EVT_MENU(ID_MIDI_LIST, GOFrame::OnMidiList)
EVT_MENU(ID_MIDI_MONITOR, GOFrame::OnMidiMonitor)
EVT_MENU(ID_AUDIO_PANIC, GOFrame::OnAudioPanic)
EVT_MENU(ID_AUDIO_MEMSET, GOFrame::OnAudioMemset)
EVT_MENU(ID_AUDIO_STATE, GOFrame::OnAudioState)
EVT_MENU(ID_SETTINGS, GOFrame::OnSettings)
EVT_MENU(ID_MIDI_LOAD, GOFrame::OnMidiLoad)
EVT_MENU(wxID_HELP, GOFrame::OnHelp)
EVT_MENU(wxID_ABOUT, GOFrame::OnHelpAbout)
EVT_COMMAND(0, wxEVT_WINTITLE, GOFrame::OnSetTitle)
EVT_MENU(ID_VOLUME, GOFrame::OnSettingsVolume)
EVT_MENU(ID_POLYPHONY, GOFrame::OnSettingsPolyphony)
EVT_MENU(ID_MEMORY, GOFrame::OnSettingsMemoryEnter)
EVT_MENU(ID_TRANSPOSE, GOFrame::OnSettingsTranspose)
EVT_MENU(ID_RELEASELENGTH, GOFrame::OnSettingsReleaseLength)
EVT_MENU_RANGE(ID_PANEL_FIRST, ID_PANEL_LAST, GOFrame::OnPanel)
EVT_MENU_RANGE(ID_PRESET_0, ID_PRESET_LAST, GOFrame::OnPreset)
EVT_MENU_RANGE(ID_TEMPERAMENT_0, ID_TEMPERAMENT_LAST, GOFrame::OnTemperament)
EVT_SIZE(GOFrame::OnSize)
EVT_TEXT(ID_METER_TRANSPOSE_SPIN, GOFrame::OnSettingsTranspose)
EVT_TEXT_ENTER(ID_METER_TRANSPOSE_SPIN, GOFrame::OnSettingsTranspose)
EVT_COMMAND(ID_METER_TRANSPOSE_SPIN, wxEVT_SETVALUE, GOFrame::OnChangeTranspose)
EVT_CHOICE(ID_RELEASELENGTH_SELECT, GOFrame::OnSettingsReleaseLength)
EVT_TEXT(ID_METER_POLY_SPIN, GOFrame::OnSettingsPolyphony)
EVT_TEXT_ENTER(ID_METER_POLY_SPIN, GOFrame::OnSettingsPolyphony)
EVT_TEXT(ID_METER_FRAME_SPIN, GOFrame::OnSettingsMemory)
EVT_TEXT_ENTER(ID_METER_FRAME_SPIN, GOFrame::OnSettingsMemoryEnter)
EVT_COMMAND(ID_METER_FRAME_SPIN, wxEVT_SETVALUE, GOFrame::OnChangeSetter)
EVT_SLIDER(ID_METER_FRAME_SPIN, GOFrame::OnChangeSetter)
EVT_TEXT(ID_METER_AUDIO_SPIN, GOFrame::OnSettingsVolume)
EVT_TEXT_ENTER(ID_METER_AUDIO_SPIN, GOFrame::OnSettingsVolume)
EVT_COMMAND(ID_METER_AUDIO_SPIN, wxEVT_SETVALUE, GOFrame::OnChangeVolume)
EVT_MENU(ID_CHECK_FOR_UPDATES, GOFrame::OnUpdateCheckingRequested)
EVT_MENU(ID_SHOW_RELEASE_CHANGELOG, GOFrame::OnNewReleaseInfoRequested)
EVT_MENU(ID_DOWNLOAD_NEW_RELEASE, GOFrame::OnNewReleaseDownload)
EVT_UPDATE_CHECKING_COMPLETION(GOFrame::OnUpdateCheckingCompletion)
EVT_UPDATE_UI_RANGE(ID_FILE_RELOAD, ID_AUDIO_MEMSET, GOFrame::OnUpdateLoaded)
EVT_UPDATE_UI_RANGE(ID_PRESET_0, ID_PRESET_LAST, GOFrame::OnUpdateLoaded)
END_EVENT_TABLE()

GOFrame::GOFrame(
  GOApp &app,
  wxFrame *frame,
  wxWindowID id,
  const wxString &title,
  const wxPoint &pos,
  const wxSize &size,
  const long type,
  GOSound &sound)
  : wxFrame(frame, id, title, pos, size, type),
    GOHelpRequestor(this),
    m_App(app),
    m_file_menu(NULL),
    m_panel_menu(NULL),
    m_favorites_menu(NULL),
    m_recent_menu(NULL),
    m_doc(NULL),
    m_SamplerUsage(NULL),
    m_VolumeControl(NULL),
    m_VolumeGauge(),
    m_Transpose(NULL),
    m_ReleaseLength(NULL),
    m_Polyphony(NULL),
    m_SetterPosition(NULL),
    m_Volume(NULL),
    m_Sound(sound),
    m_config(sound.GetSettings()),
    m_listener(),
    m_Title(title),
    m_Label(),
    m_MidiMonitor(false),
    m_isMeterReady(false),
    m_IsGuiOnly(false),
    m_InSettings(false),
    m_AfterSettingsEventType(wxEVT_NULL),
    m_AfterSettingsEventId(0),
    p_AfterSettingsEventOrgan(NULL) {
  SetIcon(get_go_icon());

  wxArrayString choices;

  m_file_menu = new wxMenu;

  m_favorites_menu = new wxMenu;

  m_recent_menu = new wxMenu;

  wxMenu *preset_menu = new wxMenu;
  for (unsigned i = ID_PRESET_0; i <= ID_PRESET_LAST; i++)
    preset_menu->Append(
      i,
      wxString::Format(_("Preset %d"), i - ID_PRESET_0),
      wxEmptyString,
      wxITEM_CHECK);

  m_file_menu->Append(
    ID_FILE_LOAD, _("&Load\tCtrl+L"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->Append(wxID_ANY, _("&Favorites"), m_favorites_menu);
  m_file_menu->Append(
    ID_FILE_OPEN, _("&Open\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->Append(wxID_ANY, _("Open &Recent"), m_recent_menu);
  m_file_menu->Append(
    ID_FILE_INSTALL,
    _("&Install organ package\tCtrl+I"),
    wxEmptyString,
    wxITEM_NORMAL);
  m_file_menu->AppendSeparator();
  m_file_menu->Append(
    ID_FILE_PROPERTIES, _("Organ &Properties"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->AppendSeparator();
  m_file_menu->AppendSubMenu(preset_menu, _("Pr&eset"));
  m_file_menu->AppendSeparator();
  m_file_menu->Append(
    ID_FILE_SAVE, _("&Save\tCtrl+S"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->Append(
    ID_FILE_CACHE, _("&Update Cache..."), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->Append(
    ID_FILE_CACHE_DELETE, _("Delete &Cache..."), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->AppendSeparator();
  m_file_menu->Append(
    ID_FILE_RELOAD, _("Re&load"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->Append(
    ID_FILE_REVERT, _("Reset to &Defaults"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->AppendSeparator();
  m_file_menu->Append(
    ID_FILE_IMPORT_COMBINATIONS,
    _("Import &Combinations"),
    wxEmptyString,
    wxITEM_NORMAL);
  m_file_menu->Append(
    ID_FILE_EXPORT_COMBINATIONS,
    _("Export &Combinations"),
    wxEmptyString,
    wxITEM_NORMAL);
  m_file_menu->Append(
    ID_FILE_IMPORT_SETTINGS,
    _("&Import Settings"),
    wxEmptyString,
    wxITEM_NORMAL);
  m_file_menu->Append(
    ID_FILE_EXPORT, _("&Export Settings"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->AppendSeparator();
  m_file_menu->Append(
    ID_SETTINGS, wxT("&Settings..."), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->AppendSeparator();
  m_file_menu->Append(ID_FILE_CLOSE, _("&Close"), wxEmptyString, wxITEM_NORMAL);
  m_file_menu->Append(ID_FILE_EXIT, _("E&xit"), wxEmptyString, wxITEM_NORMAL);

  m_temperament_menu = new wxMenu;

  m_audio_menu = new wxMenu;
  m_audio_menu->AppendSubMenu(m_temperament_menu, _("&Temperament"));
  m_audio_menu->Append(
    ID_ORGAN_EDIT, _("&Organ settings"), wxEmptyString, wxITEM_CHECK);
  m_audio_menu->Append(
    ID_MIDI_LIST, _("M&idi Objects"), wxEmptyString, wxITEM_CHECK);
  m_audio_menu->AppendSeparator();
  m_audio_menu->Append(
    ID_AUDIO_STATE, _("&Sound Output State"), wxEmptyString, wxITEM_NORMAL);
  m_audio_menu->AppendSeparator();
  m_audio_menu->Append(
    ID_AUDIO_PANIC, _("&Panic\tEscape"), wxEmptyString, wxITEM_NORMAL);
  m_audio_menu->Append(
    ID_AUDIO_MEMSET, _("&Memory Set\tShift"), wxEmptyString, wxITEM_CHECK);
  m_audio_menu->AppendSeparator();
  m_audio_menu->Append(
    ID_MIDI_LOAD, _("Load &MIDI\tCtrl+P"), wxEmptyString, wxITEM_NORMAL);
  m_audio_menu->Append(
    ID_MIDI_MONITOR, _("&Log MIDI events"), wxEmptyString, wxITEM_CHECK);

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(wxID_HELP, _("&Help\tF1"), wxEmptyString, wxITEM_NORMAL);
  help_menu->Append(wxID_ABOUT, _("&About"), wxEmptyString, wxITEM_NORMAL);
  help_menu->Append(
    ID_CHECK_FOR_UPDATES,
    _("&Check for updates"),
    wxEmptyString,
    wxITEM_NORMAL);
  m_panel_menu = new wxMenu();

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(m_file_menu, _("&File"));
  menu_bar->Append(m_audio_menu, _("&Audio/Midi"));
  menu_bar->Append(m_panel_menu, _("&Panel"));
  menu_bar->Append(help_menu, _("&Help"));
  SetMenuBar(menu_bar);

  m_ToolBar = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
  m_ToolBar->SetToolBitmapSize(wxSize(16, 16));
  m_ToolBar->AddTool(
    ID_AUDIO_MEMSET,
    _("&Memory Set\tShift"),
    GetImage_set(),
    _("Memory Set"),
    wxITEM_CHECK);
  m_ToolBar->AddTool(
    ID_MEMORY,
    _("&Memory Level"),
    GetImage_memory(),
    _("Memory Level"),
    wxITEM_NORMAL);
  m_SetterPosition = new wxSpinCtrl(
    m_ToolBar,
    ID_METER_FRAME_SPIN,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(50, wxDefaultCoord),
    wxSP_ARROW_KEYS,
    0,
    999);
  m_ToolBar->AddControl(m_SetterPosition);
  m_SetterPosition->SetValue(0);

  m_ToolBar->AddTool(
    ID_VOLUME, _("&Volume"), GetImage_volume(), _("Volume"), wxITEM_NORMAL);
  m_Volume = new wxSpinCtrl(
    m_ToolBar,
    ID_METER_AUDIO_SPIN,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(50, wxDefaultCoord),
    wxSP_ARROW_KEYS,
    -120,
    20);
  m_ToolBar->AddControl(m_Volume);
  m_VolumeControl = new wxControl(m_ToolBar, wxID_ANY);
  m_VolumeControlTool = m_ToolBar->AddControl(m_VolumeControl);
  AdjustVolumeControlWithSettings();
  m_Volume->SetValue(m_config.Volume());

  m_ToolBar->AddTool(
    ID_RELEASELENGTH,
    _("&Release tail length"),
    GetImage_reverb(),
    _("Release tail length"),
    wxITEM_NORMAL);
  choices.clear();
  choices.push_back(_("Max"));
  for (unsigned i = 1; i <= 60; i++)
    choices.push_back(wxString::Format(_("%d ms"), i * 50));
  m_ReleaseLength = new wxChoice(
    m_ToolBar,
    ID_RELEASELENGTH_SELECT,
    wxDefaultPosition,
    wxDefaultSize,
    choices);
  m_ToolBar->AddControl(m_ReleaseLength);
  UpdateReleaseLength(0);

  m_ToolBar->AddTool(
    ID_TRANSPOSE,
    _("&Transpose"),
    GetImage_transpose(),
    _("Transpose"),
    wxITEM_NORMAL);
  m_Transpose = new wxSpinCtrl(
    m_ToolBar,
    ID_METER_TRANSPOSE_SPIN,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(46, wxDefaultCoord),
    wxSP_ARROW_KEYS,
    -11,
    11);
  m_ToolBar->AddControl(m_Transpose);
  m_Transpose->SetValue(m_config.Transpose());

  m_ToolBar->AddTool(
    ID_POLYPHONY,
    _("&Polyphony"),
    GetImage_polyphony(),
    _("Polyphony"),
    wxITEM_NORMAL);
  m_Polyphony = new wxSpinCtrl(
    m_ToolBar,
    ID_METER_POLY_SPIN,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(62, wxDefaultCoord),
    wxSP_ARROW_KEYS,
    1,
    MAX_POLYPHONY);
  m_ToolBar->AddControl(m_Polyphony);

  m_SamplerUsage = new GOAudioGauge(m_ToolBar, wxID_ANY, wxDefaultPosition);
  m_ToolBar->AddControl(m_SamplerUsage);
  m_Polyphony->SetValue(m_config.PolyphonyLimit());

  m_ToolBar->AddTool(
    ID_AUDIO_PANIC,
    _("&Panic\tEscape"),
    GetImage_panic(),
    _("Panic"),
    wxITEM_NORMAL);

  SetAutoLayout(true);

  UpdateSize();

  SetPosSize(m_config.GetMainWindowRect());

  m_listener.Register(&m_Sound.GetMidi());
  m_isMeterReady = true;
}

GOFrame::~GOFrame() {
  m_isMeterReady = false;
  if (m_doc) {
    delete m_doc;
    m_doc = NULL;
  }
  m_listener.SetCallback(NULL);
}

bool GOFrame::AdjustVolumeControlWithSettings() {
  const unsigned count = m_config.GetTotalAudioChannels();
  bool rc = false;

  if (count != m_VolumeGauge.size()) {
    const wxPortId portId = wxPlatformInfo::Get().GetPortId();
    const bool isOsX = portId == wxPORT_COCOA || portId == wxPORT_OSX;
    int volCtlId = m_VolumeControlTool->GetId();
    int volCtlPos = m_ToolBar->GetToolPos(volCtlId);

    // OsX doesn't relayout the toolbar correctly after changing the size of
    // a control so we need to remove it and to reinsert it later
    // But RemoveTool hangs on windows, so we do it only on OsX
    if (isOsX)
      m_ToolBar->RemoveTool(volCtlId);

    m_VolumeGauge.clear();
    m_VolumeControl->DestroyChildren();
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vsizer = NULL;

    for (unsigned i = 0; i < count; i++) {
      if ((i % 2) == 0) {
        vsizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(vsizer);
      }
      GOAudioGauge *gauge
        = new GOAudioGauge(m_VolumeControl, wxID_ANY, wxDefaultPosition);
      m_VolumeGauge.push_back(gauge);
      vsizer->Add(gauge, 0, wxFIXED_MINSIZE);
    }

    m_VolumeControl->SetSizer(sizer);
    sizer->Fit(m_VolumeControl);

    // reinsert the control and relayout the toolbar on OsX
    if (isOsX)
      m_ToolBar->InsertTool(volCtlPos, m_VolumeControlTool);
    m_ToolBar->Realize();
    rc = true;
  }
  return rc;
}

void GOFrame::UpdateSize() {
  wxToolBar *tb = GetToolBar();

  tb->Realize();

  const wxSize bestTbSize(tb->GetBestSize());
  const int bestClientWidth = bestTbSize.GetWidth() + 10;

  SetClientSize(bestClientWidth, 0);

  const wxSize frameSize(GetSize());

  SetMinSize(wxSize(frameSize.GetWidth() / 4, frameSize.GetHeight()));
  SetMaxSize(wxSize(frameSize.GetWidth() * 4, frameSize.GetHeight()));
}

GOLogicalRect GOFrame::GetPosSize() const {
  GOLogicalRect lRect;
  const wxRect gRect(GetRect());

  lRect.x = gRect.x;
  lRect.y = gRect.y;
  lRect.width = gRect.width;
  lRect.height = gRect.height;
  return lRect;
}

void GOFrame::SetPosSize(const GOLogicalRect &lRect) {
  if (lRect.width && lRect.height) { // settings are valid
    wxRect rect(lRect.x, lRect.y, lRect.width, lRect.height);
    const wxRect minSize(GetMinSize());
    const wxRect maxSize(GetMaxSize());

    if (rect.width < minSize.width)
      rect.width = minSize.width;
    if (rect.width > maxSize.width)
      rect.width = maxSize.width;
    if (rect.height < minSize.height)
      rect.height = minSize.height;
    if (rect.height > maxSize.height)
      rect.height = maxSize.height;
    SetSize(rect);
  }
}

void GOFrame::UpdateReleaseLength(unsigned releaseLength) {
  GOOrganController *organController = GetOrganController();
  int releaseLengthIndex = releaseLength / 50;

  if (organController && organController->GetReleaseTail() != releaseLength)
    organController->SetReleaseTail(releaseLength);
  if (m_ReleaseLength->GetSelection() != releaseLengthIndex)
    m_ReleaseLength->SetSelection(releaseLengthIndex);
}

void GOFrame::UpdateVolumeControlWithSettings() {
  m_isMeterReady = false;
  if (AdjustVolumeControlWithSettings())
    UpdateSize();
  m_isMeterReady = true;
}

void GOFrame::Init(const wxString &filename, bool isGuiOnly) {
  if (m_config.CheckForUpdatesAtStartup()) {
    // Start update checker thread that will fire events to this frame
    m_UpdateCheckerThread = GOUpdateChecker::StartThread(
      GOUpdateChecker::CheckReason::STARTUP, this);
  }

  m_IsGuiOnly = isGuiOnly;
  Show(true);

  SettingsReasons settingsReasons;

  m_Sound.SetLogSoundErrorMessages(false);

  bool soundProblems = !m_Sound.AssureSoundIsOpen();

  if (soundProblems)
    settingsReasons.push_back(GOSettingsReason(
      m_Sound.getLastErrorMessage(), GOSettingsDialog::PAGE_AUDIO));
  m_Sound.SetLogSoundErrorMessages(true);

  bool midiProblems
    = !m_Sound.GetMidi().HasActiveDevice() && m_config.IsToCheckMidiOnStart();

  if (!soundProblems && midiProblems)
    settingsReasons.push_back(GOSettingsReason(
      _("No active MIDI input devices"), GOSettingsDialog::PAGE_MIDI_DEVICES));

  if (soundProblems || midiProblems) {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SETTINGS);

    SettingsReasons *const pReasons = new SettingsReasons(settingsReasons);

    event.SetClientData(pReasons);
    GetEventHandler()->AddPendingEvent(event);
  }

  GOArchiveManager manager(m_config, m_config.OrganCachePath());

  manager.RegisterPackageDirectory(m_config.GetPackageDirectory());
  manager.RegisterPackageDirectory(m_config.OrganPackagePath());

  // Remove demo organs that have been registered from temporary (appimage)
  // directories and they are not more valid
  m_config.AddOrgansFromArchives();
  m_config.RemoveInvalidTmpOrgans();

  if (!filename.IsEmpty())
    SendLoadFile(filename);
  else
    switch (m_config.LoadLastFile()) {
    case GOInitialLoadType::LOAD_LAST_USED:
      LoadLastOrgan();
      break;

    case GOInitialLoadType::LOAD_FIRST:
      LoadFirstOrgan();
      break;

    case GOInitialLoadType::LOAD_NONE:
      break;
    }

  m_listener.SetCallback(this);
  GOCacheCleaner clean(m_config);
  clean.Cleanup();
}

void GOFrame::AttachDetachOrganController(bool isToAttach) {
  GOOrganController *pOrgan = GetOrganController();

  if (pOrgan)
    pOrgan->SetModificationListener(isToAttach ? this : nullptr);
}

bool GOFrame::CloseOrgan(bool isForce) {
  bool isClosed = true;

  if (m_doc) {
    if (m_doc->IsModified()) {
      int choice = isForce ? wxYES
                           : wxMessageBox(
                             _("The organ settings have been modified\n"
                               "Do you want to save them?"),
                             _("Save organ settings"),
                             wxYES_NO | wxCANCEL | wxCENTRE,
                             this);

      switch (choice) {
      case wxYES:
        isClosed = m_doc->Save();
        break;
      case wxCANCEL:
        isClosed = false;
        break;
      }
    }

    if (isClosed) {
      GOMutexLocker m_locker(m_mutex, true);

      if (m_locker.IsLocked()) {
        AttachDetachOrganController(false);
        delete m_doc;
        m_doc = NULL;
        UpdatePanelMenu();
      } else
        isClosed = false;
    }
  }
  return isClosed;
}

bool GOFrame::LoadOrgan(const GOOrgan &organ, const wxString &cmb) {
  bool retCode = false;

  if (m_doc) {
    GOProgressDialog dlg;

    retCode = m_doc->LoadOrgan(&dlg, organ, cmb, m_IsGuiOnly);
    OnIsModifiedChanged(false);

    // for reflecting model changes
    AttachDetachOrganController(true);
  }
  return retCode;
}

void GOFrame::Open(const GOOrgan &organ) {
  if (CloseOrgan(false)) {
    GOMutexLocker m_locker(m_mutex, true);

    if (m_locker.IsLocked()) {
      m_doc = new GODocument(this, &m_Sound);
      LoadOrgan(organ);
    }
  }
}

GOOrganController *GOFrame::GetOrganController() const {
  return m_doc ? m_doc->GetOrganController() : nullptr;
}

void GOFrame::OnPanel(wxCommandEvent &event) {
  GOOrganController *organController = GetOrganController();
  unsigned no = event.GetId() - ID_PANEL_FIRST;

  if (organController && no < organController->GetPanelCount())
    m_doc->ShowPanel(no);
}

void GOFrame::OnIsModifiedChanged(bool modified) {
  GOOrganController *organController = GetOrganController();

  if (organController)
    UpdateReleaseLength(organController->GetReleaseTail());
  UpdatePanelMenu();
}

void GOFrame::UpdatePanelMenu() {
  GOOrganController *organController = GetOrganController();
  unsigned panelcount = (organController && organController->GetPanelCount())
    ? organController->GetPanelCount()
    : 0;
  panelcount = std::min(panelcount, (unsigned)(ID_PANEL_LAST - ID_PANEL_FIRST));

  while (m_panel_menu->GetMenuItemCount() > 0)
    m_panel_menu->Destroy(
      m_panel_menu->FindItemByPosition(m_panel_menu->GetMenuItemCount() - 1));

  for (unsigned i = 0; i < panelcount; i++) {
    GOGUIPanel *panel = organController->GetPanel(i);
    wxMenu *menu = NULL;
    if (panel->GetGroupName() == wxEmptyString)
      menu = m_panel_menu;
    else {
      for (unsigned j = 0; j < m_panel_menu->GetMenuItemCount(); j++) {
        wxMenuItem *it = m_panel_menu->FindItemByPosition(j);
        if (it->GetItemLabel() == panel->GetGroupName() && it->GetSubMenu())
          menu = it->GetSubMenu();
      }
      if (!menu) {
        menu = new wxMenu();
        m_panel_menu->AppendSubMenu(menu, panel->GetGroupName());
      }
    }
    wxMenuItem *item = menu->AppendCheckItem(ID_PANEL_FIRST + i, wxT("_"));
    item->SetItemLabel(panel->GetName());
    item->Check(m_doc->WindowExists(GODocument::PANEL, panel) ? true : false);
  }
}

void GOFrame::UpdateFavoritesMenu() {
  while (m_favorites_menu->GetMenuItemCount() > 0)
    m_favorites_menu->Destroy(m_favorites_menu->FindItemByPosition(
      m_favorites_menu->GetMenuItemCount() - 1));

  const ptr_vector<GOOrgan> &organs = m_config.GetOrganList();
  for (unsigned i = 0;
       i < organs.size() && i <= ID_LOAD_FAV_LAST - ID_LOAD_FAV_FIRST;
       i++) {
    m_favorites_menu->AppendCheckItem(
      ID_LOAD_FAV_FIRST + i,
      wxString::Format(
        _("&%d: %s"), (i + 1) % 10, organs[i]->GetUITitle().c_str()));
  }
}

void GOFrame::UpdateRecentMenu() {
  while (m_recent_menu->GetMenuItemCount() > 0)
    m_recent_menu->Destroy(
      m_recent_menu->FindItemByPosition(m_recent_menu->GetMenuItemCount() - 1));

  std::vector<const GOOrgan *> organs = m_config.GetLRUOrganList();
  for (unsigned i = 0;
       i < organs.size() && i <= ID_LOAD_LRU_LAST - ID_LOAD_LRU_FIRST;
       i++) {
    m_recent_menu->AppendCheckItem(
      ID_LOAD_LRU_FIRST + i,
      wxString::Format(
        _("&%d: %s"), (i + 1) % 10, organs[i]->GetUITitle().c_str()));
  }
}

void GOFrame::UpdateTemperamentMenu() {
  GOOrganController *organController = GetOrganController();
  wxString temperament = wxEmptyString;
  if (organController)
    temperament = organController->GetTemperament();

  while (m_temperament_menu->GetMenuItemCount() > 0)
    m_temperament_menu->Destroy(m_temperament_menu->FindItemByPosition(
      m_temperament_menu->GetMenuItemCount() - 1));

  for (unsigned i = 0; i < m_config.GetTemperaments().GetTemperamentCount()
       && i < ID_TEMPERAMENT_LAST - ID_TEMPERAMENT_0;
       i++) {
    const GOTemperament &t = m_config.GetTemperaments().GetTemperament(i);
    wxMenu *menu;
    wxString group = t.GetGroupTitle();
    if (group == wxEmptyString)
      menu = m_temperament_menu;
    else {
      menu = NULL;
      for (unsigned j = 0; j < m_temperament_menu->GetMenuItemCount(); j++) {
        wxMenuItem *it = m_temperament_menu->FindItemByPosition(j);
        if (it->GetItemLabel() == group && it->GetSubMenu())
          menu = it->GetSubMenu();
      }
      if (!menu) {
        menu = new wxMenu();
        m_temperament_menu->AppendSubMenu(menu, group);
      }
    }
    wxMenuItem *e = menu->Append(
      ID_TEMPERAMENT_0 + i, t.GetTitle(), wxEmptyString, wxITEM_CHECK);
    e->Enable(organController);
    e->Check(t.GetName() == temperament);
  }
}

void GOFrame::OnSize(wxSizeEvent &event) {
  wxWindow *child = (wxWindow *)NULL;
  for (wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
       node;
       node = node->GetNext()) {
    wxWindow *win = node->GetData();
    if (!win->IsTopLevel() && !IsOneOfBars(win)) {
      child = win;
    }
  }
  if (child)
    child->SetSize(
      0, 0, GetClientSize().GetWidth(), GetClientSize().GetHeight());
}

void GOFrame::OnMeters(wxCommandEvent &event) {
  if (m_isMeterReady) {
    const std::vector<double> vals = m_Sound.GetEngine().GetMeterInfo();
    if (vals.size() == m_VolumeGauge.size() + 1) {
      m_SamplerUsage->SetValue(33 * vals[0]);
      for (unsigned i = 1; i < vals.size(); i++)
        m_VolumeGauge[i - 1]->SetValue(lrint(32.50000000000001 * vals[i]));
      if (event.GetInt()) {
        for (unsigned i = 0; i < m_VolumeGauge.size(); i++)
          m_VolumeGauge[i]->ResetClip();
        m_SamplerUsage->ResetClip();
      }
    }
  }
}

void GOFrame::OnUpdateLoaded(wxUpdateUIEvent &event) {
  GOOrganController *organController = GetOrganController();

  if (ID_PRESET_0 <= event.GetId() && event.GetId() <= ID_PRESET_LAST) {
    event.Check(m_config.Preset() == (unsigned)(event.GetId() - ID_PRESET_0));
    return;
  }

  if (event.GetId() == ID_AUDIO_MEMSET)
    event.Check(
      organController && organController->GetSetter()
      && organController->GetSetter()->GetState().m_IsActive);
  else if (event.GetId() == ID_ORGAN_EDIT)
    event.Check(m_doc && m_doc->WindowExists(GODocument::ORGAN_DIALOG, NULL));
  else if (event.GetId() == ID_MIDI_LIST)
    event.Check(m_doc && m_doc->WindowExists(GODocument::MIDI_LIST, NULL));
  else if (event.GetId() == ID_MIDI_LIST)
    event.Check(m_MidiMonitor);

  if (event.GetId() == ID_FILE_CACHE_DELETE)
    event.Enable(organController && organController->CachePresent());
  else if (event.GetId() == ID_FILE_CACHE)
    event.Enable(organController && organController->IsCacheable());
  else if (event.GetId() == ID_MIDI_MONITOR)
    event.Enable(true);
  else
    event.Enable(
      organController
      && (event.GetId() == ID_FILE_REVERT ? organController->IsCustomized() : true));
}

void GOFrame::OnPreset(wxCommandEvent &event) {
  unsigned id = event.GetId() - ID_PRESET_0;
  if (id == m_config.Preset())
    return;
  m_config.Preset(id);
  if (m_doc)
    ProcessCommand(ID_FILE_RELOAD);
}

void GOFrame::OnTemperament(wxCommandEvent &event) {
  unsigned id = event.GetId() - ID_TEMPERAMENT_0;
  GOOrganController *organController = GetOrganController();

  if (organController && id < m_config.GetTemperaments().GetTemperamentCount())
    organController->SetTemperament(
      m_config.GetTemperaments().GetTemperament(id).GetName());
}

void GOFrame::OnLoadFile(wxCommandEvent &event) {
  GOOrgan *pOrgan = (GOOrgan *)event.GetClientData();

  if (!m_InSettings) {
    Open(*pOrgan);
    delete pOrgan;
  } else
    SetEventAfterSettings(event.GetEventType(), event.GetId(), pOrgan);
}

void GOFrame::OnLoadFavorite(wxCommandEvent &event) {
  unsigned id = event.GetId() - ID_LOAD_FAV_FIRST;
  const GOOrgan &organ = *m_config.GetOrganList()[id];
  Open(organ);
}

void GOFrame::OnLoadRecent(wxCommandEvent &event) {
  unsigned id = event.GetId() - ID_LOAD_LRU_FIRST;
  const GOOrgan &organ = *m_config.GetLRUOrganList()[id];
  Open(organ);
}

void GOFrame::OnLoad(wxCommandEvent &event) {
  GOSelectOrganDialog dlg(this, m_config);

  if (dlg.ShowModal() == wxID_OK)
    Open(*dlg.GetSelection());
}

void GOFrame::OnOpen(wxCommandEvent &event) {
  wxFileDialog dlg(
    this,
    _("Open organ"),
    m_config.OrganPath(),
    wxEmptyString,
    GOStdFileName::getOdfDlgWildcard(),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (dlg.ShowModal() == wxID_OK) {
    Open(GOOrgan(dlg.GetPath()));
  }
}

void GOFrame::OnInstall(wxCommandEvent &event) {
  wxFileDialog dlg(
    this,
    _("Install organ package"),
    m_config.OrganPath(),
    wxEmptyString,
    GOStdFileName::getPackageDlgWildcard(),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (dlg.ShowModal() == wxID_OK)
    if (InstallOrganPackage(dlg.GetPath())) {
      GOMessageBox(
        _("The organ package has been registered"),
        _("Install organ package"),
        wxOK,
        this);
      m_config.Flush();
    }
}

void GOFrame::OnImportCombinations(wxCommandEvent &event) {
  GOOrganController *pOrganController = GetOrganController();

  if (pOrganController) {
    wxFileDialog dlg(
      this,
      _("Import Combinations"),
      pOrganController->GetCombinationsDir(),
      wxEmptyString,
      _("Combinations files (*.yaml)|*.yaml|Settings files (*.cmb)|*.cmb"),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK)
      pOrganController->LoadCombination(dlg.GetPath());
  }
}

void GOFrame::OnExportCombinations(wxCommandEvent &event) {
  GOOrganController *pOrganController = GetOrganController();

  if (pOrganController) {
    const wxString organCmbDir = pOrganController->GetCombinationsDir();

    wxFileName::Mkdir(organCmbDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

    wxFileDialog dlg(
      this,
      _("Export Combinations"),
      organCmbDir,
      wxEmptyString,
      _("Combination files (*.yaml)|*.yaml"),
      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() == wxID_OK) {
      wxString exportedFilePath = dlg.GetPath();

      if (!exportedFilePath.EndsWith(wxT(".yaml"), NULL))
        exportedFilePath += wxT(".yaml");
      const wxString errMsg
        = pOrganController->ExportCombination(exportedFilePath);

      if (!errMsg.IsEmpty())
        GOMessageBox(
          wxString::Format(
            _("Failed to export combinations to '%s': %s"),
            exportedFilePath,
            errMsg),
          _("Error"),
          wxOK | wxICON_ERROR,
          this);
    }
  }
}

void GOFrame::OnImportSettings(wxCommandEvent &event) {
  GOOrganController *organController = GetOrganController();

  if (organController) {
    wxFileDialog dlg(
      this,
      _("Import Settings"),
      m_config.ExportImportPath(),
      wxEmptyString,
      _("Settings files (*.cmb)|*.cmb"),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
      GOOrgan organ = organController->GetOrganInfo();
      GOMutexLocker m_locker(m_mutex, true);

      if (m_locker.IsLocked()) {
        LoadOrgan(organ, dlg.GetPath());
      }
    }
  }
}

void GOFrame::OnExport(wxCommandEvent &event) {
  if (GetOrganController()) {
    wxFileDialog dlg(
      this,
      _("Export Settings"),
      m_config.ExportImportPath(),
      wxEmptyString,
      _("Settings files (*.cmb)|*.cmb"),
      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() == wxID_OK) {
      wxString exportedFilePath = dlg.GetPath();
      if (!exportedFilePath.EndsWith(wxT(".cmb"), NULL))
        exportedFilePath += wxT(".cmb");
      if (!m_doc->Export(exportedFilePath))
        GOMessageBox(
          wxString::Format(
            _("Failed to export settings to '%s'"), exportedFilePath.c_str()),
          _("Error"),
          wxOK | wxICON_ERROR,
          this);
    }
  }
}

void GOFrame::OnSave(wxCommandEvent &event) {
  if (GetOrganController() && !m_doc->Save())
    GOMessageBox(
      _("Failed to save the organ setting"),
      _("Error"),
      wxOK | wxICON_ERROR,
      this);
}

wxString formatSize(wxLongLong &size) {
  double n = (double)size.ToLong();
  const wxString sizes[] = {
    wxTRANSLATE("KB"), wxTRANSLATE("MB"), wxTRANSLATE("GB"), wxTRANSLATE("TB")};
  int i;

  for (i = 0; i < 3; i++) {
    n /= 1024.0;
    if (n < 1024.0)
      break;
  }
  return wxString::Format(wxT("%.2f %s"), n, wxGetTranslation(sizes[i]));
}

void GOFrame::OnCache(wxCommandEvent &event) {
  bool res = true;
  GOOrganController *organController = GetOrganController();
  GOMutexLocker m_locker(m_mutex, true);

  if (!m_locker.IsLocked())
    return;

  GOProgressDialog dlg;

  if (organController)
    res = organController->UpdateCache(&dlg, m_config.CompressCache());
  if (!res) {
    wxLogError(_("Creating the cache failed"));
    GOMessageBox(
      _("Creating the cache failed"), _("Error"), wxOK | wxICON_ERROR, this);
  }
}

void GOFrame::OnCacheDelete(wxCommandEvent &event) {
  GOOrganController *organController = GetOrganController();

  if (organController)
    organController->DeleteCache();
}

void GOFrame::OnReload(wxCommandEvent &event) {
  GOOrganController *organController = GetOrganController();

  if (organController) {
    GOOrgan organ = organController->GetOrganInfo();

    if (CloseOrgan(false))
      Open(organ);
  }
}

void GOFrame::OnMenuClose(wxCommandEvent &event) {
  if (m_doc)
    CloseOrgan(false);
}

bool GOFrame::CloseProgram(bool isForce) {
  if (m_UpdateCheckerThread) {
    // Stop update checking so that we can safely destruct the thread
    m_UpdateCheckerThread->Stop();
  }

  bool isClosed = CloseOrgan(isForce);

  if (isClosed)
    Destroy();
  return isClosed;
}

void GOFrame::OnExit(wxCommandEvent &event) { CloseProgram(); }

void GOFrame::OnCloseWindow(wxCloseEvent &event) {
  CloseProgram(!event.CanVeto());
}

void GOFrame::OnRevert(wxCommandEvent &event) {
  if (
    wxMessageBox(
      _("Any customizations you have saved to this\norgan definition file "
        "will be lost!\n\nReset to defaults and reload?"),
      _("GrandOrgue"),
      wxYES_NO | wxICON_EXCLAMATION,
      this)
    == wxYES) {
    GOOrganController *organController = GetOrganController();

    if (organController) {
      organController->DeleteSettings();
      LoadOrgan(organController->GetOrganInfo());
    }
  }
}

void GOFrame::OnProperties(wxCommandEvent &event) {
  GOOrganController *organController = GetOrganController();

  if (organController) {
    GOProperties dlg(organController, this);
    dlg.ShowModal();
  }
}

void GOFrame::OnAudioPanic(wxCommandEvent &WXUNUSED(event)) {
  m_Sound.AssureSoundIsClosed();
  m_Sound.AssureSoundIsOpen();
}

void GOFrame::OnMidiMonitor(wxCommandEvent &WXUNUSED(event)) {
  m_MidiMonitor = !m_MidiMonitor;
}

void GOFrame::OnMidiLoad(wxCommandEvent &WXUNUSED(event)) {
  wxFileDialog dlg(
    this,
    _("Load MIDI file"),
    m_config.MidiPlayerPath(),
    wxEmptyString,
    _("MID files (*.mid)|*.mid"),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (dlg.ShowModal() == wxID_OK) {
    GOOrganController *organController = GetOrganController();

    if (organController)
      organController->LoadMIDIFile(dlg.GetPath());
  }
}

void GOFrame::OnAudioMemset(wxCommandEvent &WXUNUSED(event)) {
  GOOrganController *organController = GetOrganController();

  if (organController)
    organController->GetSetter()->ToggleSetter();
}

void GOFrame::SetEventAfterSettings(
  wxEventType eventType, int eventId, GOOrgan *pOrganFile) {
  if (p_AfterSettingsEventOrgan)
    delete p_AfterSettingsEventOrgan;
  m_AfterSettingsEventType = eventType;
  m_AfterSettingsEventId = eventId;
  p_AfterSettingsEventOrgan = pOrganFile;
}

void GOFrame::OnSettings(wxCommandEvent &event) {
  m_InSettings = true;

  bool isToContinue = true; // will GO continue running? Otherwise it will exit
  SettingsReasons *const pReasons = (SettingsReasons *)event.GetClientData();
  GOSettingsDialog dialog(this, m_config, m_Sound, m_Sound.GetMidi(), pReasons);

  if (dialog.ShowModal() == wxID_OK) {
    GOArchiveManager manager(m_config, m_config.OrganCachePath());
    manager.RegisterPackageDirectory(m_config.OrganPackagePath());

    UpdateVolumeControlWithSettings();
    m_config.SetMainWindowRect(GetPosSize());

    // because the sound settings might be changed, close sound.
    // It will reopened later
    m_Sound.AssureSoundIsClosed();

    m_config.Flush();
    if (
      dialog.NeedRestart()
      && wxMessageBox(
           "Some settings changes effect after GrandOrgue "
           "restarts.\n\nWould you like to restart now?",
           "GrandOrgue",
           wxYES_NO | wxICON_QUESTION,
           this)
        == wxYES) {
      m_App.SetRestart();
      SetEventAfterSettings(wxEVT_COMMAND_MENU_SELECTED, ID_FILE_EXIT);
      isToContinue = false;
    } else if (
      dialog.NeedReload() && m_Sound.GetOrganFile() != NULL
      && wxMessageBox(
           _("Some changed settings effect unless the sample "
             "set is reloaded.\n\nWould you like to reload "
             "the sample set now?"),
           _("GrandOrgue"),
           wxYES_NO | wxICON_QUESTION,
           this)
        == wxYES) {
      SetEventAfterSettings(wxEVT_COMMAND_MENU_SELECTED, ID_FILE_RELOAD);
    }
  }
  if (pReasons)
    delete pReasons;

  // The sound might be closed in the settings dialog (for obtaining the list of
  // devices) or later if the settings were changed
  if (isToContinue)
    m_Sound.AssureSoundIsOpen();

  if (m_AfterSettingsEventType != wxEVT_NULL) {
    wxCommandEvent event(m_AfterSettingsEventType, m_AfterSettingsEventId);

    if (p_AfterSettingsEventOrgan)
      event.SetClientData(p_AfterSettingsEventOrgan);
    GetEventHandler()->AddPendingEvent(event);
    p_AfterSettingsEventOrgan = NULL;
    m_AfterSettingsEventType = wxEVT_NULL;
  }
  m_InSettings = false;
}

void GOFrame::OnAudioState(wxCommandEvent &WXUNUSED(event)) {
  GOMessageBox(m_Sound.getState(), _("Sound output"), wxOK, this);
}

void GOFrame::OnOrganSettings(wxCommandEvent &event) {
  if (m_doc)
    m_doc->ShowOrganSettingsDialog();
}

void GOFrame::OnMidiList(wxCommandEvent &event) {
  if (m_doc)
    m_doc->ShowMidiList();
}

void GOFrame::OnHelp(wxCommandEvent &event) {
  GOHelpRequestor::DisplayHelp(_("User Interface"), false);
}

void GOFrame::OnSettingsVolume(wxCommandEvent &event) {
  long n = m_Volume->GetValue();

  m_Sound.GetEngine().SetVolume(n);
  for (unsigned i = 0; i < m_VolumeGauge.size(); i++)
    m_VolumeGauge[i]->ResetClip();
}

void GOFrame::OnSettingsPolyphony(wxCommandEvent &event) {
  long n = m_Polyphony->GetValue();

  m_config.PolyphonyLimit(n);
  m_Sound.GetEngine().SetHardPolyphony(n);
  m_SamplerUsage->ResetClip();
}

void GOFrame::OnSettingsMemoryEnter(wxCommandEvent &event) {
  long n = m_SetterPosition->GetValue();
  GOOrganController *organController = GetOrganController();

  if (organController)
    organController->GetSetter()->SetPosition(n);
}

void GOFrame::OnSettingsMemory(wxCommandEvent &event) {
  long n = m_SetterPosition->GetValue();
  GOOrganController *organController = GetOrganController();

  if (organController)
    organController->GetSetter()->UpdatePosition(n);
}

void GOFrame::OnSettingsTranspose(wxCommandEvent &event) {
  GOOrganController *organController = GetOrganController();

  if (organController) {
    long n = m_Transpose->GetValue();

    m_config.Transpose(n);
    organController->GetSetter()->SetTranspose(n);
  }
}

void GOFrame::OnSettingsReleaseLength(wxCommandEvent &event) {
  UpdateReleaseLength(m_ReleaseLength->GetSelection() * 50);
}

void GOFrame::OnHelpAbout(wxCommandEvent &event) { DoSplash(false); }

void GOFrame::DoSplash(bool timeout) { GOSplash::DoSplash(timeout, this); }

void GOFrame::OnMenuOpen(wxMenuEvent &event) {
  DoMenuUpdates(event.GetMenu());
  if (event.GetMenu() == m_panel_menu)
    UpdatePanelMenu();
  if (event.GetMenu() == m_file_menu)
    UpdateFavoritesMenu();
  if (event.GetMenu() == m_file_menu)
    UpdateRecentMenu();
  if (event.GetMenu() == m_audio_menu)
    UpdateTemperamentMenu();
  event.Skip();
}

void GOFrame::OnChangeTranspose(wxCommandEvent &event) {
  m_Transpose->SetValue(event.GetInt());
}

void GOFrame::OnChangeSetter(wxCommandEvent &event) {
  m_SetterPosition->SetValue(event.GetInt());
}

void GOFrame::OnChangeVolume(wxCommandEvent &event) {
  m_Volume->SetValue(event.GetInt());
}

void GOFrame::OnKeyCommand(wxKeyEvent &event) {
  int k = event.GetKeyCode();
  if (!event.AltDown()) {
    switch (k) {
    case WXK_ESCAPE: {
      ProcessCommand(ID_AUDIO_PANIC);
      break;
    }
    }
  }
  event.Skip();
}

void GOFrame::OnMidiEvent(const GOMidiEvent &event) {
  if (m_MidiMonitor) {
    wxLogWarning(
      _("MIDI event: ") + event.ToString(m_Sound.GetMidi().GetMidiMap()));
  }

  ptr_vector<GOOrgan> &organs = m_config.GetOrganList();
  for (unsigned i = 0; i < organs.size(); i++)
    if (organs[i]->Match(event) && organs[i]->IsUsable(m_config)) {
      SendLoadOrgan(*organs[i]);
      return;
    }
}

void GOFrame::OnSetTitle(wxCommandEvent &event) {
  m_Label = event.GetString();
  if (m_Label == wxEmptyString)
    SetTitle(m_Title);
  else
    SetTitle(m_Title + _(" - ") + m_Label);
}

void GOFrame::OnMsgBox(wxMsgBoxEvent &event) {
  wxMessageBox(event.getText(), event.getTitle(), event.getStyle(), this);
}

void GOFrame::OnRenameFile(wxRenameFileEvent &event) {
  wxFileName filepath = event.getFilename();

  wxFileDialog dlg(
    this,
    _("Save as"),
    event.getDirectory(),
    event.getFilename(),
    event.getFilter(),
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (dlg.ShowModal() == wxID_OK) {
    if (filepath.GetFullPath() != dlg.GetPath())
      GORenameFile(filepath.GetFullPath(), dlg.GetPath());
  } else
    wxRemoveFile(filepath.GetFullPath());

  GOSyncDirectory(filepath.GetPath());
}

void GOFrame::OnUpdateCheckingRequested(wxCommandEvent &event) {
  if (m_UpdateCheckerThread) {
    // Update checking is already in progress, ignore request
    return;
  }
  wxBeginBusyCursor();
  m_UpdateCheckerThread = GOUpdateChecker::StartThread(
    GOUpdateChecker::CheckReason::USER_REQUEST, this);
}

void GOFrame::OnUpdateCheckingCompletion(
  GOUpdateChecker::CompletionEvent &event) {
  // Remove update checker thread
  m_UpdateCheckerThread->Wait();
  m_UpdateCheckerThread = nullptr;

  // Handle result
  const GOUpdateChecker::Result &result = event.GetResult();
  if (result.checkReason == GOUpdateChecker::CheckReason::USER_REQUEST) {
    wxEndBusyCursor();
    if (result.successful) {
      if (result.updateAvailable) {
        GONewReleaseDialog dialog(
          this, m_config, event.GetResult().latestRelease);
        if (dialog.ShowModal() == wxID_OK) {
          m_config.Flush();
        }
      } else {
        wxMessageBox(
          _("You are using the latest GrandOrgue version"),
          _("Update checker"));
      }
    } else {
      wxMessageBox(
        _("Failed to check for updates: %s").Format(result.errorMessage));
    }
  } else if (
    result.checkReason == GOUpdateChecker::CheckReason::STARTUP
    && result.successful && result.updateAvailable) {
    m_StartupUpdateCheckerResult = result;
    wxMenu *updateAvailableMenu = new wxMenu();
    wxString versionChangeStr
      = wxString::Format("%s -> %s", APP_VERSION, result.latestRelease.version);
    updateAvailableMenu->Append(ID_SHOW_RELEASE_CHANGELOG, versionChangeStr);
    updateAvailableMenu->Append(
      ID_SHOW_RELEASE_CHANGELOG, _("&Show changelog"));
    updateAvailableMenu->Append(ID_DOWNLOAD_NEW_RELEASE, _("&Download"));
    wxMenuBar *menuBar = GetMenuBar();
    menuBar->Append(updateAvailableMenu, _("&Update available!"));
  }
}

void GOFrame::OnNewReleaseInfoRequested(wxCommandEvent &event) {
  GONewReleaseDialog dialog(
    this, m_config, m_StartupUpdateCheckerResult.latestRelease);
  if (dialog.ShowModal() == wxID_OK) {
    m_config.Flush();
  }
}

void GOFrame::OnNewReleaseDownload(wxCommandEvent &event) {
  GOUpdateChecker::OpenDownloadPageInBrowser();
}

bool GOFrame::InstallOrganPackage(wxString name) {
  GOArchiveManager manager(m_config, m_config.OrganCachePath());
  wxString result = manager.InstallPackage(name);
  if (result != wxEmptyString) {
    GOMessageBox(result, _("Error"), wxOK | wxICON_ERROR, this);
    return false;
  } else
    return true;
}

void GOFrame::LoadLastOrgan() {
  std::vector<const GOOrgan *> list = m_config.GetLRUOrganList();
  if (list.size() > 0)
    SendLoadOrgan(*list[0]);
}

void GOFrame::LoadFirstOrgan() {
  ptr_vector<GOOrgan> &list = m_config.GetOrganList();
  if (list.size() > 0)
    SendLoadOrgan(*list[0]);
}

void GOFrame::SendLoadFile(wxString filename) {
  wxFileName name = filename;
  if (name.GetExt() == wxT("orgue")) {
    if (InstallOrganPackage(filename))
      LoadLastOrgan();
  } else
    SendLoadOrgan(GOOrgan(filename));
}

void GOFrame::SendLoadOrgan(const GOOrgan &organ) {
  wxCommandEvent evt(wxEVT_LOADFILE, 0);
  evt.SetClientData(new GOOrgan(organ));
  GetEventHandler()->AddPendingEvent(evt);
}
