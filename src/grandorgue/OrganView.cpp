/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "OrganView.h"
#include "GOGUIPanel.h"
#include "GOGUIPanelWidget.h"
#include "OrganDocument.h"
#include "GrandOrgueFile.h"
#include "Images.h"

IMPLEMENT_DYNAMIC_CLASS(OrganView, wxView)


OrganView::OrganView(unsigned panelID) :
	m_panel(NULL),
	m_container(NULL),
	m_frame(NULL),
	m_doc(NULL),
	m_panelID(panelID)
{
}

OrganView::~OrganView()
{
	if (m_container)
	{
		GrandOrgueFile* organfile = m_doc->GetOrganFile();
		organfile->GetPanel(m_panelID)->SetParentWindow(NULL);
	}
}

bool OrganView::CreateWindow()
{
	if (!m_doc)
		return false;
	GrandOrgueFile* organfile = m_doc->GetOrganFile();
	if (!organfile)
		return false;
	if (organfile->GetPanelCount() <= m_panelID)
		return false;
	if (organfile->GetPanel(m_panelID)->GetWindow())
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
	m_panel = new GOGUIPanelWidget(organfile->GetPanel(m_panelID), m_container, this);

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
	m_frame->Show();

	organfile->GetPanel(m_panelID)->SetParentWindow(GetFrame());
	return true;
}

bool OrganView::OnCreate(wxDocument *doc, long flags)
{
	m_doc = (OrganDocument*)doc;
	CreateWindow();
	return true;

}

void OrganView::OnUpdate(wxView *sender, wxObject *hint)
{
	if (m_panel)
		m_panel->OnUpdate();
	else if (!CreateWindow())
		Close(true);
}

void OrganView::OnDraw(wxDC*)
{
}

bool OrganView::OnClose(bool deleteWindow)
{
	if (GetFrame() && deleteWindow)
	{
		GetFrame()->Destroy();
		SetFrame(0);
	}
	Activate(false);
	if (m_panelID >= 1)
		return true;

	return wxView::OnClose(deleteWindow);
}

void OrganView::OnWindowClosed()
{
	m_panel = NULL;
	SetFrame(0);
}

