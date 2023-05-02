/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTERSTATE_H
#define GOSETTERSTATE_H

struct GOSetterState {
  enum SetterType { SETTER_REGULAR, SETTER_SCOPE, SETTER_SCOPED };

  // Is the Set button pushed now
  bool m_IsActive = false;
  // What type button is pushed now: Regular, Scope or Scoped
  SetterType m_SetterType = SETTER_REGULAR;
  // Is the Full button is pushed now
  bool m_IsStoreInvisible = false;
  // has the current combinations been changed after last loaded or saved
  // used for lighting the save button
  bool m_IsModified = false;
};

#endif /* GOSETTERSTATE_H */
