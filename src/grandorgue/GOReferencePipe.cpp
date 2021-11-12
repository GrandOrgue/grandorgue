/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOReferencePipe.h"

#include "config/GOConfigReader.h"
#include "GOManual.h"
#include "GORank.h"
#include "GOStop.h"
#include "GODefinitionFile.h"
#include <wx/intl.h>
#include <wx/tokenzr.h>

GOReferencePipe::GOReferencePipe(GODefinitionFile* organfile, GORank* rank, unsigned midi_key_number) :
	GOPipe(organfile, rank, midi_key_number),
	m_Reference(NULL),
	m_ReferenceID(0),
	m_Filename()
{
}

void GOReferencePipe::Load(GOConfigReader& cfg, wxString group, wxString prefix)
{
	m_organfile->RegisterCacheObject(this);
	m_Filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
	if (!m_Filename.StartsWith(wxT("REF:")))
		throw (wxString)_("ReferencePipe without Reference");
}

void GOReferencePipe::Initialize()
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

bool GOReferencePipe::LoadCache(GOCache& cache)
{
	return true;
}

bool GOReferencePipe::SaveCache(GOCacheWriter& cache)
{
	return true;
}

void GOReferencePipe::UpdateHash(GOHash& hash)
{
}

void GOReferencePipe::LoadData()
{
}

const wxString& GOReferencePipe::GetLoadTitle()
{
	return m_Filename;
}

void GOReferencePipe::Change(unsigned velocity, unsigned old_velocity)
{
	m_Reference->Set(velocity, m_ReferenceID);
}
