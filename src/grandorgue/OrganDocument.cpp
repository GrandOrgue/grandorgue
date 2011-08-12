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
#include "wxGaugeAudio.h"
#include "GOrgueMeter.h"
#include "GrandOrgueFile.h"

IMPLEMENT_DYNAMIC_CLASS(OrganDocument, wxDocument)

extern GOrgueSound* g_sound;
extern GrandOrgueFile* organfile;

OrganDocument::~OrganDocument(void)
{
	CloseOrgan();
	::wxGetApp().frame->SetTitle(wxT(APP_NAME));
}

bool OrganDocument::OnCloseDocument()
{
    if (!b_loaded)
        return false;
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
	bool open_sound = g_sound->OpenSound(false, NULL);

	/* abort if we failed to open the sound device */
	if (!open_sound)
		return false;

	organfile = new GrandOrgueFile;
	wxString error = organfile->Load(file, file2);
	if (!error.IsEmpty())
	{
		CloseOrgan();
		if (!file2.IsEmpty())
			DeleteAllViews();
		 if (error != wxT("!"))
		 {
			::wxLogError(wxT("%s\n"),error.c_str());
			wxMessageBox(error, _("Load error"), wxOK | wxICON_ERROR, NULL);
		}
		b_loaded = true;
		return false;
	}
	organfile->PreparePlayback();

	SetTitle(organfile->GetChurchName());

	Modify(false);
	UpdateAllViews(0, this);

	/* we have loaded the organ so we can now enable playback */
	g_sound->ActivatePlayback();
	b_loaded = true;

	return true;
}

bool OrganDocument::DoSaveDocument(const wxString& file)
{
	organfile->Save(file);
	return true;
}

void OrganDocument::CloseOrgan()
{
	g_sound->CloseSound(organfile);
	if (organfile)
	{
		delete organfile;
		organfile = 0;
	}
	::wxGetApp().frame->m_meters[0]->m_meters[0]->ResetClip();
	::wxGetApp().frame->m_meters[0]->m_meters[1]->ResetClip();
	::wxGetApp().frame->m_meters[1]->m_meters[0]->ResetClip();
}
