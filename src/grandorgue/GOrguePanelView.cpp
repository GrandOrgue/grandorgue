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

#include "GOrguePanelView.h"

#include "GOGUIPanel.h"
#include "GOGUIPanelWidget.h"
#include "Images.h"
#include <wx/frame.h>
#include <wx/icon.h>
#include <wx/image.h>

BEGIN_EVENT_TABLE(GOrguePanelView, wxScrolledWindow)
	EVT_SIZE(GOrguePanelView::OnSize)
END_EVENT_TABLE()

GOrguePanelView* GOrguePanelView::createWindow(GOrgueDocument* doc, GOGUIPanel* panel, wxWindow* parent)
{
	bool is_main = (parent != NULL);
	if (!parent)
	{
		wxFrame* frame = new wxFrame(NULL, -1, panel->GetName(), wxDefaultPosition, wxDefaultSize, 
					     wxMINIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);
		wxIcon icon;
		icon.CopyFromBitmap(GetImage_GOIcon());
		frame->SetIcon(icon);
		parent = frame;
	}
	return new GOrguePanelView(doc, panel, parent, is_main);
}


GOrguePanelView::GOrguePanelView(GOrgueDocument* doc, GOGUIPanel* panel, wxWindow* parent, bool main_panel) :
	wxScrolledWindow(parent),
	GOrgueView(doc, main_panel ? this : parent),
	m_panelwidget(NULL),
	m_panel(panel)
{
	wxWindow* frame = parent;
	GOGUIPanelWidget* panelwidget = new GOGUIPanelWidget(panel, this);

	/* Calculate scrollbar size */
	this->SetSize(50, 50);
	this->SetVirtualSize(100, 100);
	this->SetScrollRate(5, 5);
	m_Scroll = this->GetSize();
	m_Scroll.DecBy(this->GetClientSize());

	wxSize max = panelwidget->GetSize();
	max.DecBy(m_Scroll);
	this->SetVirtualSize(max);
	this->SetSize(panelwidget->GetSize());

	wxRect size = panel->GetWindowSize();
	frame->SetMaxSize(wxSize(wxDefaultCoord, wxDefaultCoord));
	frame->SetClientSize(panelwidget->GetSize());
	frame->SetMaxSize(frame->GetSize());
	if (size.GetWidth() && size.GetHeight())
		frame->SetSize(size);
	else
		frame->Center(wxBOTH);
	this->SetPosition(wxPoint(0, 0));
	m_panelwidget = panelwidget;
	frame->Show();
	frame->Update();
	this->SetPosition(wxPoint(0, 0));

	m_panel->SetView(this);
}

GOrguePanelView::~GOrguePanelView()
{
}

void GOrguePanelView::AddEvent(GOGUIControl* control)
{
	wxCommandEvent event(wxEVT_GOCONTROL, 0);
	event.SetClientData(control);
	m_panelwidget->GetEventHandler()->AddPendingEvent(event);
}

void GOrguePanelView::SyncState()
{
	m_panel->SetWindowSize(GetParent()->GetRect());
	m_panel->SetInitialOpenWindow(true);
}

bool GOrguePanelView::Destroy()
{
	m_panel->SetView(NULL);
	return wxScrolledWindow::Destroy();
}

void GOrguePanelView::OnSize(wxSizeEvent& event)
{
	if (m_panelwidget)
	{
		wxSize max = event.GetSize();
		max = m_panelwidget->UpdateSize(max);
		max.DecBy(m_Scroll);
		this->SetVirtualSize(max);

		int x, xu, y, yu;
		GetScrollPixelsPerUnit(&xu, &yu);
		GetViewStart(&x, &y);
		if (x * xu + max.GetWidth() > event.GetSize().GetWidth())
			x = (event.GetSize().GetWidth() - max.GetWidth()) / xu;
		if (y * yu + max.GetHeight() > event.GetSize().GetHeight())
			y = (event.GetSize().GetHeight() - max.GetHeight()) / yu;
		this->Scroll(x, y);
	}
	event.Skip();
}
