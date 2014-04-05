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

#include "GOrgueReferencePipe.h"

#include "GOrgueConfigReader.h"
#include "GOrgueManual.h"
#include "GOrgueRank.h"
#include "GOrgueStop.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>
#include <wx/tokenzr.h>

GOrgueReferencePipe::GOrgueReferencePipe(GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number) :
	GOrguePipe(organfile, rank, midi_key_number),
	m_Reference(NULL),
	m_ReferenceID(0),
	m_Filename()
{
}

void GOrgueReferencePipe::Load(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterCacheObject(this);
	m_Filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
	if (!m_Filename.StartsWith(wxT("REF:")))
		throw (wxString)_("ReferencePipe without Reference");
}

void GOrgueReferencePipe::Initialize()
{
	if (!m_Filename.StartsWith(wxT("REF:")))
		throw (wxString)_("ReferencePipe without Reference");

	unsigned long manual, stop, pipe;
	wxArrayString strs = wxStringTokenize(m_Filename.Mid(4), wxT(":"), wxTOKEN_RET_EMPTY_ALL);
	if (strs.GetCount() != 3 ||
	    !strs[0].ToULong(&manual) ||
	    !strs[1].ToULong(&stop) ||
	    !strs[2].ToULong(&pipe))
		throw (wxString)_("Invalid reference ") + m_Filename;
	if ((manual < m_organfile->GetFirstManualIndex()) || (manual >= m_organfile->GetODFManualCount()) ||
	    (stop <= 0) || (stop > m_organfile->GetManual(manual)->GetStopCount()) ||
	    (pipe <= 0) || (pipe > m_organfile->GetManual(manual)->GetStop(stop-1)->GetRank(0)->GetPipeCount()))
		throw (wxString)_("Invalid reference ") + m_Filename;
	m_Reference = m_organfile->GetManual(manual)->GetStop(stop-1)->GetRank(0)->GetPipe(pipe-1);
	m_ReferenceID = m_Reference->RegisterReference(this);
}

bool GOrgueReferencePipe::LoadCache(GOrgueCache& cache)
{
	return true;
}

bool GOrgueReferencePipe::SaveCache(GOrgueCacheWriter& cache)
{
	return true;
}

void GOrgueReferencePipe::UpdateHash(SHA_CTX& ctx)
{
}

void GOrgueReferencePipe::LoadData()
{
}

const wxString& GOrgueReferencePipe::GetLoadTitle()
{
	return m_Filename;
}

void GOrgueReferencePipe::Change(unsigned velocity, unsigned old_velocity)
{
	m_Reference->Set(velocity, m_ReferenceID);
}
