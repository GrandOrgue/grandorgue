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

#ifndef GORGUEPANELVIEW_H
#define GORGUEPANELVIEW_H

#include <wx/wx.h>
#include "GOrgueView.h"

class GOGUIControl;
class GOGUIPanel;
class GOGUIPanelWidget;

class GOrguePanelView : public wxScrolledWindow, public GOrgueView
{
private:
	GOGUIPanelWidget* m_panelwidget;
	GOGUIPanel* m_panel;

public:
	GOrguePanelView(GOrgueDocument* doc, GOGUIPanel* panel, wxWindow* parent, bool main_panel);
	~GOrguePanelView();

	void AddEvent(GOGUIControl* control);

	static GOrguePanelView* createWindow(GOrgueDocument* doc, GOGUIPanel* panel, wxWindow* parent);
	void SyncState();

	bool Destroy();
};

#endif
