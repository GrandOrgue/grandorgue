/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPIPECONFIGLISTENER_H
#define GOPIPECONFIGLISTENER_H

class GOPipeConfigListener {
public:
  virtual void NotifyPipeConfigModified() = 0;
};

#endif /* GOPIPECONFIGLISTENER_H */
