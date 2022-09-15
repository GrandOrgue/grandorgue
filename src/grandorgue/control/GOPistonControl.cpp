/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPiston.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "GOCoupler.h"
#include "GODefinitionFile.h"
#include "GODrawStop.h"
#include "GOManual.h"
#include "GOStop.h"
#include "GOSwitch.h"
#include "GOTremulant.h"
#include "config/GOConfigReader.h"

GOPiston::GOPiston(GODefinitionFile *organfile)
  : GOPushbutton(organfile), drawstop(NULL) {
  m_organfile->RegisterControlChangedHandler(this);
}

void GOPiston::Load(GOConfigReader &cfg, wxString group) {
  int i, j;
  wxString type = cfg.ReadStringTrim(ODFSetting, group, wxT("ObjectType"));
  type.MakeUpper();

  if (type == wxT("STOP")) {
    i = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("ManualNumber"),
      m_organfile->GetFirstManualIndex(),
      m_organfile->GetODFManualCount() - 1);
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_organfile->GetManual(i)->GetStopCount())
      - 1;
    drawstop = m_organfile->GetManual(i)->GetStop(j);
  } else if (type == wxT("COUPLER")) {
    i = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("ManualNumber"),
      m_organfile->GetFirstManualIndex(),
      m_organfile->GetODFManualCount() - 1);
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_organfile->GetManual(i)->GetODFCouplerCount())
      - 1;
    drawstop = m_organfile->GetManual(i)->GetCoupler(j);
  } else if (type == wxT("TREMULANT")) {
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_organfile->GetTremulantCount())
      - 1;
    drawstop = m_organfile->GetTremulant(j);
  } else if (type == wxT("SWITCH")) {
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_organfile->GetSwitchCount())
      - 1;
    drawstop = m_organfile->GetSwitch(j);
  } else
    throw wxString::Format(_("Invalid object type for reversible piston"));

  if (drawstop->IsReadOnly())
    wxLogError(
      _("Reversible piston connect to a read-only object: %s"), group.c_str());

  GOPushbutton::Load(cfg, group);
  ControlChanged(drawstop);
}

void GOPiston::ControlChanged(void *control) {
  if (control == drawstop)
    Display(drawstop->IsEngaged() ^ drawstop->DisplayInverted());
}

void GOPiston::Push() { this->drawstop->Push(); }

wxString GOPiston::GetMidiType() { return _("Piston"); }
