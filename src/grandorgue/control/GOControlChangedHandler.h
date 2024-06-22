/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONTROLCHANGEDHANDLER_H
#define GOCONTROLCHANGEDHANDLER_H

class GOControl;

class GOControlChangedHandler {
public:
  virtual ~GOControlChangedHandler() {}

  virtual void ControlChanged(GOControl *control) = 0;
};

#endif
