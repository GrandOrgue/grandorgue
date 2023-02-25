/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOPistonControl.h"

#include <wx/intl.h>
#include <wx/log.h>

#include "config/GOConfigReader.h"
#include "model/GOCoupler.h"
#include "model/GODrawStop.h"
#include "model/GOManual.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

#include "GOOrganController.h"

GOPistonControl::GOPistonControl(GOOrganController *organController)
  : GOPushbuttonControl(organController), drawstop(NULL) {
  m_OrganController->RegisterControlChangedHandler(this);
}

void GOPistonControl::Load(GOConfigReader &cfg, wxString group) {
  int i, j;
  wxString type = cfg.ReadStringTrim(ODFSetting, group, wxT("ObjectType"));
  type.MakeUpper();

  if (type == wxT("STOP")) {
    i = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("ManualNumber"),
      m_OrganController->GetFirstManualIndex(),
      m_OrganController->GetODFManualCount() - 1);
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_OrganController->GetManual(i)->GetStopCount())
      - 1;
    drawstop = m_OrganController->GetManual(i)->GetStop(j);
  } else if (type == wxT("COUPLER")) {
    i = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("ManualNumber"),
      m_OrganController->GetFirstManualIndex(),
      m_OrganController->GetODFManualCount() - 1);
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_OrganController->GetManual(i)->GetODFCouplerCount())
      - 1;
    drawstop = m_OrganController->GetManual(i)->GetCoupler(j);
  } else if (type == wxT("TREMULANT")) {
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_OrganController->GetTremulantCount())
      - 1;
    drawstop = m_OrganController->GetTremulant(j);
  } else if (type == wxT("SWITCH")) {
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          m_OrganController->GetSwitchCount())
      - 1;
    drawstop = m_OrganController->GetSwitch(j);
  } else
    throw wxString::Format(_("Invalid object type for reversible piston"));

  if (drawstop->IsReadOnly())
    wxLogError(
      _("Reversible piston connect to a read-only object: %s"), group.c_str());

  GOPushbuttonControl::Load(cfg, group);
  ControlChanged(drawstop);
}

void GOPistonControl::ControlChanged(void *control) {
  if (control == drawstop)
    Display(drawstop->IsEngaged() ^ drawstop->DisplayInverted());
}

void GOPistonControl::Push() { this->drawstop->Push(); }

wxString GOPistonControl::GetMidiType() { return _("Piston"); }
