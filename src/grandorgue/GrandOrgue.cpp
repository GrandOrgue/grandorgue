/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
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


#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include "GrandOrgue.h"
#include "OrganView.h"
#include "OrganDocument.h"
#include "SettingsDialog.h"
#include "GrandOrgueFile.h"
#include "GOrgueDivisional.h"
#include "GOrgueMidi.h"

#include <wx/ipc.h>
#include <wx/snglinst.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/splash.h>

#ifdef __WIN32__
#include <windows.h>
#include <shlobj.h>
#endif

#include <malloc.h>

IMPLEMENT_APP(GOrgueApp)

extern GrandOrgueFile* organfile;

class stConnection : public wxConnection
{
public:
    stConnection() { }
    ~stConnection() { }
    bool OnExecute(const wxString& topic, wxChar* data, int size, wxIPCFormat format)
    {
        GOrgueApp* app = &::wxGetApp();

        app->frame->Raise();
        if (data[0])
            app->AsyncLoadFile(data);

        return true;
    }
};

class stServer : public wxServer
{
public:
    wxConnectionBase* OnAcceptConnection(const wxString& topic)
    {
        GOrgueApp* app = &::wxGetApp();
        if (!app->frame || !app->m_docManager)
            return false;

        if (topic == wxT("open"))
            return new stConnection();
        return 0;
    }
};

class stClient : public wxClient
{
public:
    stClient() { }
    wxConnectionBase* OnMakeConnection() { return new stConnection; }
};

class GOrgueDocManager : public wxDocManager
{
public:
    void OnUpdateFileSave(wxUpdateUIEvent& event);
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GOrgueDocManager, wxDocManager)
    EVT_UPDATE_UI(wxID_SAVE, GOrgueDocManager::OnUpdateFileSave)
END_EVENT_TABLE();

void GOrgueDocManager::OnUpdateFileSave(wxUpdateUIEvent& event)
{
    wxDocument *doc = GetCurrentDocument();
    event.Enable( doc );
}

GOrgueApp::GOrgueApp()
  :frame(NULL),
   m_docManager(NULL),
   m_path(),
   m_help(NULL),
   m_locale(),
   m_server(NULL),
   m_soundSystem(NULL),
   pConfig(NULL),
   single_instance(NULL)
{
}

bool GOrgueApp::OnInit()
{
    int lang = m_locale.GetSystemLanguage();

    switch(lang)
    {
    case wxLANGUAGE_GERMAN:
    case wxLANGUAGE_GERMAN_AUSTRIAN:
    case wxLANGUAGE_GERMAN_BELGIUM:
    case wxLANGUAGE_GERMAN_LIECHTENSTEIN:
    case wxLANGUAGE_GERMAN_LUXEMBOURG:
    case wxLANGUAGE_GERMAN_SWISS:
        m_locale.Init(wxLANGUAGE_GERMAN);
        m_locale.AddCatalog(wxT("GrandOrgue"));
        break;
    default:
        break;
    }

	single_instance = new wxSingleInstanceChecker(wxT(APP_NAME));
	if (single_instance->IsAnotherRunning())
	{
	    wxLogNull logNull;
	    stClient* client = new stClient;
	    wxConnectionBase* connection = client->MakeConnection(wxT("localhost"), wxT(APP_NAME), wxT("open"));
	    if (connection)
	    {
	        connection->Execute(argc > 1 ? argv[1] : wxT(""));
	        connection->Disconnect();
	        delete connection;
	    }
	    delete client;
		return false;
	}
	else
	{
	    m_server = new stServer;
	    if (!m_server->Create(wxT(APP_NAME)))
            ::wxLogError(wxT("Failed to create IPC service."));
	}

	SetAppName(wxT(APP_NAME));
	SetClassName(wxT(APP_NAME));
	SetVendorName(wxT("Our Organ"));
	pConfig = wxConfigBase::Get();
	pConfig->SetRecordDefaults();


	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);
    wxFileSystem::AddHandler(new wxZipFSHandler);
	wxImage::AddHandler(new wxJPEGHandler);
	wxImage::AddHandler(new wxGIFHandler);
	wxImage::AddHandler(new wxPNGHandler);
	srand(::wxGetUTCTime());

	m_help = new wxHtmlHelpController(wxHF_CONTENTS | wxHF_INDEX | wxHF_SEARCH | wxHF_ICONS_BOOK | wxHF_FLAT_TOOLBAR);
        m_help->AddBook(wxFileName(wxT("GrandOrgue.htb")));

#ifdef __WIN32__

	SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
	char docs[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, docs);
	m_path = docs;
	m_path += "\\";
#endif
#ifdef linux
	wxLog *logger=new wxLogStream(&std::cout);
	wxLog::SetActiveTarget(logger);
    m_path=wxT("$HOME"); //TODO??
    //wxFont::SetDefaultEncoding(wxFONTENCODING_CP1250);
#endif
	m_docManager = new GOrgueDocManager;
	new wxDocTemplate(m_docManager, _("Sample set definition files"), wxT("*.organ"), wxEmptyString, wxT("organ"), wxT("Organ Doc"), wxT("Organ View"), CLASSINFO(OrganDocument), CLASSINFO(OrganView));
	m_docManager->SetMaxDocsOpen(1);

	m_soundSystem = new GOrgueSound();
	frame = new GOrgueFrame(m_docManager, (wxFrame*)NULL, wxID_ANY, wxT(APP_NAME), wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE  | wxMAXIMIZE_BOX | wxRESIZE_BORDER);
	frame->DoSplash();
	bool open_sound = m_soundSystem->OpenSound(true, organfile);
	::wxSleep(2);
	frame->Show(true);

    if (!open_sound)
	{
	    SettingsDialog dialog(frame);
        SetTopWindow(&dialog);
	    dialog.ShowModal();
	}
    SetTopWindow(frame);

    if (argc > 1 && argv[1][0])
    {
        AsyncLoadFile(argv[1]);
        argv[1][0] = 0;
        argc = 1;
    }
    GOrgueLCD_Open();

    m_soundSystem->GetMidi().UpdateOrganMIDI(); //retrieve MIDI settings for loading organs

	return true;
}

void GOrgueApp::AsyncLoadFile(wxString what)
{
    if (!frame || !m_docManager)
        return;

    wxFileName fn(what);
    fn.Normalize();
    wxCommandEvent event(wxEVT_LOADFILE, 0);
    event.SetString(fn.GetFullPath());
    frame->AddPendingEvent(event);
}

int GOrgueApp::OnExit()
{
	GOrgueLCD_Close();
	delete m_soundSystem;
	if (m_docManager)
	{
		m_docManager->FileHistorySave(*pConfig);
		delete m_docManager;
	}
	delete m_help;
    delete m_server;
    delete single_instance;

    return wxApp::OnExit();
}

