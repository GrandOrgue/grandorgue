/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "GrandOrgue.h"

#include "GOrgueEvent.h"
#include "GOrgueLCD.h"
#include "GOrgueLog.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include "GrandOrgueDef.h"
#include "GrandOrgueFrame.h"
#include <wx/config.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/image.h>
#include <wx/regex.h>
#include <wx/stdpaths.h>

#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

IMPLEMENT_APP(GOrgueApp)

GOrgueApp::GOrgueApp() :
   m_Frame(NULL),
   m_locale(),
   m_Settings(NULL),
   m_soundSystem(NULL),
   m_Log(NULL),
   m_FileName(),
   m_InstanceName()
{
}

const wxCmdLineEntryDesc GOrgueApp::m_cmdLineDesc [] = {
	{ wxCMD_LINE_SWITCH, wxTRANSLATE("h"), wxTRANSLATE("help"), wxTRANSLATE("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_OPTION, wxTRANSLATE("i"), wxTRANSLATE("instance"), wxTRANSLATE("specifiy GrandOrgue instance name"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL  },
	{ wxCMD_LINE_PARAM,  NULL, NULL, wxTRANSLATE("organ file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_NONE }
};

void GOrgueApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetLogo(wxString::Format(_("GrandOrgue %s - Virtual Pipe Organ Software"), wxT(APP_VERSION)));
	parser.SetDesc (m_cmdLineDesc);
}

bool GOrgueApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	wxString str;
	if (parser.Found(wxT("i"), &str))
	{
		wxRegEx r(wxT("^[A-Za-z0-9]+$"), wxRE_ADVANCED);
		if (!r.Matches(str))
		{
			wxMessageOutput::Get()->Printf(_("Invalid instance name"));
			return false;
		}
		m_InstanceName = wxT("-") + str;
	}
	for (unsigned i = 0; i < parser.GetParamCount(); i++)
		m_FileName = parser.GetParam(i);
	return true;
}

bool GOrgueApp::OnInit()
{
	/* wxMessageOutputStderr break wxLogStderr (fwide), therefore use MessageBox everywhere */
	wxMessageOutput::Set(new wxMessageOutputMessageBox());

	m_locale.Init(wxLANGUAGE_DEFAULT);
	m_locale.AddCatalog(wxT("GrandOrgue"));

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
	SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
#endif

	if (!wxApp::OnInit())
		return false;

	wxConfigBase::Set(new wxConfig(GetAppName() + m_InstanceName));

	m_Settings = new GOrgueSettings(m_InstanceName);
	m_Settings->Load();

	m_soundSystem = new GOrgueSound(*m_Settings);

	m_Frame = new GOrgueFrame(NULL, wxID_ANY, wxString::Format(_("GrandOrgue %s"), wxT(APP_VERSION)), wxDefaultPosition, wxDefaultSize, 
				  wxMINIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE, *m_soundSystem);
	SetTopWindow(m_Frame);
	m_Log = new GOrgueLog(m_Frame);
	wxLog::SetActiveTarget(m_Log);
	m_Frame->DoSplash();
	m_Frame->Init();

	if (!m_FileName.IsEmpty())
		AsyncLoadFile(m_FileName);
	else if (m_Settings->GetLoadLastFile() && m_Settings->GetLastFile() != wxEmptyString)
		AsyncLoadFile(m_Settings->GetLastFile());
	else if (m_Settings->GetLoadLastFile())
	{
		wxString name = wxStandardPaths::Get().GetResourcesDir() + wxFILE_SEP_PATH + wxT("demo") + wxFILE_SEP_PATH + wxT("demo.organ");
		if (wxFileExists(name))
			AsyncLoadFile(name);
	}

	GOrgueLCD_Open();

	return true;
}

void GOrgueApp::MacOpenFile(const wxString &filename)
{
	AsyncLoadFile(filename);
}

void GOrgueApp::AsyncLoadFile(wxString what)
{
	if (m_Frame)
	{
		wxFileName fn(what);
		fn.Normalize();
		wxCommandEvent event(wxEVT_LOADFILE, 0);
		event.SetString(fn.GetFullPath());
		m_Frame->GetEventHandler()->AddPendingEvent(event);
	}
}

int GOrgueApp::OnRun()
{
	return wxApp::OnRun();
}

int GOrgueApp::OnExit()
{
	GOrgueLCD_Close();
	delete m_soundSystem;
	delete m_Settings;
	wxLog::SetActiveTarget(NULL);
	delete m_Log;

	return wxApp::OnExit();
}

