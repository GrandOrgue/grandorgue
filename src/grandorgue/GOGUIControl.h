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

#ifndef GOGUICONTROL_H
#define GOGUICONTROL_H

#include <wx/wx.h>

class GOGUIPanel;
class GOGUIDisplayMetrics;
class IniFileConfig;

class GOGUIControl {
protected:
	GOGUIPanel* m_panel;
	wxString m_group;
	GOGUIDisplayMetrics* m_metrics;
	void* m_control;
	wxRect m_BoundingRect;

public:
	GOGUIControl(GOGUIPanel* panel, void* control);
	virtual ~GOGUIControl();

	virtual void Load(IniFileConfig& cfg, wxString group);
	virtual void Save(IniFileConfig& cfg, bool prefix);

	virtual void ControlChanged(void* control);
	virtual void Draw(wxDC* dc);
	virtual const wxRect& GetBoundingRect();
	virtual void HandleKey(int key);
	virtual void HandleMousePress(int x, int y, bool right);
	virtual void HandleMouseScroll(int x, int y, int amount);
};

#endif
