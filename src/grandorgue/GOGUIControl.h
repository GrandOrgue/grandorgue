/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUICONTROL_H
#define GOGUICONTROL_H

#include "GOrgueControlChangedHandler.h"
#include "GOrgueSaveableObject.h"
#include <wx/gdicmn.h>
#include <wx/string.h>

class GOGUIDisplayMetrics;
class GOGUILayoutEngine;
class GOGUIMouseState;
class GOGUIPanel;
class GOrgueBitmap;
class GOrgueConfigReader;
class GOrgueDC;

class GOGUIControl : private GOrgueSaveableObject, protected GOrgueControlChangedHandler {
protected:
	GOGUIPanel* m_panel;
	GOGUIDisplayMetrics* m_metrics;
	GOGUILayoutEngine* m_layout;
	void* m_control;
	wxRect m_BoundingRect;
	bool m_DrawPending;

	void Init(GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg);

	void ControlChanged(void* control);

public:
	GOGUIControl(GOGUIPanel* panel, void* control);
	virtual ~GOGUIControl();

	virtual void Load(GOrgueConfigReader& cfg, wxString group);
	virtual void Layout();

	virtual void PrepareDraw(double scale, GOrgueBitmap* background);
	virtual void Draw(GOrgueDC& dc);
	virtual const wxRect& GetBoundingRect();
	virtual bool HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	virtual bool HandleMouseScroll(int x, int y, int amount);
};

#endif
