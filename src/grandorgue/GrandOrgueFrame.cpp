/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include <algorithm>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>
#include <wx/html/helpctrl.h>
#include <wx/splash.h>
#include <wx/spinctrl.h>
#include "wxGaugeAudio.h"
#include "Images.h"
#include "GOGUIPanel.h"
#include "GOrgueEvent.h"
#include "GOrgueMidi.h"
#include "GOrgueProperties.h"
#include "GOrgueSetter.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include "GOrgueTemperament.h"
#include "GrandOrgueID.h"
#include "GrandOrgueFile.h"
#include "OrganDialog.h"
#include "OrganDocument.h"
#include "OrganView.h"
#include "SettingsDialog.h"
#include "SplashScreen.h"

IMPLEMENT_CLASS(GOrgueFrame, wxDocParentFrame)
BEGIN_EVENT_TABLE(GOrgueFrame, wxDocParentFrame)
	EVT_MIDI(GOrgueFrame::OnMidiEvent)
	EVT_KEY_DOWN(GOrgueFrame::OnKeyCommand)
	EVT_COMMAND(0, wxEVT_METERS, GOrgueFrame::OnMeters)
	EVT_COMMAND(0, wxEVT_LOADFILE, GOrgueFrame::OnLoadFile)
	EVT_MENU_OPEN(GOrgueFrame::OnMenuOpen)
	EVT_MENU(wxID_OPEN, GOrgueFrame::OnOpen)
	EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, GOrgueFrame::OnOpen)
	EVT_MENU(ID_FILE_RELOAD, GOrgueFrame::OnReload)
	EVT_MENU(ID_FILE_REVERT, GOrgueFrame::OnRevert)
	EVT_MENU(ID_FILE_PROPERTIES, GOrgueFrame::OnProperties)
	EVT_MENU(ID_FILE_IMPORT_SETTINGS, GOrgueFrame::OnImportSettings)
	EVT_MENU(ID_FILE_IMPORT_COMBINATIONS, GOrgueFrame::OnImportCombinations)
	EVT_MENU(ID_FILE_EXPORT, GOrgueFrame::OnExport)
	EVT_MENU(ID_FILE_CACHE, GOrgueFrame::OnCache)
	EVT_MENU(ID_FILE_CACHE_DELETE, GOrgueFrame::OnCacheDelete)
	EVT_MENU(ID_ORGAN_EDIT, GOrgueFrame::OnEditOrgan)
	EVT_MENU(ID_AUDIO_PANIC, GOrgueFrame::OnAudioPanic)
	EVT_MENU(ID_AUDIO_RECORD, GOrgueFrame::OnAudioRecord)
	EVT_MENU(ID_AUDIO_MEMSET, GOrgueFrame::OnAudioMemset)
	EVT_MENU(ID_AUDIO_SETTINGS, GOrgueFrame::OnAudioSettings)
	EVT_MENU(wxID_HELP, GOrgueFrame::OnHelp)
	EVT_MENU(wxID_ABOUT, GOrgueFrame::OnHelpAbout)
	EVT_COMMAND(0, wxEVT_SHOWHELP, GOrgueFrame::OnShowHelp)
	// New events for Volume, Polyphony, Memory Level, Transpose and Reverb
	EVT_MENU(ID_VOLUME, GOrgueFrame::OnSettingsVolume)
	EVT_MENU(ID_POLYPHONY, GOrgueFrame::OnSettingsPolyphony)
	EVT_MENU(ID_MEMORY, GOrgueFrame::OnSettingsMemory)
	EVT_MENU(ID_TRANSPOSE, GOrgueFrame::OnSettingsTranspose)
	EVT_MENU(ID_REVERB, GOrgueFrame::OnSettingsReverb)

	// End
	EVT_MENU_RANGE(ID_PANEL_FIRST, ID_PANEL_LAST, GOrgueFrame::OnPanel)
	EVT_MENU_RANGE(ID_PRESET_0, ID_PRESET_LAST, GOrgueFrame::OnPreset)
	EVT_MENU_RANGE(ID_TEMPERAMENT_0, ID_TEMPERAMENT_LAST, GOrgueFrame::OnTemperament)
	EVT_SIZE(GOrgueFrame::OnSize)
	EVT_TEXT(ID_METER_TRANSPOSE_SPIN, GOrgueFrame::OnSettingsTranspose)
	EVT_TEXT_ENTER(ID_METER_TRANSPOSE_SPIN, GOrgueFrame::OnSettingsTranspose)
	EVT_CHOICE(ID_REVERB_SELECT, GOrgueFrame::OnSettingsReverb)
	EVT_TEXT(ID_METER_POLY_SPIN, GOrgueFrame::OnSettingsPolyphony)
	EVT_TEXT_ENTER(ID_METER_POLY_SPIN, GOrgueFrame::OnSettingsPolyphony)
	EVT_TEXT(ID_METER_FRAME_SPIN, GOrgueFrame::OnSettingsMemory)
	EVT_TEXT_ENTER(ID_METER_FRAME_SPIN, GOrgueFrame::OnSettingsMemory)
	EVT_COMMAND(ID_METER_FRAME_SPIN, wxEVT_SETVALUE, GOrgueFrame::OnChangeSetter)
	EVT_SLIDER(ID_METER_FRAME_SPIN, GOrgueFrame::OnChangeSetter)
	EVT_TEXT(ID_METER_AUDIO_SPIN, GOrgueFrame::OnSettingsVolume)
	EVT_TEXT_ENTER(ID_METER_AUDIO_SPIN, GOrgueFrame::OnSettingsVolume)
	EVT_COMMAND(ID_METER_AUDIO_SPIN, wxEVT_SETVALUE, GOrgueFrame::OnChangeVolume)

	EVT_UPDATE_UI(wxID_SAVE, GOrgueFrame::OnUpdateLoaded)
	EVT_UPDATE_UI_RANGE(ID_FILE_RELOAD, ID_AUDIO_MEMSET, GOrgueFrame::OnUpdateLoaded)
	EVT_UPDATE_UI_RANGE(ID_PRESET_0, ID_PRESET_LAST, GOrgueFrame::OnUpdateLoaded)
	EVT_UPDATE_UI_RANGE(ID_TEMPERAMENT_0, ID_TEMPERAMENT_LAST, GOrgueFrame::OnUpdateLoaded)
END_EVENT_TABLE()

GOrgueFrame::GOrgueFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type, GOrgueSound& sound) :
	wxDocParentFrame(manager, frame, id, title, pos, size, type),
	m_panel_menu(NULL),
	m_Help(NULL),
	m_SamplerUsage(NULL),
	m_VolumeLeft(NULL),
	m_VolumeRight(NULL),
	m_Transpose(NULL),
	m_Reverb(NULL),
	m_Polyphony(NULL),
	m_SetterPosition(NULL),
	m_Volume(NULL),
	m_Temperaments(),
	m_Sound(sound),
	m_Settings(sound.GetSettings())
{
	wxIcon icon;
	icon.CopyFromBitmap(GetImage_GOIcon());
	SetIcon(icon);
	
	InitHelp();
	
	wxArrayString choices;
	
	wxMenu *file_menu = new wxMenu;
	
	wxMenu *recent_menu = new wxMenu;
	m_docManager->FileHistoryUseMenu(recent_menu);
	m_docManager->FileHistoryLoad(m_Settings.GetConfig());
	
	wxToolBar* tb = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
	tb->SetToolBitmapSize(wxSize(16, 16));
	
	wxMenu *preset_menu = new wxMenu;
	for(unsigned i = ID_PRESET_0; i <= ID_PRESET_LAST; i++)
		preset_menu->Append(i,  wxString::Format(_("Preset %d"), i - ID_PRESET_0), wxEmptyString, wxITEM_CHECK);
	
	file_menu->Append(wxID_OPEN, _("&Open\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
	file_menu->Append(wxID_ANY, _("Open &Recent"), recent_menu);
	file_menu->AppendSeparator();
	file_menu->Append(ID_FILE_PROPERTIES, _("Organ &Properties"), wxEmptyString, wxITEM_NORMAL);
	file_menu->AppendSeparator();
	file_menu->AppendSubMenu(preset_menu, _("Pr&eset"));
	file_menu->AppendSeparator();
	file_menu->Append(wxID_SAVE, _("&Save\tCtrl+S"), wxEmptyString, wxITEM_NORMAL);
	file_menu->Append(ID_FILE_CACHE, _("&Update Cache..."), wxEmptyString, wxITEM_NORMAL);
	file_menu->Append(ID_FILE_CACHE_DELETE, _("Delete &Cache..."), wxEmptyString, wxITEM_NORMAL);
	file_menu->AppendSeparator();
	file_menu->Append(ID_FILE_RELOAD, _("Re&load"), wxEmptyString, wxITEM_NORMAL);
	file_menu->Append(ID_FILE_REVERT, _("Reset to &Defaults"), wxEmptyString, wxITEM_NORMAL);
	file_menu->AppendSeparator();
	file_menu->Append(ID_FILE_IMPORT_SETTINGS, _("&Import Settings"), wxEmptyString, wxITEM_NORMAL);
	file_menu->Append(ID_FILE_IMPORT_COMBINATIONS, _("Import &Combinations"), wxEmptyString, wxITEM_NORMAL);
	file_menu->Append(ID_FILE_EXPORT, _("&Export Settings/Combinations"), wxEmptyString, wxITEM_NORMAL);
	file_menu->AppendSeparator();
	file_menu->Append(wxID_CLOSE, _("&Close"), wxEmptyString, wxITEM_NORMAL);
	file_menu->Append(wxID_EXIT, _("E&xit"), wxEmptyString, wxITEM_NORMAL);
	
	m_Temperaments = GOrgueTemperament::GetTemperaments();
	wxMenu *temperament_menu = new wxMenu;
	for(unsigned i = 0; i < m_Temperaments.size(); i++)
	{
		wxMenu *menu;
		wxString group = m_Temperaments[i]->GetGroup();
		if (group == wxEmptyString)
			menu = temperament_menu;
		else
		{
			menu = NULL;
			for(unsigned j = 0; j < temperament_menu->GetMenuItemCount(); j++)
			{
				wxMenuItem* it = temperament_menu->FindItemByPosition(j);
				if (it->GetItemLabel() == group && it->GetSubMenu())
					menu = it->GetSubMenu();
			}
			if (!menu)
			{
				menu = new wxMenu();
				temperament_menu->AppendSubMenu(menu, group);
			}
		}
		menu->Append(ID_TEMPERAMENT_0 + i, wxGetTranslation(m_Temperaments[i]->GetName()), wxEmptyString, wxITEM_CHECK);
	}
	
	wxMenu *audio_menu = new wxMenu;
	audio_menu->AppendSubMenu(temperament_menu, _("&Temperament"));
	audio_menu->Append(ID_ORGAN_EDIT, _("&Organ settings"), wxEmptyString, wxITEM_NORMAL);
	audio_menu->AppendSeparator();
	audio_menu->Append(ID_AUDIO_SETTINGS, _("Audio/Midi &Settings"), wxEmptyString, wxITEM_NORMAL);
	audio_menu->AppendSeparator();
	audio_menu->Append(ID_AUDIO_RECORD, _("&Record\tCtrl+R"), wxEmptyString, wxITEM_CHECK);
	audio_menu->Append(ID_AUDIO_PANIC, _("&Panic\tEscape"), wxEmptyString, wxITEM_NORMAL);
	audio_menu->Append(ID_AUDIO_MEMSET, _("&Memory Set\tShift"), wxEmptyString, wxITEM_CHECK);
	
	
	wxMenu *help_menu = new wxMenu;
	help_menu->Append(wxID_HELP, _("&Help\tF1"), wxEmptyString, wxITEM_NORMAL);
	help_menu->Append(wxID_ABOUT, _("&About"), wxEmptyString, wxITEM_NORMAL);
	
	tb->AddTool(ID_AUDIO_MEMSET, _("&Memory Set\tShift"), GetImage_set(), _("Memory Set"), wxITEM_CHECK);
	tb->AddTool(ID_MEMORY, _("&Memory Level"), GetImage_memory(), _("Memory Level"), wxITEM_NORMAL);
	m_SetterPosition = new wxSpinCtrl(tb, ID_METER_FRAME_SPIN, wxEmptyString, wxDefaultPosition, wxSize(46, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 999);
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
	m_Volume->SetValue(m_Settings.GetVolume());
	
	tb->AddTool(ID_REVERB, _("&Reverb"), GetImage_reverb(), _("Reverb"), wxITEM_NORMAL);
	choices.clear();
	choices.push_back(_("Max"));
	choices.push_back(_("2.8 s"));
	choices.push_back(_("1.4 s"));
	choices.push_back(_("700 ms"));
	choices.push_back(_("350 ms"));
	choices.push_back(_("175 ms"));
	m_Reverb = new wxChoice(tb, ID_REVERB_SELECT, wxDefaultPosition, wxDefaultSize, choices);
	tb->AddControl(m_Reverb);
	int n = m_Settings.GetReverb();
	m_Reverb->SetSelection(n);
		if (n > 0)
		n = -17 + n;
	m_Sound.GetEngine().SetReverb(n);
	
	tb->AddTool(ID_TRANSPOSE, _("&Transpose"), GetImage_transpose(), _("Transpose"), wxITEM_NORMAL);
	m_Transpose = new wxSpinCtrl(tb, ID_METER_TRANSPOSE_SPIN, wxEmptyString, wxDefaultPosition, wxSize(46, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 11);
	tb->AddControl(m_Transpose);
	m_Transpose->SetValue(0);
	
	tb->AddTool(ID_POLYPHONY, _("&Polyphony"), GetImage_polyphony(), _("Polyphony"), wxITEM_NORMAL);
	m_Polyphony = new wxSpinCtrl(tb, ID_METER_POLY_SPIN, wxEmptyString, wxDefaultPosition, wxSize(56, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 4096);
	tb->AddControl(m_Polyphony);
	
	m_SamplerUsage = new wxGaugeAudio(tb, wxID_ANY, wxDefaultPosition);
	tb->AddControl(m_SamplerUsage);
	m_Polyphony->SetValue(m_Settings.GetPolyphonyLimit());
	
	tb->AddTool(ID_AUDIO_PANIC, _("&Panic\tEscape"), GetImage_panic(), _("Panic"), wxITEM_NORMAL);
	
	
	m_panel_menu = new wxMenu();
	
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, _("&File"));
	menu_bar->Append(audio_menu, _("&Audio/Midi"));
	menu_bar->Append(m_panel_menu, _("&Panel"));
	menu_bar->Append(help_menu, _("&Help"));
	SetMenuBar(menu_bar);
	tb->Realize();
	
	SetClientSize(880, 495);	// default minimal size
	Center(wxBOTH);
	SetAutoLayout(true);
}

GOrgueFrame::~GOrgueFrame()
{
	if (m_Help)
		delete m_Help;
}

void GOrgueFrame::Init()
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
}

void GOrgueFrame::InitHelp()
{
	m_Help = new wxHtmlHelpController(wxHF_CONTENTS | wxHF_INDEX | wxHF_SEARCH | wxHF_ICONS_BOOK | wxHF_FLAT_TOOLBAR);

	wxString result;
	wxString lang = wxGetLocale()->GetCanonicalName();

	wxString searchpath;
	searchpath.Append(wxStandardPaths::Get().GetResourcesDir() + wxFILE_SEP_PATH + wxT("help"));

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

void GOrgueFrame::OnPanel(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	GrandOrgueFile* organfile = doc ? doc->GetOrganFile() : NULL;
	unsigned no = event.GetId() - ID_PANEL_FIRST + 1;
	if (!organfile || organfile->GetPanelCount() <= no)
		return;
	wxWindow* win = organfile->GetPanel(no)->GetParentWindow();
	if (win)
	{
		win->Raise();
		win->SetFocus();
	}
	else
	{
		OrganView* view = new OrganView(no);
		view->SetDocument(doc);
		view->OnCreate(doc, 0);
	}
}

void GOrgueFrame::UpdatePanelMenu()
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	GrandOrgueFile* organfile = doc ? doc->GetOrganFile() : NULL;
	unsigned panelcount = std::min (organfile ? organfile->GetPanelCount() - 1 : 0, (unsigned)(ID_PANEL_LAST - ID_PANEL_FIRST));

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
		item->Check(panel->GetParentWindow() ? true : false);
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
	GrandOrgueFile* organfile = NULL;
	if (m_docManager->GetCurrentDocument())
		organfile = ((OrganDocument*)m_docManager->GetCurrentDocument())->GetOrganFile();

	if (ID_PRESET_0 <= event.GetId() && event.GetId() <= ID_PRESET_LAST)
	{
		event.Check(m_Settings.GetPreset() == (unsigned)(event.GetId() - ID_PRESET_0));
		return;
	}

	if (ID_TEMPERAMENT_0 <= event.GetId() && event.GetId() <= ID_TEMPERAMENT_LAST)
	{
		event.Enable(organfile);
		event.Check(organfile && m_Temperaments[event.GetId() - ID_TEMPERAMENT_0]->GetName() == organfile->GetTemperament());
		return;
	}

	if (event.GetId() == ID_AUDIO_RECORD)
		event.Check(m_Sound.IsRecording());
	else if (event.GetId() == ID_AUDIO_MEMSET)
		event.Check(organfile && organfile->GetSetter() && organfile->GetSetter()->IsSetterActive());

	if (event.GetId() == ID_FILE_CACHE_DELETE)
		event.Enable(organfile && organfile->CachePresent());
	else
		event.Enable(organfile && (event.GetId() == ID_FILE_REVERT ? organfile->IsCustomized() : true));
}

void GOrgueFrame::OnPreset(wxCommandEvent& event)
{
	unsigned id = event.GetId() - ID_PRESET_0;
	if (id == m_Settings.GetPreset())
		return;
	m_Settings.SetPreset(id);
	if (m_docManager->GetCurrentDocument())
		ProcessCommand(wxID_FILE1);
}

void GOrgueFrame::OnTemperament(wxCommandEvent& event)
{
	unsigned id = event.GetId() - ID_TEMPERAMENT_0;
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;

	doc->GetOrganFile()->SetTemperament(m_Temperaments[id]->GetName());
}

void GOrgueFrame::OnLoadFile(wxCommandEvent& event)
{
	m_docManager->CreateDocument(event.GetString(), wxDOC_SILENT);
}

void GOrgueFrame::OnOpen(wxCommandEvent& event)
{
	if (event.GetId() == wxID_OPEN)
	{
		GetDocumentManager()->SetLastDirectory(m_Settings.GetOrganPath());
		ProcessCommand(wxID_OPEN);
		if (m_docManager->GetCurrentDocument() && ((OrganDocument*)m_docManager->GetCurrentDocument())->GetOrganFile())
		{
			m_Settings.SetOrganPath(GetDocumentManager()->GetLastDirectory());
		}
	}
	else
		event.Skip();
}

void GOrgueFrame::OnImportSettings(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;

	wxFileDialog dlg(this, _("Import Settings"), m_Settings.GetSettingPath(), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_Settings.SetSettingPath(dlg.GetDirectory());
		wxString file = doc->GetOrganFile()->GetODFFilename();
		doc->DoOpenDocument(file, dlg.GetPath());
	}
}

void GOrgueFrame::OnImportCombinations(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;

	wxFileDialog dlg(this, _("Import Combinations"), m_Settings.GetSettingPath(), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_Settings.SetSettingPath(dlg.GetDirectory());
		doc->GetOrganFile()->LoadCombination(dlg.GetPath());
		doc->Modify(true);
	}
}

void GOrgueFrame::OnExport(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;

	wxFileDialog dlg(this, _("Export Settings"), m_Settings.GetSettingPath(), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_Settings.SetSettingPath(dlg.GetDirectory());
		doc->DoSaveDocument(dlg.GetPath());
		doc->Modify(false);
	}
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
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;
	if (!doc->GetOrganFile()->UpdateCache(m_Settings.GetCompressCache()))
	{
		wxLogError(_("Creating the cache failed"));
		wxMessageBox(_("Creating the cache failed"), _("Error"), wxOK | wxICON_ERROR, NULL);
	}
}

void GOrgueFrame::OnCacheDelete(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;
	doc->GetOrganFile()->DeleteCache();
}

void GOrgueFrame::OnReload(wxCommandEvent& event)
{
	ProcessCommand(wxID_FILE1);
}

void GOrgueFrame::OnRevert(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (doc && doc->GetOrganFile() && ::wxMessageBox(_("Any customizations you have saved to this\norgan definition file will be lost!\n\nReset to defaults and reload?"), wxT(APP_NAME), wxYES_NO | wxICON_EXCLAMATION, this) == wxYES)
	{
		{
			wxFileConfig cfg(wxEmptyString, wxEmptyString, doc->GetOrganFile()->GetODFFilename(), wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS, wxCSConv(wxT("ISO-8859-1")));
			m_docManager->GetCurrentDocument()->Modify(false);
			doc->GetOrganFile()->Revert(cfg);
			doc->GetOrganFile()->DeleteSettings();
		}
		ProcessCommand(wxID_FILE1);
	}
}

void GOrgueFrame::OnProperties(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	GOrgueProperties dlg(doc->GetOrganFile(), this);
	dlg.ShowModal();
}

void GOrgueFrame::OnAudioPanic(wxCommandEvent& WXUNUSED(event))
{
	m_Sound.ResetSound(true);
}

void GOrgueFrame::OnAudioRecord(wxCommandEvent& WXUNUSED(event))
{
	if (m_Sound.IsRecording())
		m_Sound.StopRecording();
	else
	{
		wxFileDialog dlg(this, _("Save as"), m_Settings.GetWAVPath(), wxEmptyString, _("WAV files (*.wav)|*.wav"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() == wxID_OK)
			{
				m_Settings.SetWAVPath(dlg.GetDirectory());
				wxString filepath = dlg.GetPath();
				if (filepath.Find(wxT(".wav")) == wxNOT_FOUND)
				{
					filepath.append(wxT(".wav"));
				}
				m_Sound.StartRecording(filepath);
			}
	}
}

void GOrgueFrame::OnAudioMemset(wxCommandEvent& WXUNUSED(event))
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;
	doc->GetOrganFile()->GetSetter()->ToggleSetter();
}

void GOrgueFrame::OnAudioSettings(wxCommandEvent& WXUNUSED(event))
{
	wxLogDebug(_("settingsdialog.."));
	SettingsDialog dialog(this, m_Sound);
	wxLogDebug(_("success"));
	dialog.ShowModal();
}

void GOrgueFrame::OnEditOrgan(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;
	OrganDialog dialog(this, doc->GetOrganFile());
	dialog.ShowModal();
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

	m_Settings.SetVolume(n);
	m_Sound.GetEngine().SetVolume(n);
	m_VolumeLeft->ResetClip();
	m_VolumeRight->ResetClip();
}

void GOrgueFrame::OnSettingsPolyphony(wxCommandEvent& event)
{
	long n = m_Polyphony->GetValue();

	m_Settings.SetPolyphonyLimit(n);
	m_Sound.GetEngine().SetHardPolyphony(n);
	m_SamplerUsage->ResetClip();
}

void GOrgueFrame::OnSettingsMemory(wxCommandEvent& event)
{
	long n = m_SetterPosition->GetValue();

	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (doc && doc->GetOrganFile())
		doc->GetOrganFile()->GetSetter()->SetPosition(n);
}

void GOrgueFrame::OnSettingsTranspose(wxCommandEvent& event)
{
	long n = m_Transpose->GetValue();

	m_Settings.SetTranspose(n);
	m_Sound.ResetSound();
}

void GOrgueFrame::OnSettingsReverb(wxCommandEvent& event)
{
	int n = m_Reverb->GetSelection();
	m_Settings.SetReverb(n);
	if (n > 0)
		n = -17 + n;
	m_Sound.GetEngine().SetReverb(n);
}

void GOrgueFrame::OnHelpAbout(wxCommandEvent& event)
{
	DoSplash(false);
}

void GOrgueFrame::DoSplash(bool timeout)
{
	new GOrgueSplash
		(timeout
		,this
		,wxID_ANY
		);
}


void GOrgueFrame::OnMenuOpen(wxMenuEvent& event)
{
    DoMenuUpdates(event.GetMenu());
    if (event.GetMenu() == m_panel_menu)
		UpdatePanelMenu();
    event.Skip();
}

void GOrgueFrame::OnChangeSetter(wxCommandEvent& event)
{
	m_SetterPosition->SetValue(event.GetInt());
}

void GOrgueFrame::OnChangeVolume(wxCommandEvent& event)
{
	m_Settings.SetVolume(event.GetInt());
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

void GOrgueFrame::OnMidiEvent(GOrgueMidiEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (doc)
		doc->OnMidiEvent(event);

	int j = event.GetEventCode();
	if (j == -1)
		return;
	// MIDI for different organ??
	std::map<long, wxString>::const_iterator it = m_Settings.GetOrganList().find(j);
	if (it != m_Settings.GetOrganList().end())
	{
		wxCommandEvent evt(wxEVT_LOADFILE, 0);
		evt.SetString(it->second);
		GetEventHandler()->AddPendingEvent(evt);
	}
}
