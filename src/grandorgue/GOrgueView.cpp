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

#include "GOrgueEvent.h"
#include "GOrgueView.h"
#include "GOGUIPanel.h"
#include "GOGUIPanelWidget.h"
#include "GOrgueDocument.h"
#include "GrandOrgueFile.h"
#include "Images.h"

GOrgueView::GOrgueView(unsigned panelID) :
	m_panel(NULL),
	m_container(NULL),
	m_frame(NULL),
	m_doc(NULL),
	m_panelID(panelID)
{
}

GOrgueView::~GOrgueView()
{
	if (m_panelID == 0)
	{
		wxCommandEvent event(wxEVT_WINTITLE, 0);
		event.SetString(wxEmptyString);
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
	}
	if (m_container)
	{
		GrandOrgueFile* organfile = m_doc->GetOrganFile();
		organfile->GetPanel(m_panelID)->SetView(NULL);
	}
}

void GOrgueView::OnChangeFilename()
{
	wxView::OnChangeFilename();
	if (m_panelID == 0 && m_doc)
	{
		wxCommandEvent event(wxEVT_WINTITLE, 0);
		event.SetString(m_doc->GetUserReadableName());
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
	}
}

bool GOrgueView::CreateWindow()
{
	if (!m_doc)
		return false;
	GrandOrgueFile* organfile = m_doc->GetOrganFile();
	if (!organfile)
		return false;
	if (organfile->GetPanelCount() <= m_panelID)
		return false;
	if (organfile->GetPanel(m_panelID)->GetView())
		return false;

	if (m_panelID)
	{
		wxIcon icon;
		icon.CopyFromBitmap(GetImage_GOIcon());
		wxDocChildFrame* frame = new wxDocChildFrame(m_doc, this, NULL, -1, organfile->GetPanel(m_panelID)->GetName(), wxDefaultPosition, wxDefaultSize, 
							     wxMINIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE);
		frame->SetIcon(icon);
		m_frame = frame;
		m_container = new wxScrolledWindow(m_frame);
		SetFrame(m_frame);
	}
	else
	{
		m_frame = wxTheApp->GetTopWindow();
		m_container = new wxScrolledWindow(m_frame);
		SetFrame(m_container);
	}
	m_panel = new GOGUIPanelWidget(organfile->GetPanel(m_panelID), m_container);

	/* Calculate scrollbar size */
	m_container->SetSize(50, 50);
	m_container->SetVirtualSize(100, 100);
	m_container->SetScrollRate(5, 5);
	wxSize scroll = m_container->GetSize();
	scroll.DecBy(m_container->GetClientSize());

	wxSize max = m_panel->GetSize();
	max.DecBy(scroll);
	m_container->SetVirtualSize(max);
	m_container->SetSize(m_panel->GetSize());

	wxRect size = organfile->GetPanel(m_panelID)->GetWindowSize();
	m_frame->SetMaxSize(wxSize(wxDefaultCoord, wxDefaultCoord));
	m_frame->SetClientSize(m_panel->GetSize());
	m_frame->SetMaxSize(m_frame->GetSize());
	if (size.GetWidth() && size.GetHeight())
		m_frame->SetSize(size);
	else
		m_frame->Center(wxBOTH);
	m_container->SetPosition(wxPoint(0, 0));
	m_frame->Show();
	m_frame->Update();
	m_container->SetPosition(wxPoint(0, 0));

	organfile->GetPanel(m_panelID)->SetView(this);
	return true;
}

bool GOrgueView::OnCreate(wxDocument *doc, long flags)
{
	m_doc = (GOrgueDocument*)doc;
	CreateWindow();
	return true;

}

void GOrgueView::OnUpdate(wxView *sender, wxObject *hint)
{
	if (m_panel)
		m_panel->OnUpdate();
	else if (!CreateWindow())
		Close(true);
}

void GOrgueView::OnDraw(wxDC*)
{
}

bool GOrgueView::OnClose(bool deleteWindow)
{
	if (GetFrame() && deleteWindow)
	{
		GetFrame()->Destroy();
		m_panel = NULL;
		SetFrame(0);
	}
	Activate(false);
	if (m_panelID >= 1)
		return true;

	return wxView::OnClose(deleteWindow);
}

void GOrgueView::AddEvent(GOGUIControl* control)
{
	wxCommandEvent event(wxEVT_GOCONTROL, 0);
	event.SetClientData(control);
	if (m_panel)
		m_panel->GetEventHandler()->AddPendingEvent(event);
}

GOrgueDocument* GOrgueView::getDocument()
{
	return m_doc;
}
