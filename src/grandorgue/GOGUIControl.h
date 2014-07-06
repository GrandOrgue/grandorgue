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

#ifndef GOGUICONTROL_H
#define GOGUICONTROL_H

#include "GOrgueSaveableObject.h"
#include <wx/gdicmn.h>
#include <wx/string.h>

class GOGUIDisplayMetrics;
class GOGUIMouseState;
class GOGUIPanel;
class GOrgueConfigReader;
class GOrgueDC;

class GOGUIControl : private GOrgueSaveableObject {
protected:
	GOGUIPanel* m_panel;
	GOGUIDisplayMetrics* m_metrics;
	GOGUIDisplayMetrics* m_layout;
	void* m_control;
	wxRect m_BoundingRect;
	bool m_DrawPending;

	void Save(GOrgueConfigWriter& cfg);

public:
	GOGUIControl(GOGUIPanel* panel, void* control);
	virtual ~GOGUIControl();

	virtual void Init(GOrgueConfigReader& cfg, wxString group);
	virtual void Load(GOrgueConfigReader& cfg, wxString group);

	virtual void ControlChanged(void* control);
	virtual void Draw(GOrgueDC& dc);
	virtual const wxRect& GetBoundingRect();
	virtual bool HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	virtual bool HandleMouseScroll(int x, int y, int amount);
};

#endif
