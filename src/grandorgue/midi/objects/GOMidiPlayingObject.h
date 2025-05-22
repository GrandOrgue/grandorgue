/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIPLAYINGOBJECT_H
#define GOMIDIPLAYINGOBJECT_H

#include "GOMidiObject.h"

class GOMidiPlayingObject : public GOMidiObject, public GOSoundStateHandler {
protected:
  GOOrganModel &r_OrganModel;

public:
  GOMidiPlayingObject(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName);

  virtual ~GOMidiPlayingObject();

  void ShowConfigDialog();

  void PreparePlayback() override {}
  void PrepareRecording() override {}
  void AbortPlayback() override {}

  // Used in the GOMidiList dialog
  virtual wxString GetElementStatus() = 0;
  virtual std::vector<wxString> GetElementActions() = 0;
  virtual void TriggerElementActions(unsigned no) = 0;
};

#endif /* GOMIDIPLAYINGOBJECT_H */
