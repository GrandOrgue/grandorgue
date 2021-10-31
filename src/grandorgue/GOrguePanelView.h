/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPANELVIEW_H
#define GORGUEPANELVIEW_H

#include "GOrgueView.h"
#include <wx/scrolwin.h>
#include <wx/toplevel.h>

class GOGUIControl;
class GOGUIPanel;
class GOGUIPanelWidget;

class GOrguePanelView : public wxScrolledWindow, public GOrgueView
{
private:
	GOGUIPanelWidget* m_panelwidget;
	GOGUIPanel* m_panel;
	wxTopLevelWindow* m_TopWindow; // only if the parent is top level window, else NULL
	wxSize m_Scroll;

	void OnSize(wxSizeEvent& event);

public:
	GOrguePanelView(GOrgueDocumentBase* doc, GOGUIPanel* panel, wxWindow* parent);
	~GOrguePanelView();

	void AddEvent(GOGUIControl* control);

	// creates a PanelView and a wxFrame filled with this PanelView
	static GOrguePanelView* createWithFrame(GOrgueDocumentBase* doc, GOGUIPanel* panel);
	void SyncState();

	void Raise();
	bool Destroy();
	void RemoveView();

	DECLARE_EVENT_TABLE()
};

#endif
