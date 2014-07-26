/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "ptrvector.h"
#include "GOrgueBitmap.h"
#include "GOrgueSaveableObject.h"
#include <wx/gdicmn.h>
#include <wx/string.h>

class GOGUIControl;
class GOGUIDisplayMetrics;
class GOGUILayoutEngine;
class GOGUIMouseState;
class GOGUIPanelWidget;
class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueButton;
class GOrgueDC;
class GOrguePanelView;
class GrandOrgueFile;

class GOGUIPanel : private GOrgueSaveableObject
{
protected:
	GrandOrgueFile* m_organfile;
	ptr_vector<GOGUIControl> m_controls;
	std::vector<GOrgueBitmap> m_WoodImages;
	wxString m_Name;
	wxString m_GroupName;
	GOGUIDisplayMetrics* m_metrics;
	GOGUILayoutEngine* m_layout;
	GOrguePanelView* m_view;
	wxRect m_size;
	bool m_InitialOpenWindow;

	void LoadControl(GOGUIControl* control, GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg);

public:
	GOGUIPanel(GrandOrgueFile* organfile);
	virtual ~GOGUIPanel();
	void Init(GOrgueConfigReader& cfg, GOGUIDisplayMetrics* metrics, wxString name, wxString group, wxString group_name=wxT(""));
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Layout();

	void SetView(GOrguePanelView* view);

	GrandOrgueFile* GetOrganFile();
	const wxString& GetName();
	const wxString& GetGroupName();
	void AddEvent(GOGUIControl* control);
	void AddControl(GOGUIControl* control);
	GOGUIDisplayMetrics* GetDisplayMetrics();
	GOGUILayoutEngine* GetLayoutEngine();
	void ControlChanged(void* control);
	void Draw(GOrgueDC& dc);
	void TileWood(GOrgueDC& dc, unsigned which, int sx, int sy, int cx, int cy);
	GOrgueBitmap LoadBitmap(wxString filename, wxString maskname);
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
