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
#include "GOrgueDocument.h"
#include "GrandOrgueFile.h"
#include "Images.h"

GOrgueView::GOrgueView() :
	m_doc(NULL)
{
}

GOrgueView::~GOrgueView()
{
	wxCommandEvent event(wxEVT_WINTITLE, 0);
	event.SetString(wxEmptyString);
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
}

void GOrgueView::OnChangeFilename()
{
	wxView::OnChangeFilename();
	if (m_doc)
	{
		wxCommandEvent event(wxEVT_WINTITLE, 0);
		event.SetString(m_doc->GetUserReadableName());
		wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(event);
	}
}

bool GOrgueView::OnCreate(wxDocument *doc, long flags)
{
	m_doc = (GOrgueDocument*)doc;
	return true;

}

void GOrgueView::OnDraw(wxDC*)
{
}

bool GOrgueView::OnClose(bool deleteWindow)
{
	Activate(false);
	return wxView::OnClose(deleteWindow);
}
