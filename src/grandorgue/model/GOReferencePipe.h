/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOREFERENCEPIPE_H
#define GOREFERENCEPIPE_H

#include "GOCacheObject.h"
#include "GOPipe.h"
#include "GOReferencingObject.h"

class GOOrganModel;

class GOReferencePipe : public GOPipe, public GOReferencingObject {
private:
  GOOrganModel *m_model;
  GOPipe *m_Reference;
  unsigned m_ReferenceID;
  wxString m_Filename;

  void OnResolvingReferences() override;

  void VelocityChanged(unsigned velocity, unsigned old_velocity) override;

public:
  GOReferencePipe(GOOrganModel *model, GORank *rank, unsigned midi_key_number);

  void Load(GOConfigReader &cfg, const wxString &group, const wxString &prefix)
    override;
};

#endif
