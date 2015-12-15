/*
 * GrandOrgue - free pipe organ simulator
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

#include "GrandOrgueFrame.h"

#include "GOGUIPanel.h"
#include "GOrgueArchiveManager.h"
#include "GOrgueDocument.h"
#include "GOrgueEvent.h"
#include "GOrgueLimits.h"
#include "GOrgueMidi.h"
#include "GOrgueMidiEvent.h"
#include "GOrgueOrgan.h"
#include "GOrgueProgressDialog.h"
#include "GOrgueProperties.h"
#include "GOrgueStdPath.h"
#include "GOrgueSetter.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include "GOrgueTemperament.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueID.h"
#include "OrganSelectDialog.h"
#include "SettingsDialog.h"
#include "SplashScreen.h"
#include "Images.h"
#include "wxGaugeAudio.h"
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/html/helpctrl.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/splash.h>
#include <wx/toolbar.h>
#include <algorithm>

BEGIN_EVENT_TABLE(GOrgueFrame, wxFrame)
	EVT_MSGBOX(GOrgueFrame::OnMsgBox)
	EVT_CLOSE(GOrgueFrame::OnCloseWindow)
	EVT_KEY_DOWN(GOrgueFrame::OnKeyCommand)
	EVT_COMMAND(0, wxEVT_METERS, GOrgueFrame::OnMeters)
	EVT_COMMAND(0, wxEVT_LOADFILE, GOrgueFrame::OnLoadFile)
	EVT_MENU_OPEN(GOrgueFrame::OnMenuOpen)
	EVT_MENU(ID_FILE_OPEN, GOrgueFrame::OnOpen)
	EVT_MENU(ID_FILE_LOAD, GOrgueFrame::OnLoad)
	EVT_MENU(ID_FILE_INSTALL, GOrgueFrame::OnInstall)
	EVT_MENU_RANGE(ID_LOAD_FAV_FIRST, ID_LOAD_FAV_LAST, GOrgueFrame::OnLoadFavorite)
	EVT_MENU_RANGE(ID_LOAD_LRU_FIRST, ID_LOAD_LRU_LAST, GOrgueFrame::OnLoadRecent)
	EVT_MENU(ID_FILE_SAVE, GOrgueFrame::OnSave)
	EVT_MENU(ID_FILE_CLOSE, GOrgueFrame::OnClose)
	EVT_MENU(ID_FILE_EXIT, GOrgueFrame::OnExit)
	EVT_MENU(ID_FILE_RELOAD, GOrgueFrame::OnReload)
	EVT_MENU(ID_FILE_REVERT, GOrgueFrame::OnRevert)
	EVT_MENU(ID_FILE_PROPERTIES, GOrgueFrame::OnProperties)
	EVT_MENU(ID_FILE_IMPORT_SETTINGS, GOrgueFrame::OnImportSettings)
	EVT_MENU(ID_FILE_IMPORT_COMBINATIONS, GOrgueFrame::OnImportCombinations)
	EVT_MENU(ID_FILE_EXPORT, GOrgueFrame::OnExport)
	EVT_MENU(ID_FILE_CACHE, GOrgueFrame::OnCache)
	EVT_MENU(ID_FILE_CACHE_DELETE, GOrgueFrame::OnCacheDelete)
	EVT_MENU(ID_ORGAN_EDIT, GOrgueFrame::OnEditOrgan)
	EVT_MENU(ID_MIDI_LIST, GOrgueFrame::OnMidiList)
	EVT_MENU(ID_MIDI_MONITOR, GOrgueFrame::OnMidiMonitor)
	EVT_MENU(ID_AUDIO_PANIC, GOrgueFrame::OnAudioPanic)
	EVT_MENU(ID_AUDIO_RECORD, GOrgueFrame::OnAudioRecord)
	EVT_MENU(ID_AUDIO_MEMSET, GOrgueFrame::OnAudioMemset)
	EVT_MENU(ID_AUDIO_STATE, GOrgueFrame::OnAudioState)
	EVT_MENU(ID_AUDIO_SETTINGS, GOrgueFrame::OnAudioSettings)
	EVT_MENU(ID_MIDI_RECORD, GOrgueFrame::OnMidiRecord)
	EVT_MENU(ID_MIDI_PLAY, GOrgueFrame::OnMidiPlay)
	EVT_MENU(wxID_HELP, GOrgueFrame::OnHelp)
	EVT_MENU(wxID_ABOUT, GOrgueFrame::OnHelpAbout)
	EVT_COMMAND(0, wxEVT_SHOWHELP, GOrgueFrame::OnShowHelp)
	EVT_COMMAND(0, wxEVT_WINTITLE, GOrgueFrame::OnSetTitle)
	EVT_MENU(ID_VOLUME, GOrgueFrame::OnSettingsVolume)
	EVT_MENU(ID_POLYPHONY, GOrgueFrame::OnSettingsPolyphony)
	EVT_MENU(ID_MEMORY, GOrgueFrame::OnSettingsMemoryEnter)
	EVT_MENU(ID_TRANSPOSE, GOrgueFrame::OnSettingsTranspose)
	EVT_MENU(ID_RELEASELENGTH, GOrgueFrame::OnSettingsReleaseLength)
	EVT_MENU_RANGE(ID_PANEL_FIRST, ID_PANEL_LAST, GOrgueFrame::OnPanel)
	EVT_MENU_RANGE(ID_PRESET_0, ID_PRESET_LAST, GOrgueFrame::OnPreset)
	EVT_MENU_RANGE(ID_TEMPERAMENT_0, ID_TEMPERAMENT_LAST, GOrgueFrame::OnTemperament)
	EVT_SIZE(GOrgueFrame::OnSize)
	EVT_TEXT(ID_METER_TRANSPOSE_SPIN, GOrgueFrame::OnSettingsTranspose)
	EVT_TEXT_ENTER(ID_METER_TRANSPOSE_SPIN, GOrgueFrame::OnSettingsTranspose)
	EVT_COMMAND(ID_METER_TRANSPOSE_SPIN, wxEVT_SETVALUE, GOrgueFrame::OnChangeTranspose)
	EVT_CHOICE(ID_RELEASELENGTH_SELECT, GOrgueFrame::OnSettingsReleaseLength)
	EVT_TEXT(ID_METER_POLY_SPIN, GOrgueFrame::OnSettingsPolyphony)
	EVT_TEXT_ENTER(ID_METER_POLY_SPIN, GOrgueFrame::OnSettingsPolyphony)
	EVT_TEXT(ID_METER_FRAME_SPIN, GOrgueFrame::OnSettingsMemory)
	EVT_TEXT_ENTER(ID_METER_FRAME_SPIN, GOrgueFrame::OnSettingsMemoryEnter)
	EVT_COMMAND(ID_METER_FRAME_SPIN, wxEVT_SETVALUE, GOrgueFrame::OnChangeSetter)
	EVT_SLIDER(ID_METER_FRAME_SPIN, GOrgueFrame::OnChangeSetter)
	EVT_TEXT(ID_METER_AUDIO_SPIN, GOrgueFrame::OnSettingsVolume)
	EVT_TEXT_ENTER(ID_METER_AUDIO_SPIN, GOrgueFrame::OnSettingsVolume)
	EVT_COMMAND(ID_METER_AUDIO_SPIN, wxEVT_SETVALUE, GOrgueFrame::OnChangeVolume)

	EVT_UPDATE_UI_RANGE(ID_FILE_RELOAD, ID_AUDIO_MEMSET, GOrgueFrame::OnUpdateLoaded)
	EVT_UPDATE_UI_RANGE(ID_PRESET_0, ID_PRESET_LAST, GOrgueFrame::OnUpdateLoaded)
END_EVENT_TABLE()

GOrgueFrame::GOrgueFrame(wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type, GOrgueSound& sound) :
	wxFrame(frame, id, title, pos, size, type),
	m_file_menu(NULL),
	m_panel_menu(NULL),
	m_favorites_menu(NULL),
	m_recent_menu(NULL),
	m_doc(NULL),
	m_Help(NULL),
	m_SamplerUsage(NULL),
	m_VolumeLeft(NULL),
	m_VolumeRight(NULL),
	m_Transpose(NULL),
	m_ReleaseLength(NULL),
	m_Polyphony(NULL),
	m_SetterPosition(NULL),
	m_Volume(NULL),
	m_Sound(sound),
	m_Settings(sound.GetSettings()),
	m_listener(),
	m_Title(title),
	m_Label()
{
	wxIcon icon;
	icon.CopyFromBitmap(GetImage_GOIcon());
	SetIcon(icon);
	
	InitHelp();
	
	wxArrayString choices;
	
	m_file_menu = new wxMenu;
       
	m_favorites_menu = new wxMenu;
	
	m_recent_menu = new wxMenu;
	
	wxToolBar* tb = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
	tb->SetToolBitmapSize(wxSize(16, 16));
	
	wxMenu *preset_menu = new wxMenu;
	for(unsigned i = ID_PRESET_0; i <= ID_PRESET_LAST; i++)
		preset_menu->Append(i,  wxString::Format(_("Preset %d"), i - ID_PRESET_0), wxEmptyString, wxITEM_CHECK);
	
	m_file_menu->Append(ID_FILE_LOAD, _("&Load\tCtrl+L"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(wxID_ANY, _("&Favorites"), m_favorites_menu);
	m_file_menu->Append(ID_FILE_OPEN, _("&Open\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(wxID_ANY, _("Open &Recent"), m_recent_menu);
	m_file_menu->Append(ID_FILE_INSTALL, _("&Install organ package\tCtrl+I"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->AppendSeparator();
	m_file_menu->Append(ID_FILE_PROPERTIES, _("Organ &Properties"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->AppendSeparator();
	m_file_menu->AppendSubMenu(preset_menu, _("Pr&eset"));
	m_file_menu->AppendSeparator();
	m_file_menu->Append(ID_FILE_SAVE, _("&Save\tCtrl+S"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(ID_FILE_CACHE, _("&Update Cache..."), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(ID_FILE_CACHE_DELETE, _("Delete &Cache..."), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->AppendSeparator();
	m_file_menu->Append(ID_FILE_RELOAD, _("Re&load"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(ID_FILE_REVERT, _("Reset to &Defaults"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->AppendSeparator();
	m_file_menu->Append(ID_FILE_IMPORT_SETTINGS, _("&Import Settings"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(ID_FILE_IMPORT_COMBINATIONS, _("Import &Combinations"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(ID_FILE_EXPORT, _("&Export Settings/Combinations"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->AppendSeparator();
	m_file_menu->Append(ID_FILE_CLOSE, _("&Close"), wxEmptyString, wxITEM_NORMAL);
	m_file_menu->Append(ID_FILE_EXIT, _("E&xit"), wxEmptyString, wxITEM_NORMAL);
	
	m_temperament_menu = new wxMenu;
	
	m_audio_menu = new wxMenu;
	m_audio_menu->AppendSubMenu(m_temperament_menu, _("&Temperament"));
	m_audio_menu->Append(ID_ORGAN_EDIT, _("&Organ settings"), wxEmptyString, wxITEM_CHECK);
	m_audio_menu->Append(ID_MIDI_LIST, _("M&idi Objects"), wxEmptyString, wxITEM_CHECK);
	m_audio_menu->AppendSeparator();
	m_audio_menu->Append(ID_AUDIO_SETTINGS, _("Audio/Midi &Settings"), wxEmptyString, wxITEM_NORMAL);
	m_audio_menu->Append(ID_AUDIO_STATE, _("&Sound Output State"), wxEmptyString, wxITEM_NORMAL);
	m_audio_menu->AppendSeparator();
	m_audio_menu->Append(ID_AUDIO_RECORD, _("&Record\tCtrl+R"), wxEmptyString, wxITEM_CHECK);
	m_audio_menu->Append(ID_AUDIO_PANIC, _("&Panic\tEscape"), wxEmptyString, wxITEM_NORMAL);
	m_audio_menu->Append(ID_AUDIO_MEMSET, _("&Memory Set\tShift"), wxEmptyString, wxITEM_CHECK);
	m_audio_menu->AppendSeparator();
	m_audio_menu->Append(ID_MIDI_RECORD, _("R&ecord MIDI\tCtrl+M"), wxEmptyString, wxITEM_CHECK);
	m_audio_menu->Append(ID_MIDI_PLAY, _("Play &MIDI\tCtrl+P"), wxEmptyString, wxITEM_CHECK);
	m_audio_menu->Append(ID_MIDI_MONITOR, _("&Log MIDI events"), wxEmptyString, wxITEM_CHECK);
	
	
	wxMenu *help_menu = new wxMenu;
	help_menu->Append(wxID_HELP, _("&Help\tF1"), wxEmptyString, wxITEM_NORMAL);
	help_menu->Append(wxID_ABOUT, _("&About"), wxEmptyString, wxITEM_NORMAL);
	
	tb->AddTool(ID_AUDIO_MEMSET, _("&Memory Set\tShift"), GetImage_set(), _("Memory Set"), wxITEM_CHECK);
	tb->AddTool(ID_MEMORY, _("&Memory Level"), GetImage_memory(), _("Memory Level"), wxITEM_NORMAL);
	m_SetterPosition = new wxSpinCtrl(tb, ID_METER_FRAME_SPIN, wxEmptyString, wxDefaultPosition, wxSize(50, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 999);
	tb->AddControl(m_SetterPosition);
	m_SetterPosition->SetValue(0);	
	
	tb->AddTool(ID_VOLUME, _("&Volume"), GetImage_volume(), _("Volume"), wxITEM_NORMAL);
	m_Volume = new wxSpinCtrl(tb, ID_METER_AUDIO_SPIN, wxEmptyString, wxDefaultPosition, wxSize(50, wxDefaultCoord), wxSP_ARROW_KEYS, -120, 20);
	tb->AddControl(m_Volume);
	
	{
		wxControl* control = new wxControl(tb, wxID_ANY);
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		
		m_VolumeLeft = new wxGaugeAudio(control, wxID_ANY, wxDefaultPosition);
		sizer->Add(m_VolumeLeft, 0, wxFIXED_MINSIZE);
		m_VolumeRight = new wxGaugeAudio(control, wxID_ANY, wxDefaultPosition);
		sizer->Add(m_VolumeRight, 0, wxFIXED_MINSIZE);
		
		control->SetSizer(sizer);
		sizer->Fit(control);
		control->Layout();
		
		tb->AddControl(control);
	}
	m_Volume->SetValue(m_Settings.Volume());
	
	tb->AddTool(ID_RELEASELENGTH, _("&Release tail length"), GetImage_reverb(), _("Release tail length"), wxITEM_NORMAL);
	choices.clear();
	choices.push_back(_("Max"));
	for(unsigned i = 1; i <= 60; i++)
		choices.push_back(wxString::Format(_("%d ms"), i * 50));
	m_ReleaseLength = new wxChoice(tb, ID_RELEASELENGTH_SELECT, wxDefaultPosition, wxDefaultSize, choices);
	tb->AddControl(m_ReleaseLength);
	unsigned n = m_Settings.ReleaseLength();
	m_ReleaseLength->SetSelection(n / 50);
	m_Sound.GetEngine().SetReleaseLength(n);
	
	tb->AddTool(ID_TRANSPOSE, _("&Transpose"), GetImage_transpose(), _("Transpose"), wxITEM_NORMAL);
	m_Transpose = new wxSpinCtrl(tb, ID_METER_TRANSPOSE_SPIN, wxEmptyString, wxDefaultPosition, wxSize(46, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 11);
	tb->AddControl(m_Transpose);
	m_Transpose->SetValue(0);
	
	tb->AddTool(ID_POLYPHONY, _("&Polyphony"), GetImage_polyphony(), _("Polyphony"), wxITEM_NORMAL);
	m_Polyphony = new wxSpinCtrl(tb, ID_METER_POLY_SPIN, wxEmptyString, wxDefaultPosition, wxSize(62, wxDefaultCoord), wxSP_ARROW_KEYS, 1, MAX_POLYPHONY);
	tb->AddControl(m_Polyphony);
	
	m_SamplerUsage = new wxGaugeAudio(tb, wxID_ANY, wxDefaultPosition);
	tb->AddControl(m_SamplerUsage);
	m_Polyphony->SetValue(m_Settings.PolyphonyLimit());
	
	tb->AddTool(ID_AUDIO_PANIC, _("&Panic\tEscape"), GetImage_panic(), _("Panic"), wxITEM_NORMAL);
	
	
	m_panel_menu = new wxMenu();
	
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(m_file_menu, _("&File"));
	menu_bar->Append(m_audio_menu, _("&Audio/Midi"));
	menu_bar->Append(m_panel_menu, _("&Panel"));
	menu_bar->Append(help_menu, _("&Help"));
	SetMenuBar(menu_bar);
	tb->Realize();
	
	SetClientSize(880, 495);	// default minimal size
	Center(wxBOTH);
	SetAutoLayout(true);
	m_listener.Register(&m_Sound.GetMidi());
}

GOrgueFrame::~GOrgueFrame()
{
	if (m_doc)
		delete m_doc;
	m_listener.SetCallback(NULL);
	if (m_Help)
		delete m_Help;
}

void GOrgueFrame::Init(wxString filename)
{
	Show(true);

	m_Sound.SetLogSoundErrorMessages(false);
	bool open_sound = m_Sound.OpenSound();
	m_Sound.SetLogSoundErrorMessages(true);
	if (!open_sound || !m_Sound.GetMidi().HasActiveDevice())
	{
		wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_AUDIO_SETTINGS);
		GetEventHandler()->AddPendingEvent(event);
	}
	GOrgueArchiveManager manager(m_Settings);
	manager.RegisterPackageDirectory(m_Settings.GetPackageDirectory());
	manager.RegisterPackageDirectory(m_Settings.OrganPackagePath());
	if (!filename.IsEmpty())
		SendLoadFile(filename);
	else if (m_Settings.LoadLastFile())
		LoadLastOrgan();

	m_listener.SetCallback(this);
}

void GOrgueFrame::InitHelp()
{
	m_Help = new wxHtmlHelpController(wxHF_CONTENTS | wxHF_INDEX | wxHF_SEARCH | wxHF_ICONS_BOOK | wxHF_FLAT_TOOLBAR);

	wxString result;
	wxString lang = wxGetLocale()->GetCanonicalName();

	wxString searchpath;
	searchpath.Append(GOrgueStdPath::GetResourceDir() + wxFILE_SEP_PATH + wxT("help"));

	if (!wxFindFileInPath(&result, searchpath, wxT("GrandOrgue_") + lang + wxT(".htb")))
	{
		if (lang.Find(wxT('_')))
			lang = lang.Left(lang.Find(wxT('_')));
		if (!wxFindFileInPath(&result, searchpath, wxT("GrandOrgue_") + lang + wxT(".htb")))
		{
			if (!wxFindFileInPath(&result, searchpath, wxT("GrandOrgue.htb")))
				result = wxT("GrandOrgue.htb");
		}
	}
	wxLogDebug(_("Using helpfile %s (search path: %s)"), result.c_str(), searchpath.c_str());
        m_Help->AddBook(result);
}

bool GOrgueFrame::DoClose(bool force)
{
	if (!m_doc)
		return true;
	GOMutexLocker m_locker(m_mutex, true);
	if(!m_locker.IsLocked())
		return false;
	if (m_doc->IsModified() && m_doc->GetOrganFile() && !force)
	{
		int res = wxMessageBox(_("The organ settings have been modified. Do you want to save the changes?"), m_doc->GetOrganFile()->GetChurchName(), wxYES_NO|wxCANCEL|wxICON_QUESTION, this);
		if (res == wxCANCEL)
			return false;
		if (res == wxYES)
			m_doc->Save();
	}
	delete m_doc;
	m_doc = NULL;
	return true;
}

void GOrgueFrame::Open(const GOrgueOrgan& organ, bool force)
{
	if (!DoClose(force))
		return;
	GOMutexLocker m_locker(m_mutex, true);
	if(!m_locker.IsLocked())
		return;
	GOrgueProgressDialog dlg;
	m_doc = new GOrgueDocument(&m_Sound);
	m_doc->Load(&dlg, organ);
}

GOrgueDocument* GOrgueFrame::GetDocument()
{
	return m_doc;
}

void GOrgueFrame::OnPanel(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	unsigned no = event.GetId() - ID_PANEL_FIRST + 1;
	if (doc && doc->GetOrganFile() && no < doc->GetOrganFile()->GetPanelCount())
		doc->ShowPanel(no);
}

void GOrgueFrame::UpdatePanelMenu()
{
	GOrgueDocument* doc = GetDocument();
	GrandOrgueFile* organfile = doc ? doc->GetOrganFile() : NULL;
	unsigned panelcount = (organfile && organfile->GetPanelCount()) ? organfile->GetPanelCount() - 1 : 0;
	panelcount = std::min (panelcount, (unsigned)(ID_PANEL_LAST - ID_PANEL_FIRST));

	while (m_panel_menu->GetMenuItemCount() > 0)
		m_panel_menu->Destroy(m_panel_menu->FindItemByPosition(m_panel_menu->GetMenuItemCount() - 1));

	for(unsigned i = 0; i < panelcount; i++)
	{
		GOGUIPanel* panel = organfile->GetPanel(i + 1);
		wxMenu* menu = NULL;
		if (panel->GetGroupName() == wxEmptyString)
			menu = m_panel_menu;
		else
		{
			for(unsigned j = 0; j < m_panel_menu->GetMenuItemCount(); j++)
			{
				wxMenuItem* it = m_panel_menu->FindItemByPosition(j);
				if (it->GetItemLabel() == panel->GetGroupName() && it->GetSubMenu())
					menu = it->GetSubMenu();
			}
			if (!menu)
			{
				menu = new wxMenu();
				m_panel_menu->AppendSubMenu(menu, panel->GetGroupName());
			}
		}
		wxMenuItem* item = menu->AppendCheckItem(ID_PANEL_FIRST + i, wxT("_"));
		item->SetItemLabel(panel->GetName());
		item->Check(doc->WindowExists(GOrgueDocument::PANEL, panel) ? true : false);
	}
}

void GOrgueFrame::UpdateFavoritesMenu()
{
	while (m_favorites_menu->GetMenuItemCount() > 0)
		m_favorites_menu->Destroy(m_favorites_menu->FindItemByPosition(m_favorites_menu->GetMenuItemCount() - 1));

	ptr_vector<GOrgueOrgan>& organs = m_Settings.GetOrganList();
	for(unsigned i = 0; i < organs.size() && i <= ID_LOAD_FAV_LAST - ID_LOAD_FAV_FIRST; i++)
	{
		m_favorites_menu->AppendCheckItem(ID_LOAD_FAV_FIRST + i, wxString::Format(_("&%d: %s"), (i + 1) % 10, organs[i]->GetUITitle().c_str()));
	}
}

void GOrgueFrame::UpdateRecentMenu()
{
	while (m_recent_menu->GetMenuItemCount() > 0)
		m_recent_menu->Destroy(m_recent_menu->FindItemByPosition(m_recent_menu->GetMenuItemCount() - 1));

	std::vector<GOrgueOrgan*> organs = m_Settings.GetLRUOrganList();
	for(unsigned i = 0; i < organs.size() && i <= ID_LOAD_LRU_LAST - ID_LOAD_LRU_FIRST; i++)
	{
		m_recent_menu->AppendCheckItem(ID_LOAD_LRU_FIRST + i, wxString::Format(_("&%d: %s"), (i + 1) % 10, organs[i]->GetUITitle().c_str()));
	}
}

void GOrgueFrame::UpdateTemperamentMenu()
{
	GOrgueDocument* doc = GetDocument();
	GrandOrgueFile* organfile = doc ? doc->GetOrganFile() : NULL;
	wxString temperament = wxEmptyString;
	if (organfile)
		temperament = organfile->GetTemperament();

	while (m_temperament_menu->GetMenuItemCount() > 0)
		m_temperament_menu->Destroy(m_temperament_menu->FindItemByPosition(m_temperament_menu->GetMenuItemCount() - 1));

	for(unsigned i = 0; i < m_Settings.GetTemperaments().GetTemperamentCount() && i < ID_TEMPERAMENT_LAST - ID_TEMPERAMENT_0; i++)
	{
		const GOrgueTemperament& t = m_Settings.GetTemperaments().GetTemperament(i);
		wxMenu *menu;
		wxString group = t.GetGroup();
		if (group == wxEmptyString)
			menu = m_temperament_menu;
		else
		{
			menu = NULL;
			for(unsigned j = 0; j < m_temperament_menu->GetMenuItemCount(); j++)
			{
				wxMenuItem* it = m_temperament_menu->FindItemByPosition(j);
				if (it->GetItemLabel() == group && it->GetSubMenu())
					menu = it->GetSubMenu();
			}
			if (!menu)
			{
				menu = new wxMenu();
				m_temperament_menu->AppendSubMenu(menu, group);
			}
		}
		wxMenuItem* e = menu->Append(ID_TEMPERAMENT_0 + i, t.GetTitle(), wxEmptyString, wxITEM_CHECK);
		e->Enable(organfile);
		e->Check(t.GetName() == temperament);
	}
}

void GOrgueFrame::OnSize(wxSizeEvent& event)
{
	wxWindow *child = (wxWindow *)NULL;
	for ( wxWindowList::compatibility_iterator node = GetChildren().GetFirst(); node; node = node->GetNext() )
	{
		wxWindow *win = node->GetData();
		if ( !win->IsTopLevel() && !IsOneOfBars(win) )
		{
			child = win;
		}
	}
	if (child)
		child->SetSize(0, 0, GetClientSize().GetWidth(), GetClientSize().GetHeight());
}


void GOrgueFrame::OnMeters(wxCommandEvent& event)
{
	int n = event.GetInt();
	m_VolumeLeft->SetValue (n & 0xFF);
	m_VolumeRight->SetValue((n >> 8) & 0xFF);
	m_SamplerUsage->SetValue((n >> 16) & 0xFF);
	if (((n >> 24) & 0xff) == 0xF0)
	{
		m_VolumeLeft->ResetClip();
		m_VolumeRight->ResetClip();
		m_SamplerUsage->ResetClip();
	}
}

void GOrgueFrame::OnUpdateLoaded(wxUpdateUIEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	GrandOrgueFile* organfile = doc ? doc->GetOrganFile() : NULL;

	if (ID_PRESET_0 <= event.GetId() && event.GetId() <= ID_PRESET_LAST)
	{
		event.Check(m_Settings.Preset() == (unsigned)(event.GetId() - ID_PRESET_0));
		return;
	}

	if (event.GetId() == ID_AUDIO_RECORD)
		event.Check(m_Sound.IsAudioRecording());
	else if (event.GetId() == ID_MIDI_RECORD)
		event.Check(m_Sound.IsMidiRecording());
	else if (event.GetId() == ID_MIDI_PLAY)
		event.Check(m_Sound.IsMidiPlaying());
	else if (event.GetId() == ID_AUDIO_MEMSET)
		event.Check(organfile && organfile->GetSetter() && organfile->GetSetter()->IsSetterActive());
	else if (event.GetId() == ID_ORGAN_EDIT)
		event.Check(doc && doc->WindowExists(GOrgueDocument::ORGAN_DIALOG, NULL));
	else if (event.GetId() == ID_MIDI_LIST)
		event.Check(doc && doc->WindowExists(GOrgueDocument::MIDI_LIST, NULL));
	else if (event.GetId() == ID_MIDI_LIST)
		event.Check(m_MidiMonitor);

	if (event.GetId() == ID_FILE_CACHE_DELETE)
		event.Enable(organfile && organfile->CachePresent());
	else if (event.GetId() == ID_FILE_CACHE)
		event.Enable(organfile && organfile->IsCacheable());
	else if (event.GetId() == ID_MIDI_MONITOR)
		event.Enable(true);
	else
		event.Enable(organfile && (event.GetId() == ID_FILE_REVERT ? organfile->IsCustomized() : true));
}

void GOrgueFrame::OnPreset(wxCommandEvent& event)
{
	unsigned id = event.GetId() - ID_PRESET_0;
	if (id == m_Settings.Preset())
		return;
	m_Settings.Preset(id);
	if (GetDocument())
		ProcessCommand(ID_FILE_RELOAD);
}

void GOrgueFrame::OnTemperament(wxCommandEvent& event)
{
	unsigned id = event.GetId() - ID_TEMPERAMENT_0;
	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile() && id < m_Settings.GetTemperaments().GetTemperamentCount())
		doc->GetOrganFile()->SetTemperament(m_Settings.GetTemperaments().GetTemperament(id).GetName());
}

void GOrgueFrame::OnLoadFile(wxCommandEvent& event)
{
	GOrgueOrgan* organ = (GOrgueOrgan*)event.GetClientData();
	Open(*organ, event.GetInt() == 1);
	delete organ;
}

void GOrgueFrame::OnLoadFavorite(wxCommandEvent& event)
{
	unsigned id = event.GetId() - ID_LOAD_FAV_FIRST;
	GOrgueOrgan& organ = *m_Settings.GetOrganList()[id];
	Open(organ);
}

void GOrgueFrame::OnLoadRecent(wxCommandEvent& event)
{
	unsigned id = event.GetId() - ID_LOAD_LRU_FIRST;
	GOrgueOrgan& organ = *m_Settings.GetLRUOrganList()[id];
	Open(organ);
}

void GOrgueFrame::OnLoad(wxCommandEvent& event)
{
	OrganSelectDialog dlg(this, _("Select organ to load"), m_Settings);
	if (dlg.ShowModal() != wxID_OK)
		return;
	Open(*dlg.GetSelection());
}

void GOrgueFrame::OnOpen(wxCommandEvent& event)
{
	wxFileDialog dlg(this, _("Open organ"), m_Settings.OrganPath(), wxEmptyString, _("Sample set definition files (*.organ)|*.organ"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
	{
		Open(GOrgueOrgan(dlg.GetPath()));
	}
}

void GOrgueFrame::OnInstall(wxCommandEvent& event)
{
	wxFileDialog dlg(this, _("Install organ package"), m_Settings.OrganPath(), wxEmptyString, _("Organ package (*.orgue)|*.orgue"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
		if (InstallOrganPackage(dlg.GetPath()))
		{
			GOMessageBox(_("The organ package has been registered"), _("Install organ package"), wxOK , this);
			m_Settings.Flush();
		}

}

void GOrgueFrame::OnImportSettings(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (!doc || !doc->GetOrganFile())
		return;

	wxFileDialog dlg(this, _("Import Settings"), m_Settings.SettingPath(), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
	{
		GOrgueProgressDialog pdlg;
		GOrgueOrgan organ = doc->GetOrganFile()->GetOrganInfo();
		GOMutexLocker m_locker(m_mutex, true);
		if(!m_locker.IsLocked())
			return;
		doc->Import(&pdlg, organ, dlg.GetPath());
	}
}

void GOrgueFrame::OnImportCombinations(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (!doc || !doc->GetOrganFile())
		return;

	wxFileDialog dlg(this, _("Import Combinations"), m_Settings.SettingPath(), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
	{
		doc->ImportCombination(dlg.GetPath());
	}
}

void GOrgueFrame::OnExport(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (!doc || !doc->GetOrganFile())
		return;

	wxFileDialog dlg(this, _("Export Settings"), m_Settings.SettingPath(), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK)
	{
		if (!doc->Export(dlg.GetPath()))
			wxMessageBox(wxString::Format(_("Failed to export settings to '%s'"), dlg.GetPath().c_str()), _("Error"), wxOK | wxICON_ERROR, this);
	}
}

void GOrgueFrame::OnSave(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (!doc || !doc->GetOrganFile())
		return;
	if (!doc->Save())
		wxMessageBox(_("Failed to save the organ setting"), _("Error"), wxOK | wxICON_ERROR, this);
}

wxString formatSize(wxLongLong& size)
{
	double n = (double)size.ToLong();
	const wxString sizes[] = {wxTRANSLATE("KB"), wxTRANSLATE("MB"), wxTRANSLATE("GB"), wxTRANSLATE("TB")};
	int i;

	for (i = 0; i < 3; i++)
	{
		n /= 1024.0;
		if (n < 1024.0)
			break;
	}
    return wxString::Format(wxT("%.2f %s"), n, wxGetTranslation(sizes[i]));
}

void GOrgueFrame::OnCache(wxCommandEvent& event)
{
	bool res = true;
	GOrgueDocument* doc = GetDocument();
	GOMutexLocker m_locker(m_mutex, true);
	if (!m_locker.IsLocked())
		return;
	GOrgueProgressDialog dlg;
	if (doc && doc->GetOrganFile())
		res = doc->GetOrganFile()->UpdateCache(&dlg, m_Settings.CompressCache());
	if (!res)
	{
		wxLogError(_("Creating the cache failed"));
		wxMessageBox(_("Creating the cache failed"), _("Error"), wxOK | wxICON_ERROR, this);
	}
}

void GOrgueFrame::OnCacheDelete(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
		doc->GetOrganFile()->DeleteCache();
}

void GOrgueFrame::OnReload(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (!doc || !doc->GetOrganFile())
		return;
	GOrgueOrgan organ = doc->GetOrganFile()->GetOrganInfo();
	if (!DoClose())
		return;
	Open(organ);
}

void GOrgueFrame::OnClose(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (!doc)
		return;
	DoClose();
}

void GOrgueFrame::OnExit(wxCommandEvent& event)
{
	Close();
}

bool GOrgueFrame::Close(bool force)
{
	if (!force && !DoClose())
		return false;
	Destroy();
	return true;
}

void GOrgueFrame::OnCloseWindow(wxCloseEvent& event)
{
	Close();
}

void GOrgueFrame::OnRevert(wxCommandEvent& event)
{
	if (wxMessageBox(_("Any customizations you have saved to this\norgan definition file will be lost!\n\nReset to defaults and reload?"), _("GrandOrgue"), wxYES_NO | wxICON_EXCLAMATION, this) == wxYES)
	{
		GOrgueDocument* doc = GetDocument();
		GOrgueProgressDialog dlg;
		if (doc)
			doc->Revert(&dlg);
	}
}

void GOrgueFrame::OnProperties(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
	{
		GOrgueProperties dlg(doc->GetOrganFile(), this);
		dlg.ShowModal();
	}
}

void GOrgueFrame::OnAudioPanic(wxCommandEvent& WXUNUSED(event))
{
	m_Sound.ResetSound(true);
}

void GOrgueFrame::OnAudioRecord(wxCommandEvent& WXUNUSED(event))
{
	if (m_Sound.IsAudioRecording())
		m_Sound.StopAudioRecording();
	else
	{
		wxFileDialog dlg(this, _("Save as"), m_Settings.AudioRecorderPath(), wxEmptyString, _("WAV files (*.wav)|*.wav"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() == wxID_OK)
		{
			wxFileName filepath = dlg.GetPath();
			if (filepath.GetExt() == wxEmptyString)
				filepath.SetExt(wxT("wav"));
			m_Sound.StartAudioRecording(filepath.GetFullPath());
		}
	}
}

void GOrgueFrame::OnMidiMonitor(wxCommandEvent& WXUNUSED(event))
{
	m_MidiMonitor = !m_MidiMonitor;
}

void GOrgueFrame::OnMidiRecord(wxCommandEvent& WXUNUSED(event))
{
	if (m_Sound.IsMidiRecording())
		m_Sound.StopMidiRecording();
	else
	{
		wxFileDialog dlg(this, _("Save as"), m_Settings.MidiRecorderPath(), wxEmptyString, _("MID files (*.mid)|*.mid"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() == wxID_OK)
		{
			wxFileName filepath = dlg.GetPath();
			if (filepath.GetExt() == wxEmptyString)
				filepath.SetExt(wxT("mid"));
			m_Sound.StartMidiRecording(filepath.GetFullPath());
		}
	}
}

void GOrgueFrame::OnMidiPlay(wxCommandEvent& WXUNUSED(event))
{
	if (m_Sound.IsMidiPlaying())
		m_Sound.StopMidiPlaying();
	else
	{
		wxFileDialog dlg(this, _("Load MIDI"), m_Settings.MidiPlayerPath(), wxEmptyString, _("MID files (*.mid)|*.mid"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (dlg.ShowModal() == wxID_OK)
		{
			wxString filepath = dlg.GetPath();
			m_Sound.StartMidiPlaying(filepath);
		}
	}
}

void GOrgueFrame::OnAudioMemset(wxCommandEvent& WXUNUSED(event))
{
	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
		doc->GetOrganFile()->GetSetter()->ToggleSetter();
}

void GOrgueFrame::OnAudioSettings(wxCommandEvent& WXUNUSED(event))
{
	SettingsDialog dialog(this, m_Sound);
	if (dialog.ShowModal() == wxID_OK)
	{
		GOrgueArchiveManager manager(m_Settings);
		manager.RegisterPackageDirectory(m_Settings.OrganPackagePath());

		m_Sound.ResetSound(true);
		m_Settings.Flush();
	}
}

void GOrgueFrame::OnAudioState(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(m_Sound.getState(), _("Sound output"), wxOK, this);
}

void GOrgueFrame::OnEditOrgan(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
		doc->ShowOrganDialog();
}

void GOrgueFrame::OnMidiList(wxCommandEvent& event)
{
	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
		doc->ShowMidiList();
}

void GOrgueFrame::OnHelp(wxCommandEvent& event)
{
	wxCommandEvent help(wxEVT_SHOWHELP, 0);
	help.SetString(_("User Interface"));
	ProcessEvent(help);
}

void GOrgueFrame::OnShowHelp(wxCommandEvent& event)
{
	m_Help->Display(event.GetString());
}

void GOrgueFrame::OnSettingsVolume(wxCommandEvent& event)
{
	long n = m_Volume->GetValue();

	m_Sound.GetEngine().SetVolume(n);
	m_VolumeLeft->ResetClip();
	m_VolumeRight->ResetClip();
}

void GOrgueFrame::OnSettingsPolyphony(wxCommandEvent& event)
{
	long n = m_Polyphony->GetValue();

	m_Settings.PolyphonyLimit(n);
	m_Sound.GetEngine().SetHardPolyphony(n);
	m_SamplerUsage->ResetClip();
}

void GOrgueFrame::OnSettingsMemoryEnter(wxCommandEvent& event)
{
	long n = m_SetterPosition->GetValue();

	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
		doc->GetOrganFile()->GetSetter()->SetPosition(n);
}

void GOrgueFrame::OnSettingsMemory(wxCommandEvent& event)
{
	long n = m_SetterPosition->GetValue();

	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
		doc->GetOrganFile()->GetSetter()->UpdatePosition(n);
}

void GOrgueFrame::OnSettingsTranspose(wxCommandEvent& event)
{
	long n = m_Transpose->GetValue();

	m_Settings.Transpose(n);
	GOrgueDocument* doc = GetDocument();
	if (doc && doc->GetOrganFile())
		doc->GetOrganFile()->GetSetter()->SetTranspose(n);
}

void GOrgueFrame::OnSettingsReleaseLength(wxCommandEvent& event)
{
	m_Settings.ReleaseLength(m_ReleaseLength->GetSelection() * 50);
	m_Sound.GetEngine().SetReleaseLength(m_Settings.ReleaseLength());
}

void GOrgueFrame::OnHelpAbout(wxCommandEvent& event)
{
	DoSplash(false);
}

void GOrgueFrame::DoSplash(bool timeout)
{
	new GOrgueSplash (timeout, this, wxID_ANY);
}


void GOrgueFrame::OnMenuOpen(wxMenuEvent& event)
{
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

void GOrgueFrame::OnChangeTranspose(wxCommandEvent& event)
{
	m_Transpose->SetValue(event.GetInt());
}

void GOrgueFrame::OnChangeSetter(wxCommandEvent& event)
{
	m_SetterPosition->SetValue(event.GetInt());
}

void GOrgueFrame::OnChangeVolume(wxCommandEvent& event)
{
	m_Volume->SetValue(event.GetInt());
}

void GOrgueFrame::OnKeyCommand(wxKeyEvent& event)
{
	int k = event.GetKeyCode();
	if ( !event.AltDown())
	{
		switch(k)
		{
			case WXK_ESCAPE:
			{
				ProcessCommand(ID_AUDIO_PANIC);
				break;
			}
		}
	}
	event.Skip();
}

void GOrgueFrame::OnMidiEvent(const GOrgueMidiEvent& event)
{
	if (m_MidiMonitor)
	{
		wxLogWarning(_("MIDI event: ") + event.ToString(m_Sound.GetMidi().GetMidiMap()));
	}

	ptr_vector<GOrgueOrgan>& organs = m_Settings.GetOrganList();
	for(unsigned i = 0; i < organs.size(); i++)
		if (organs[i]->Match(event) && organs[i]->IsUsable(m_Settings))
		{
			SendLoadOrgan(*organs[i], true);
			return;
		}
}

void GOrgueFrame::OnSetTitle(wxCommandEvent& event)
{
	m_Label = event.GetString();
	if (m_Label == wxEmptyString)
		SetTitle(m_Title);
	else
		SetTitle(m_Title + _(" - ") + m_Label);
}

void GOrgueFrame::OnMsgBox(wxMsgBoxEvent& event)
{
	wxMessageBox(event.getText(), event.getTitle(), event.getStyle(), this);
}

bool GOrgueFrame::InstallOrganPackage(wxString name)
{
	GOrgueArchiveManager manager(m_Settings);
	wxString result = manager.InstallPackage(name);
	if (result != wxEmptyString)
	{
		GOMessageBox(result, _("Error"), wxOK | wxICON_ERROR, this);
		return false;
	}
	else
		return true;
}

void GOrgueFrame::LoadLastOrgan()
{
	std::vector<GOrgueOrgan*> list = m_Settings.GetLRUOrganList();
	if (list.size() > 0)
		SendLoadOrgan(*list[0]);
}

void GOrgueFrame::SendLoadFile(wxString filename, bool force)
{
	wxFileName name = filename;
	if (name.GetExt() == wxT("orgue"))
	{
		if (InstallOrganPackage(filename))
			LoadLastOrgan();
	}
	else
		SendLoadOrgan(GOrgueOrgan(filename), force);
}

void GOrgueFrame::SendLoadOrgan(const GOrgueOrgan& organ, bool force)
{
	wxCommandEvent evt(wxEVT_LOADFILE, 0);
	evt.SetClientData(new GOrgueOrgan(organ));
	evt.SetInt(force ? 1 : 0);
	GetEventHandler()->AddPendingEvent(evt);
}
