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

#pragma once

#define APP_NAME "GrandOrgue v0.2.0.1"
#define MAX_POLYPHONY 4096
#define LOW_MEM_CONDITION 33554432
		// (32 megabytes)
//#define wxUSE_UNICODE 1

#include <wx/wx.h>
#include <wx/ipc.h>
#include <wx/file.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/brush.h>
#include <wx/splash.h>
#include <wx/config.h>
#include <wx/region.h>
#include <wx/tglbtn.h>
#include <wx/numdlg.h>
#include <wx/slider.h>
#include <wx/fs_zip.h>
#include <wx/control.h>
#include <wx/mstream.h>
#include <wx/progdlg.h>
#include <wx/propdlg.h>
#include <wx/docview.h>
#include <wx/cmdproc.h>
#include <wx/hashmap.h>
#include <wx/filesys.h>
#include <wx/mimetype.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/snglinst.h>
#include <wx/html/helpctrl.h>
#ifdef __WXMSW__
#include <wx/msw/private.h>
#include <wx/msw/registry.h>
#endif

#include <map>
#include <string>

#include "zlib.h"
#include "RtAudio.h"
#include "RtMidi.h"

#include "wxGaugeAudio.h"
#include "MyMeter.h"
#include "OrganFile.h"
#include "MySound.h"
#include "OrganView.h"
#include "OrganDocument.h"
#include "SettingsDialog.h"
#include "MyProperties.h"

#ifdef __VFD__
#include "MyLCD.h"
#endif

class MyFrame: public wxDocParentFrame
{
	DECLARE_CLASS(MyFrame)
public:
    MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type);
    ~MyFrame(void);

	void OnMeters(wxCommandEvent& event);

    void OnLoadFile(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnLoad(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnCache(wxCommandEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnRevert(wxCommandEvent& event);
	void OnProperties(wxCommandEvent& event);

	void OnAudioPanic(wxCommandEvent& event);
	void OnAudioRecord(wxCommandEvent& event);
	void OnAudioMemset(wxCommandEvent& event);
	void OnAudioSettings(wxCommandEvent& event);

	void OnHelp(wxCommandEvent& event);
	void OnHelpRegister(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);

	//Add by Graham Goode Nov 2009
	void OnSettingsVolume(wxCommandEvent& event);
	void OnSettingsPolyphony(wxCommandEvent& event);
	void OnSettingsMemory(wxCommandEvent& event);
	void OnSettingsTranspose(wxCommandEvent& event);
	//

	void OnKeyCommand(wxKeyEvent& event);

	void OnMenuOpen(wxMenuEvent& event);

	void DoSplash(bool timeout = true);

	void OnUpdateLoaded(wxUpdateUIEvent& event);

	MyMeter* m_meters[4];
	wxMemoryDC* m_gaugedc;
	wxBitmap m_gauge;
	wxBrush m_pedalBrush;

	DECLARE_EVENT_TABLE()
protected:
	void AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString = wxEmptyString, unsigned char* toolbarImage = 0, int size = 0, wxItemKind kind = wxITEM_NORMAL);
};

class stServer;

class MyApp : public wxApp
{
public:
	MyApp();
    bool OnInit();
	int OnExit();
	MyFrame* frame;
	wxDocManager* m_docManager;
	wxString m_path;
	wxHtmlHelpController* m_help;
protected:
    wxLocale m_locale;
    stServer* m_server;
	MySound* soundSystem;
	wxConfigBase* pConfig;
	wxSingleInstanceChecker* single_instance;
};

enum
{
	ID_MYFIRSTMENU = wxID_HIGHEST,

	ID_FILE_OPEN,
	ID_FILE_RELOAD,
	ID_FILE_REVERT,
	ID_FILE_LOAD,
	ID_FILE_SAVE,
	ID_FILE_CACHE,
	ID_FILE_PROPERTIES,

	ID_AUDIO_RECORD,
	ID_AUDIO_MEMSET,
	ID_AUDIO_PANIC,
    ID_AUDIO_SETTINGS,

    // Added by Graham Goode Nov 2009
    ID_VOLUME,
    ID_POLYPHONY,
    ID_MEMORY,
    ID_TRANSPOSE,
    //

	ID_METER_AUDIO_SPIN,
	ID_METER_AUDIO_LEFT,
	ID_METER_AUDIO_RIGHT,
	ID_METER_POLY_SPIN,
	ID_METER_POLY_GAUGE,
	ID_METER_FRAME_SPIN,
	ID_METER_TRANSPOSE_SPIN,
};

DECLARE_APP(MyApp)

/*
 * wxSplashScreenModal
 */

class WXDLLIMPEXP_ADV wxSplashScreenModal : public wxDialog
{
public:
    // for RTTI macros only
    wxSplashScreenModal() {}
    wxSplashScreenModal(const wxBitmap& bitmap, long splashStyle, int milliseconds,
                   wxWindow* parent, wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxSIMPLE_BORDER|wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP);
    ~wxSplashScreenModal();

    void OnCloseWindow(wxCloseEvent& event);
    void OnNotify(wxTimerEvent& event);

    long GetSplashStyle() const { return m_splashStyle; }
    wxSplashScreenWindow* GetSplashWindow() const { return m_window; }
    int GetTimeout() const { return m_milliseconds; }

protected:
    wxSplashScreenWindow*   m_window;
    long                    m_splashStyle;
    int                     m_milliseconds;
    wxTimer                 m_timer;

    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSplashScreenModal)
};
