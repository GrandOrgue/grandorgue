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

#ifndef GOGUIPANEL_H
#define GOGUIPANEL_H

#include <wx/wx.h>
#include "ptrvector.h"

class GOGUIControl;
class GOGUIDisplayMetrics;
class GOGUIMouseState;
class GOGUIPanelWidget;
class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueButton;
class GOrguePanelView;
class GrandOrgueFile;

class GOGUIPanel {
protected:
	GrandOrgueFile* m_organfile;
	ptr_vector<GOGUIControl> m_controls;
	std::vector<wxBitmap*> m_WoodImages;
	wxString m_Name;
	wxString m_GroupName;
	GOGUIDisplayMetrics* m_metrics;
	GOrguePanelView* m_view;
	wxString m_group;
	wxRect m_size;
	bool m_InitialOpenWindow;

public:
	GOGUIPanel(GrandOrgueFile* organfile);
	virtual ~GOGUIPanel();
	void Init(GOrgueConfigReader& cfg, GOGUIDisplayMetrics* metrics, wxString name, wxString group, wxString group_name=wxT(""));
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg);

	void SetView(GOrguePanelView* view);

	GrandOrgueFile* GetOrganFile();
	const wxString& GetName();
	const wxString& GetGroupName();
	void AddEvent(GOGUIControl* control);
	void AddControl(GOGUIControl* control);
	GOGUIDisplayMetrics* GetDisplayMetrics();
	void ControlChanged(void* control);
	void Draw(wxDC* dc);
	wxString WrapText(wxDC* dc, const wxString& string, int width);
	void TileBitmap(wxDC* dc, wxBitmap* bitmap, wxRect target, int xo, int yo);
	void TileWood(wxDC* dc, int which, int sx, int sy, int cx, int cy);
	wxBitmap* GetWoodImage(unsigned index);
	wxBitmap* LoadBitmap(wxString filename, wxString maskname);
	void HandleKey(int key);
	void HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	void HandleMouseScroll(int x, int y, int amount);
	void Modified();
	
	unsigned GetWidth();
	unsigned GetHeight();
	bool InitialOpenWindow();

	wxRect GetWindowSize();
	void SetWindowSize(wxRect rect);
	void SetInitialOpenWindow(bool open);
};


#endif
