/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOApp.h"

#include <wx/cmdline.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/image.h>
#include <wx/regex.h>
#include <wx/stopwatch.h>

#include "config/GOConfig.h"
#include "sound/GOSound.h"

#include "GOFrame.h"
#include "GOLog.h"
#include "GOStdPath.h"
#include "go_defs.h"

#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

IMPLEMENT_APP(GOApp)

GOApp::GOApp()
  : m_Restart(false),
    m_Frame(NULL),
    m_locale(),
    m_config(NULL),
    m_soundSystem(NULL),
    m_Log(NULL),
    m_FileName(),
    m_InstanceName(),
    m_IsGuiOnly(false) {}

static const char *const SWITCH_GUI = "g";
static const char *const SWITCH_HELP = "h";
static const char *const OPTION_INSTANCE = "i";

static const wxCmdLineEntryDesc cmd_line_desc[] = {
  {wxCMD_LINE_SWITCH,
   SWITCH_GUI,
   "gui-only",
   wxTRANSLATE("Load just GUI. Not to load any sound samples"),
   wxCMD_LINE_VAL_NONE,
   0},
  {wxCMD_LINE_SWITCH,
   SWITCH_HELP,
   "help",
   wxTRANSLATE("displays help on the command line parameters"),
   wxCMD_LINE_VAL_NONE,
   wxCMD_LINE_OPTION_HELP},
  {wxCMD_LINE_OPTION,
   OPTION_INSTANCE,
   "instance",
   wxTRANSLATE("specify GrandOrgue instance name"),
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_SWITCH,
   "v",
   "verbose",
   wxTRANSLATE("generate verbose log messages"),
   wxCMD_LINE_VAL_NONE,
   0x0},
  {wxCMD_LINE_PARAM,
   NULL,
   NULL,
   wxTRANSLATE("organ file"),
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_NONE}};

void GOApp::OnInitCmdLine(wxCmdLineParser &parser) {
  parser.SetLogo(wxString::Format(
    _("GrandOrgue %s - Virtual Pipe Organ Software"), wxT(APP_VERSION)));
  parser.SetDesc(cmd_line_desc);
}

bool GOApp::OnCmdLineParsed(wxCmdLineParser &parser) {
  bool res = wxApp::OnCmdLineParsed(parser);

  if (res)
    m_IsGuiOnly = parser.FoundSwitch(SWITCH_GUI) == wxCMD_SWITCH_ON;
  if (res) {
    wxString str;

    if (parser.Found(OPTION_INSTANCE, &str)) {
      wxRegEx r(wxT("^[A-Za-z0-9]+$"), wxRE_ADVANCED);

      if (r.Matches(str))
        m_InstanceName = wxT("-") + str;
      else {
        wxMessageOutput::Get()->Printf(_("Invalid instance name"));
        res = false;
      }
    }
  }
  if (res)
    for (unsigned i = 0; i < parser.GetParamCount(); i++)
      m_FileName = parser.GetParam(i);
  return res;
}

bool GOApp::OnInit() {
  /* wxMessageOutputStderr break wxLogStderr (fwide), therefore use MessageBox
   * everywhere */
  wxMessageOutput::Set(new wxMessageOutputMessageBox());

#ifdef __WXMAC__
  /* This ensures that the executable (when it is not in the form of an OS X
   * bundle, is brought into the foreground). GetCurrentProcess() should not
   * be used as it has been deprecated as of 10.9. We use a "Process
   * Identification Constant" instead. See the "Process Manager Reference"
   * document for more information. */
  static const ProcessSerialNumber PSN = {0, kCurrentProcess};
  TransformProcessType(&PSN, kProcessTransformToForegroundApplication);
#endif

  SetAppName(wxT("GrandOrgue"));
  SetClassName(wxT("GrandOrgue"));
  SetVendorName(wxT("Our Organ"));

  wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);
  wxFileSystem::AddHandler(new wxZipFSHandler);
  wxImage::AddHandler(new wxJPEGHandler);
  wxImage::AddHandler(new wxGIFHandler);
  wxImage::AddHandler(new wxPNGHandler);
  wxImage::AddHandler(new wxBMPHandler);
  wxImage::AddHandler(new wxICOHandler);
  srand(::wxGetUTCTime());

#ifdef __WIN32__
  SetThreadExecutionState(
    ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
#endif

  if (!wxApp::OnInit())
    return false;

  m_config = new GOConfig(m_InstanceName);
  m_config->Load();

  GOStdPath::InitLocaleDir();
  m_locale.Init(m_config->GetLanguageId());
  m_locale.AddCatalog(wxT("GrandOrgue"));

  m_soundSystem = new GOSound(*m_config);

  m_Frame = new GOFrame(
    *this,
    NULL,
    wxID_ANY,
    wxString::Format(_("GrandOrgue %s"), wxT(APP_VERSION)),
    wxDefaultPosition,
    wxDefaultSize,
    wxMINIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX
      | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE,
    *m_soundSystem);
  SetTopWindow(m_Frame);
  m_Log = new GOLog(m_Frame);
  wxLog::SetActiveTarget(m_Log);
  m_Frame->Init(m_FileName, m_IsGuiOnly);

  return true;
}

void GOApp::MacOpenFile(const wxString &filename) {
  if (m_Frame)
    m_Frame->SendLoadFile(filename);
}

int GOApp::OnRun() { return wxApp::OnRun(); }

int GOApp::OnExit() {
  wxLog::SetActiveTarget(NULL);

  int rc = wxApp::OnExit();

  if (m_Restart) {
    wchar_t **cmdargs(argv);

    wxExecute(cmdargs);
  }
  return rc;
}

void GOApp::CleanUp() {
  // Ensure that GOFrame and other objects are destroyed before deleting
  wxApp::CleanUp();

  // CleanUp() may be called even if OnInit() has not succeed, so we need to
  // check
  if (m_soundSystem) {
    delete m_soundSystem;
    m_soundSystem = nullptr;
  }
  if (m_config) {
    delete m_config;
    m_config = nullptr;
  }
  if (m_Log) {
    delete m_Log;
    m_Log = nullptr;
  }
}

void GOApp::SetRestart() { m_Restart = true; }