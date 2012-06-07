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

#ifndef GOGUICONTROL_H
#define GOGUICONTROL_H

#include <wx/wx.h>

class GOGUIDisplayMetrics;
class GOGUIMouseState;
class GOGUIPanel;
class GOrgueConfigReader;
class GOrgueConfigWriter;

class GOGUIControl {
protected:
	GOGUIPanel* m_panel;
	wxString m_group;
	GOGUIDisplayMetrics* m_metrics;
	void* m_control;
	wxRect m_BoundingRect;
	bool m_DrawPending;

public:
	GOGUIControl(GOGUIPanel* panel, void* control);
	virtual ~GOGUIControl();

	virtual void Load(GOrgueConfigReader& cfg, wxString group);
	virtual void Save(GOrgueConfigWriter& cfg);

	virtual void ControlChanged(void* control);
	virtual void Draw(wxDC* dc);
	virtual const wxRect& GetBoundingRect();
	virtual void HandleKey(int key);
	virtual void HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	virtual void HandleMouseScroll(int x, int y, int amount);
};

#endif
