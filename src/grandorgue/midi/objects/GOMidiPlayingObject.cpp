/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiPlayingObject.h"

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "midi/elements/GOMidiReceiver.h"
#include "model/GOOrganModel.h"

GOMidiPlayingObject::GOMidiPlayingObject(
  GOOrganModel &organModel, ObjectType objectType)
  : GOMidiObject(organModel.GetConfig().GetMidiMap(), objectType),
    r_OrganModel(organModel) {
  r_OrganModel.RegisterLifecycleListener(this);
  r_OrganModel.RegisterMidiObject(this);
  r_OrganModel.RegisterSaveableObject(this);
}

GOMidiPlayingObject::~GOMidiPlayingObject() {
  r_OrganModel.UnregisterSaveableObject(this);
  r_OrganModel.UnRegisterMidiObject(this);
  r_OrganModel.UnRegisterLifecycleListener(this);
}

const GOMidiObject *GOMidiPlayingObject::FindInitialMidiObject() const {
  return r_OrganModel.GetConfig().FindMidiInitialObject(GetPath());
}

void GOMidiPlayingObject::AfterMidiLoaded() {
  const GOMidiObject *pInitialObj = FindInitialMidiObject();

  if (pInitialObj) {
    if (!IsMidiConfigured())
      CopyMidiSettingFrom(*pInitialObj);
    else if (!IsReadOnly()) {
      // Already configured objects still pick up the Web Remote events, so
      // the phone also works with organs set up before it existed. Other
      // devices are left alone: they are the user's own choices.
      GOMidiReceiver *pRecv = GetMidiReceiver();
      const GOMidiReceiver *pInitialRecv = pInitialObj->GetMidiReceiver();
      const unsigned webRemoteId
        = r_OrganModel.GetConfig().GetWebRemoteDeviceId();

      // id 0 would mean "any device" and is only there if the config was
      // never loaded
      if (pRecv && pInitialRecv && webRemoteId)
        pRecv->AddMissingEventsFrom(*pInitialRecv, webRemoteId);
    }
  }
}
