/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMODIFICATIONLISTENER_H
#define GOMODIFICATIONLISTENER_H

class GOModificationListener {
public:
  virtual void OnIsModifiedChanged(bool modified) = 0;
};

#endif /* GOMODIFICATIONLISTENER_H */
