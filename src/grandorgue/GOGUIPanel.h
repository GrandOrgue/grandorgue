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

#ifndef GOGUIPANEL_H
#define GOGUIPANEL_H

#include <wx/wx.h>
#include "ptrvector.h"

class GOrgueControl;
class GOGUIDisplayMetrics;
class GrandOrgueFile;
class GOGUIControl;
class GOGUIPanelWidget;
class IniFileConfig;
class GOGUIMouseState;

class GOGUIPanel {
protected:
	GrandOrgueFile* m_organfile;
	ptr_vector<GOGUIControl> m_controls;
	ptr_vector<wxBitmap> m_images;
	ptr_vector<wxBitmap> m_WoodImages;
	wxString m_Name;
	wxString m_GroupName;
	GOGUIDisplayMetrics* m_metrics;
	GOGUIPanelWidget* m_window;
	wxWindow* m_parent;
	wxString m_group;
	wxRect m_size;
	bool m_InitialOpenWindow;

public:
	GOGUIPanel(GrandOrgueFile* organfile);
	virtual ~GOGUIPanel();
	void Init(IniFileConfig& cfg, GOGUIDisplayMetrics* metrics, wxString name, wxString group, wxString group_name=wxT(""));
	void Load(IniFileConfig& cfg, wxString group);
	void Save(IniFileConfig& cfg, bool prefix);

	GOGUIPanelWidget* GetWindow();
	void SetWindow(GOGUIPanelWidget* window);

	GrandOrgueFile* GetOrganFile();
	const wxString& GetName();
	const wxString& GetGroupName();
	void AddEvent(GOGUIControl* control);
	void AddControl(GOGUIControl* control);
	GOGUIDisplayMetrics* GetDisplayMetrics();
	void ControlChanged(void* control);
	void Draw(wxDC* dc);
	wxString WrapText(wxDC* dc, const wxString& string, int width);
	void TileWood(wxDC* dc, int which, int sx, int sy, int cx, int cy);
	wxBitmap* GetImage(unsigned index);
	wxBitmap* GetWoodImage(unsigned index);
	void HandleKey(int key);
	void HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	void HandleMouseScroll(int x, int y, int amount);
	void Modified();
	
	unsigned GetWidth();
	unsigned GetHeight();
	bool InitialOpenWindow();

	wxWindow* GetParentWindow();
	void SetParentWindow(wxWindow* win);
	wxRect GetWindowSize();
};


#endif
