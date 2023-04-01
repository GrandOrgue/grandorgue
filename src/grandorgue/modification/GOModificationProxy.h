/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMODIFICATIONPROXY_H
#define GOMODIFICATIONPROXY_H

#include "GOModificationListener.h"

class GOModificationProxy : public GOModificationListener {
private:
  GOModificationListener *p_listener = nullptr;

public:
  void SetModificationListener(GOModificationListener *pListener) {
    p_listener = pListener;
  }

  virtual void OnIsModifiedChanged(bool modified) override;
};

#endif /* GOMODIFICATIONPROXY_H */
