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

#include "GrandOrgueFrame.h"

#include <wx/menu.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/toolbar.h>
#include <wx/icon.h>
#include <wx/config.h>
#include <wx/progdlg.h>
#include <wx/html/helpctrl.h>
#include <wx/splash.h>
#include "GOrgueMeter.h"
#include "GOrgueMidi.h"
#include "GOrguePipe.h"
#include "GOrgueProperties.h"
#include "GOrgueSound.h"
#include "GrandOrgueID.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "OrganDocument.h"
#include "SettingsDialog.h"
#include "SplashScreen.h"
#include "zlib.h"


// New PNG Icons for the toolbar images added by Graham Goode in Nov 2009
#include "images/help.h"
#include "images/open.h"
#include "images/panic.h"
#include "images/properties.h"
#include "images/record.h"
#include "images/reload.h"
#include "images/save.h"
#include "images/settings.h"
#include "images/set.h"
#include "images/volume.h"
#include "images/polyphony.h"
#include "images/memory.h"
#include "images/transpose.h"
#include "images/gauge.h"
#include "images/splash.h"

// end
IMPLEMENT_CLASS(GOrgueFrame, wxDocParentFrame)
BEGIN_EVENT_TABLE(GOrgueFrame, wxDocParentFrame)
	EVT_COMMAND(0, wxEVT_METERS, GOrgueFrame::OnMeters)
	EVT_COMMAND(0, wxEVT_LOADFILE, GOrgueFrame::OnLoadFile)
    EVT_MENU_OPEN(GOrgueFrame::OnMenuOpen)
	EVT_MENU(ID_FILE_OPEN, GOrgueFrame::OnOpen)
	EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, GOrgueFrame::OnOpen)
	EVT_MENU(ID_FILE_RELOAD, GOrgueFrame::OnReload)
	EVT_MENU(ID_FILE_REVERT, GOrgueFrame::OnRevert)
	EVT_MENU(ID_FILE_PROPERTIES, GOrgueFrame::OnProperties)
	EVT_MENU(ID_FILE_LOAD, GOrgueFrame::OnLoad)
	EVT_MENU(ID_FILE_SAVE, GOrgueFrame::OnSave)
	EVT_MENU(ID_FILE_CACHE, GOrgueFrame::OnCache)
	EVT_MENU(ID_AUDIO_PANIC, GOrgueFrame::OnAudioPanic)
	EVT_MENU(ID_AUDIO_RECORD, GOrgueFrame::OnAudioRecord)
	EVT_MENU(ID_AUDIO_MEMSET, GOrgueFrame::OnAudioMemset)
	EVT_MENU(ID_AUDIO_SETTINGS, GOrgueFrame::OnAudioSettings)
	EVT_MENU(wxID_HELP, GOrgueFrame::OnHelp)
	EVT_MENU(wxID_ABOUT, GOrgueFrame::OnHelpAbout)
	// New events for Volume, Polyphony, Memory Level, and Transpose
	EVT_MENU(ID_VOLUME, GOrgueFrame::OnSettingsVolume)
	EVT_MENU(ID_POLYPHONY, GOrgueFrame::OnSettingsPolyphony)
	EVT_MENU(ID_MEMORY, GOrgueFrame::OnSettingsMemory)
	EVT_MENU(ID_TRANSPOSE, GOrgueFrame::OnSettingsTranspose)
	// End

	EVT_UPDATE_UI(wxID_SAVE, GOrgueFrame::OnUpdateLoaded)
	EVT_UPDATE_UI_RANGE(ID_FILE_RELOAD, ID_AUDIO_MEMSET, GOrgueFrame::OnUpdateLoaded)
END_EVENT_TABLE()

extern GOrgueSound* g_sound;
extern GrandOrgueFile* organfile;



void GOrgueFrame::AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString, unsigned char* toolbarImage, int size, wxItemKind kind)
{
	menu->Append(id, item, wxEmptyString, kind);
	if (toolbarImage)
	{
        wxMemoryInputStream mem(toolbarImage, size);
        wxImage img(mem, wxBITMAP_TYPE_PNG);
        GetToolBar()->AddTool(id, item, wxBitmap(img), helpString, kind);
	}
}

GOrgueFrame::GOrgueFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type)
    : wxDocParentFrame(manager, frame, id, title, pos, size, type)
{
	m_gaugedc = 0;
	m_opening = false;

#ifdef _WIN32
	SetIcon(wxIcon(wxT("#101")));
#endif

	wxMenu *file_menu = new wxMenu;

	wxMenu *recent_menu = new wxMenu;
	m_docManager->FileHistoryUseMenu(recent_menu);
	m_docManager->FileHistoryLoad(*wxConfigBase::Get());

	wxToolBar* tb = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
	tb->SetToolBitmapSize(wxSize(16, 16));

	AddTool(file_menu, ID_FILE_OPEN, _("&Open...\tCtrl+O"), _("Open"), Icon_open, sizeof(Icon_open));
	file_menu->Append(wxID_ANY, _("Open &Recent"), recent_menu);
	file_menu->AppendSeparator();
	AddTool(file_menu, ID_FILE_RELOAD, _("Re&load"), _("Reload"), Icon_reload, sizeof(Icon_reload));
	AddTool(file_menu, ID_FILE_REVERT, _("Reset to &Defaults"));
	file_menu->AppendSeparator();
	AddTool(file_menu, ID_FILE_LOAD, _("&Import Settings..."));
	AddTool(file_menu, ID_FILE_SAVE, _("&Export Settings..."));
	AddTool(file_menu, ID_FILE_CACHE, _("&Update Cache..."));
	file_menu->AppendSeparator();
	AddTool(file_menu, wxID_SAVE, _("&Save\tCtrl+S"), _("Save"), Icon_save, sizeof(Icon_save));
	AddTool(file_menu, wxID_CLOSE, _("&Close"));
	AddTool(file_menu, ID_FILE_PROPERTIES, _("&Properties..."), _("Properties"), Icon_properties, sizeof(Icon_properties));
	file_menu->AppendSeparator();
	AddTool(file_menu, wxID_EXIT, _("E&xit"));
	tb->AddSeparator();

	wxMenu *audio_menu = new wxMenu;
	AddTool(audio_menu, ID_AUDIO_RECORD, _("&Record...\tCtrl+R"), _("Record"), Icon_record, sizeof(Icon_record), wxITEM_CHECK);
	AddTool(audio_menu, ID_AUDIO_MEMSET, _("&Memory Set\tShift"), _("Memory Set"), Icon_set, sizeof(Icon_set), wxITEM_CHECK);
	audio_menu->AppendSeparator();
	AddTool(audio_menu, ID_AUDIO_PANIC, _("&Panic\tEscape"), _("Panic"), Icon_panic, sizeof(Icon_panic));
	AddTool(audio_menu, ID_AUDIO_SETTINGS, _("&Settings..."), _("Audio Settings"), Icon_settings, sizeof(Icon_settings));
	//tb->AddSeparator();

	wxMenu *help_menu = new wxMenu;
	//AddTool(help_menu, wxID_HELP, _("&Help\tF1"), _("Help"), Icon_help, sizeof(Icon_help));
	AddTool(help_menu, wxID_HELP, _("&Help\tF1"), _("Help"));
	AddTool(help_menu, wxID_ABOUT, _("&About"));
	tb->AddSeparator();
	// Changed Text to Icons to reduce screen space - Graham Goode Nov 2009
    wxMenu *settings_menu = new wxMenu;
	{
        wxMemoryInputStream mem(Icon_gauge, sizeof(Icon_gauge));
        wxImage img(mem, wxBITMAP_TYPE_GIF);
        m_gauge = wxBitmap(img);
	}
	m_gaugedc = new wxMemoryDC();
	m_gaugedc->SelectObject(m_gauge);

	m_meters[0] = new GOrgueMeter(tb, ID_METER_AUDIO_SPIN, 3);
	m_meters[1] = new GOrgueMeter(tb, ID_METER_POLY_SPIN,  2);
	m_meters[2] = new GOrgueMeter(tb, ID_METER_FRAME_SPIN, 1);
	m_meters[3] = new GOrgueMeter(tb, ID_METER_TRANSPOSE_SPIN, 0);

	AddTool(settings_menu, ID_VOLUME, _("&Volume"), _("Volume"), Icon_volume, sizeof(Icon_volume));
	tb->AddControl(m_meters[0]);
	AddTool(settings_menu, ID_POLYPHONY, _("&Polyphony"), _("Polyphony"), Icon_polyphony, sizeof(Icon_polyphony));
	tb->AddControl(m_meters[1]);
	AddTool(settings_menu, ID_MEMORY, _("&Memory Level"), _("Memory Level"), Icon_memory, sizeof(Icon_memory));
	tb->AddControl(m_meters[2]);
	AddTool(settings_menu, ID_TRANSPOSE, _("&Transpose"), _("Transpose"), Icon_transpose, sizeof(Icon_transpose));
	tb->AddControl(m_meters[3]);


	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, _("&File"));
	menu_bar->Append(audio_menu, _("&Audio"));
	menu_bar->Append(help_menu, _("&Help"));
	SetMenuBar(menu_bar);
	tb->Realize();

	SetClientSize(880, 495);	// default minimal size
	Center(wxBOTH);


    m_pedalBrush.SetColour(0xA0, 0x80, 0x40);

}

GOrgueFrame::~GOrgueFrame()
{
    if (m_gaugedc)
    {
        delete m_gaugedc;
        m_gaugedc = 0;
    }
}

void GOrgueFrame::OnMeters(wxCommandEvent& event)
{
	int n = event.GetInt();
	m_meters[0]->SetValue(0, n);
	m_meters[0]->SetValue(1, n >> 8);
	m_meters[1]->SetValue(0, n >> 16);
}

void GOrgueFrame::OnUpdateLoaded(wxUpdateUIEvent& event)
{
	if (g_sound)
	{
		if (event.GetId() == ID_AUDIO_RECORD)
			event.Check(g_sound->IsRecording());
		else if (event.GetId() == ID_AUDIO_MEMSET)
			event.Check(g_sound->GetMidi().SetterActive());
	}
	event.Enable(organfile && m_docManager->GetCurrentDocument() && (event.GetId() == ID_FILE_REVERT ? organfile->IsCustomized() : true));
}

void GOrgueFrame::OnLoadFile(wxCommandEvent& event)
{
    if (!IsEnabled())
        return;
   bool prev = false;
    if (organfile)
    {
        prev = m_opening;
        m_opening = true;
    }
    m_docManager->CreateDocument(event.GetString(), wxDOC_SILENT);
    if (organfile)  // hopefully so...
        m_opening = prev;
}

void GOrgueFrame::OnOpen(wxCommandEvent& event)
{
	if (organfile)
		m_opening = true;
	if (event.GetId() == ID_FILE_OPEN)
	{
		wxFileName fn = wxFileName::GetCwd();
		fn.AppendDir(wxT("organs"));
		::wxGetApp().m_docManager->SetLastDirectory(wxConfig::Get()->Read(wxT("organPath"), fn.GetPath()));
		ProcessCommand(wxID_OPEN);
		if (organfile)
		{
			wxConfig::Get()->Write(wxT("organPath"), ::wxGetApp().m_docManager->GetLastDirectory());
			m_opening = false;
		}
	}
	else
		event.Skip();
}

void GOrgueFrame::OnLoad(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc || !organfile)
        return;

    wxFileDialog dlg(::wxGetApp().frame, _("Import Settings"), wxConfig::Get()->Read(wxT("cmbPath"), ::wxGetApp().m_path + wxT("My Organs")), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxOPEN | wxFILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxConfig::Get()->Write(wxT("cmbPath"), dlg.GetDirectory());
        wxString file = organfile->GetODFFilename();
        m_opening = true;
        doc->DoOpenDocument(file, dlg.GetPath());
        if (organfile)
            m_opening = false;
    }
}

void GOrgueFrame::OnSave(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc || !organfile)
        return;

    wxFileDialog dlg(::wxGetApp().frame, _("Export Settings"), wxConfig::Get()->Read(wxT("cmbPath"), ::wxGetApp().m_path + wxT("My Organs")), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxSAVE | wxOVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxConfig::Get()->Write(wxT("cmbPath"), dlg.GetDirectory());
        doc->DoSaveDocument(dlg.GetPath());
        doc->Modify(false);
    }
}

wxString formatSize(wxLongLong& size)
{
    double n = (double)size.ToLong();
    wxChar sizes[][3] = {wxT("KB"), wxT("MB"), wxT("GB"), wxT("TB")};
    int i;

    for (i = 0; i < 3; i++)
    {
        n /= 1024.0;
        if (n < 1024.0)
            break;
    }
    return wxString::Format(wxT("%.2f %s"), n, sizes[i]);
}

void GOrgueFrame::OnCache(wxCommandEvent& event)
{
#if 0
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc || !organfile)
        return;

    int i;
    std::vector<int> todo;
    wxLongLong freespace, required = 0;

    {
        wxBusyCursor busy;

        for (i = 0; i < (int)organfile->m_pipe_filenames.size(); i++)
        {
            if (organfile->m_pipe_filenames[i].IsEmpty())
                continue;
            wxString temp = organfile->m_pipe_filenames[i].BeforeLast('-');
            if (!::wxFileExists(organfile->m_pipe_filenames[i]) || (::wxFileExists(temp) && ::wxFileModificationTime(temp) > ::wxFileModificationTime(organfile->m_pipe_filenames[i])))
                todo.push_back(i);
        }
        if (todo.empty())
        {
            ::wxLogWarning("The sample cache is already up to date.");
            return;
        }

        ::wxGetDiskSpace(wxFileName(organfile->m_pipe_filenames[todo[0]]).GetPath(), 0, &freespace);
        for (i = 0; i < (int)todo.size(); i++)
            required += organfile->m_pipe_filesizes[todo[i]];
    }

    int ffile, written;
    if (::wxMessageBox(_(
        "Updating the sample cache decreases\n"
        "load time at the cost of disk space.\n"
        "\n"
        "Required: " ) + formatSize(required)  + _("\n"
        "Available: ") + formatSize(freespace) + _("\n"
        "\n"
        "Continue with this operation?"), APP_NAME, wxYES_NO | wxICON_QUESTION) == wxYES)
    {
        wxProgressDialog dlg(_("Updating sample cache"), wxEmptyString, todo.size(), 0, wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
        for (i = 0; i < (int)todo.size(); i++)
        {
            char* fn = (char*)organfile->m_pipe_filenames[todo[i]].c_str();
            if (!dlg.Update(i, fn))
                break;

            #ifdef linux
                ffile = open(fn, O_WRONLY | O_CREAT,S_IREAD | S_IWRITE);
            #endif
            #ifdef __WIN32__
            ffile = _open(fn, _O_BINARY | _O_WRONLY | _O_SEQUENTIAL | _O_CREAT, _S_IREAD | _S_IWRITE);
            #endif
            if (ffile == -1)
            {
                ::wxLogWarning(_("Could not write to '%s'"), fn);
                break;
            }

            GOrguePipe* pipe = organfile->GetPipe(todo[i]);
            int size = organfile->m_pipe_filesizes[todo[i]];
            pipe->_this = pipe;
            pipe->_adler32 = adler32(0, (Bytef*)&pipe->_this, size - offsetof(GOrguePipe, _this));
            pipe->_fourcc = *(unsigned *)"GOrgueOc";

            #ifdef linux
            written = write(ffile, pipe, size);
            close(ffile);
            #endif
            #ifdef __WIN32__
             written = _write(ffile, pipe, size);
            _close(ffile);
            #endif

            if (written != organfile->m_pipe_filesizes[todo[i]])
            {
                ::wxRemoveFile(fn);
                ::wxLogWarning(_("Could not write to '%s'"), fn);
                break;
            }
        }
    }
#else
    throw (char*)"< sample cache disabled until further notice by nappleton";
#endif
}

void GOrgueFrame::OnReload(wxCommandEvent& event)
{
	ProcessCommand(wxID_FILE1);
}

void GOrgueFrame::OnRevert(wxCommandEvent& event)
{
    if (organfile && m_docManager->GetCurrentDocument() && ::wxMessageBox(_("Any customizations you have saved to this\norgan definition file will be lost!\n\nReset to defaults and reload?"), wxT(APP_NAME), wxYES_NO | wxICON_EXCLAMATION, this) == wxYES)
    {
        {
            wxLog::EnableLogging(false);
            wxFileConfig cfg(wxEmptyString, wxEmptyString, organfile->GetODFFilename(), wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
            wxLog::EnableLogging(true);
            m_docManager->GetCurrentDocument()->Modify(false);
            organfile->Revert(cfg);
        }
        ProcessCommand(wxID_FILE1);
    }
}

void GOrgueFrame::OnProperties(wxCommandEvent& event)
{
	GOrgueProperties dlg(this);
	dlg.ShowModal();
}

void GOrgueFrame::OnAudioPanic(wxCommandEvent& WXUNUSED(event))
{
	if (!g_sound)
		return;
	g_sound->ResetSound(organfile);
}

void GOrgueFrame::OnAudioRecord(wxCommandEvent& WXUNUSED(event))
{
	if (!g_sound)
		return;

	if (g_sound->IsRecording())
		g_sound->StopRecording();
	else
		g_sound->StartRecording();
}

void GOrgueFrame::OnAudioMemset(wxCommandEvent& WXUNUSED(event))
{
	if (!::wxGetApp().m_docManager->GetCurrentDocument() || !g_sound)
		return;
	g_sound->GetMidi().ToggleSetter();
}

void GOrgueFrame::OnAudioSettings(wxCommandEvent& WXUNUSED(event))
{
	::wxLogDebug(wxT("settingsdialog.."));
	SettingsDialog dialog(this);
	::wxLogDebug(wxT("success"));
	dialog.ShowModal();
	g_sound->GetMidi().UpdateOrganMIDI();
}

void GOrgueFrame::OnHelp(wxCommandEvent& event)
{
    ::wxGetApp().m_help->Display(_("User Interface"));
}

void GOrgueFrame::OnSettingsVolume(wxCommandEvent& event)
{
	//
}

void GOrgueFrame::OnSettingsPolyphony(wxCommandEvent& event)
{
	//
}

void GOrgueFrame::OnSettingsMemory(wxCommandEvent& event)
{
	//
}

void GOrgueFrame::OnSettingsTranspose(wxCommandEvent& event)
{
	//
}

void GOrgueFrame::OnHelpAbout(wxCommandEvent& event)
{
	DoSplash(false);
}

void GOrgueFrame::DoSplash(bool timeout)
{
    wxMemoryInputStream mem(Image_Splash, sizeof(Image_Splash));
	wxImage img(mem, wxBITMAP_TYPE_JPEG);
	wxSplashScreenModal* splash = new wxSplashScreenModal(wxBitmap(img), timeout ? wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT : wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 3000, this, wxID_ANY);
	if (!timeout)
		splash->ShowModal();
}


void GOrgueFrame::OnMenuOpen(wxMenuEvent& event)
{
    DoMenuUpdates(event.GetMenu());
    event.Skip();
}
