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

#include "OrganDocument.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrgueSound.h"
#include "GOrgueMeter.h"
#include "GrandOrgueFile.h"

IMPLEMENT_DYNAMIC_CLASS(OrganDocument, wxDocument)

extern GOrgueSound* g_sound;

OrganDocument::OrganDocument() :
	m_organfile(NULL)
{
}

OrganDocument::~OrganDocument()
{
	CloseOrgan();
}

bool OrganDocument::OnCloseDocument()
{
    return wxDocument::OnCloseDocument();
}

bool OrganDocument::DoOpenDocument(const wxString& file)
{
	return DoOpenDocument(file, wxEmptyString);
}

bool OrganDocument::DoOpenDocument(const wxString& file, const wxString& file2)
{
	wxBusyCursor busy;

	CloseOrgan();
	bool open_sound = g_sound->OpenSound();

	/* abort if we failed to open the sound device */
	if (!open_sound)
		return false;

	m_organfile = new GrandOrgueFile(this, g_sound->IsStereo());
	wxString error = m_organfile->Load(file, file2);
	if (!error.IsEmpty())
	{
		if (error != wxT("!"))
		{
			wxLogError(wxT("%s\n"),error.c_str());
			wxMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
		}
		CloseOrgan();
		if (!file2.IsEmpty())
			DeleteAllViews();
		return false;
	}
	g_sound->PreparePlayback(m_organfile);
	if (m_organfile->GetVolume() != -1)
	{
		::wxGetApp().frame->m_meters[0]->SetValue(m_organfile->GetVolume());
		g_sound->GetEngine().SetVolume(m_organfile->GetVolume());
	}

	SetTitle(m_organfile->GetChurchName());

	/* we have loaded the organ so we can now enable playback */
	g_sound->ActivatePlayback();

	UpdateAllViews();

	return true;
}

bool OrganDocument::DoSaveDocument(const wxString& file)
{
	m_organfile->SetVolume(g_sound->GetEngine().GetVolume());
	m_organfile->Save(file);
	return true;
}

void OrganDocument::CloseOrgan()
{
	g_sound->CloseSound();
	if (m_organfile)
	{
		delete m_organfile;
		m_organfile = 0;
	}
}

GrandOrgueFile* OrganDocument::GetOrganFile()
{
	return m_organfile;
}
