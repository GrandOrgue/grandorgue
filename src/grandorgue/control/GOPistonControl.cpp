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
#include "model/GOOrganModel.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

GOPistonControl::GOPistonControl(GOOrganModel &organModel)
  : GOPushbuttonControl(organModel), drawstop(NULL) {
  organModel.RegisterControlChangedHandler(this);
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
      r_OrganModel.GetFirstManualIndex(),
      r_OrganModel.GetODFManualCount() - 1);
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          r_OrganModel.GetManual(i)->GetStopCount())
      - 1;
    drawstop = r_OrganModel.GetManual(i)->GetStop(j);
  } else if (type == wxT("COUPLER")) {
    i = cfg.ReadInteger(
      ODFSetting,
      group,
      wxT("ManualNumber"),
      r_OrganModel.GetFirstManualIndex(),
      r_OrganModel.GetODFManualCount() - 1);
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          r_OrganModel.GetManual(i)->GetODFCouplerCount())
      - 1;
    drawstop = r_OrganModel.GetManual(i)->GetCoupler(j);
  } else if (type == wxT("TREMULANT")) {
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          r_OrganModel.GetTremulantCount())
      - 1;
    drawstop = r_OrganModel.GetTremulant(j);
  } else if (type == wxT("SWITCH")) {
    j = cfg.ReadInteger(
          ODFSetting,
          group,
          wxT("ObjectNumber"),
          1,
          r_OrganModel.GetSwitchCount())
      - 1;
    drawstop = r_OrganModel.GetSwitch(j);
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

const wxString WX_MIDI_TYPE_CODE = wxT("Piston");
const wxString WX_MIDI_TYPE = _("Piston");

const wxString &GOPistonControl::GetMidiTypeCode() const {
  return WX_MIDI_TYPE_CODE;
}

const wxString &GOPistonControl::GetMidiType() const { return WX_MIDI_TYPE; }
