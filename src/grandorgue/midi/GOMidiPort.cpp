/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMidiPort.h"

#include "midi/GOMidiMap.h"
#include "GOMidi.h"


GOMidiPort::GOMidiPort(GOMidi* midi, wxString prefix, wxString name):
  m_midi(midi),
  m_IsActive(false),
  m_Name(name),
  m_Prefix(prefix)
{
  m_ID = m_midi->GetMidiMap().GetDeviceByString(m_Name);
}

const wxString GOMidiPort::GetClientName() const
{
  return wxT("GrandOrgue");
}
