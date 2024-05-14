/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOReferencePipe.h"

#include <wx/intl.h>
#include <wx/tokenzr.h>

#include "config/GOConfigReader.h"

#include "GOManual.h"
#include "GOOrganModel.h"
#include "GORank.h"
#include "GOStop.h"

GOReferencePipe::GOReferencePipe(
  GOOrganModel *model, GORank *rank, unsigned midi_key_number)
  : GOPipe(model, rank, midi_key_number),
    m_model(model),
    m_Reference(NULL),
    m_ReferenceID(0),
    m_Filename() {}

void GOReferencePipe::Load(
  GOConfigReader &cfg, const wxString &group, const wxString &prefix) {
  SetGroupAndPrefix(group, prefix);
  m_model->RegisterCacheObject(this);
  m_Filename = cfg.ReadStringTrim(ODFSetting, group, prefix);
  if (!m_Filename.StartsWith(wxT("REF:")))
    throw(wxString) _("ReferencePipe without Reference");
}

void GOReferencePipe::Initialize() {
  if (!m_Filename.StartsWith(wxT("REF:")))
    throw(wxString) _("ReferencePipe without Reference");

  unsigned long manual, stop, pipe;
  wxArrayString strs
    = wxStringTokenize(m_Filename.Mid(4), wxT(":"), wxTOKEN_RET_EMPTY_ALL);
  if (
    strs.GetCount() != 3 || !strs[0].ToULong(&manual) || !strs[1].ToULong(&stop)
    || !strs[2].ToULong(&pipe))
    throw(wxString) _("Invalid reference ") + m_Filename;
  if (
    (manual < m_model->GetFirstManualIndex())
    || (manual >= m_model->GetODFManualCount()) || (stop <= 0)
    || (stop > m_model->GetManual(manual)->GetStopCount()) || (pipe <= 0)
    || (pipe > m_model->GetManual(manual)->GetStop(stop - 1)->GetRank(0)->GetPipeCount()))
    throw(wxString) _("Invalid reference ") + m_Filename;
  m_Reference
    = m_model->GetManual(manual)->GetStop(stop - 1)->GetRank(0)->GetPipe(
      pipe - 1);
  m_ReferenceID = m_Reference->RegisterReference(this);
}

void GOReferencePipe::VelocityChanged(
  unsigned velocity, unsigned old_velocity) {
  m_Reference->SetVelocity(velocity, m_ReferenceID);
}
