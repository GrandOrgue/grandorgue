/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOReferencingObject.h"

#include "GOEventHandlerList.h"

GOReferencingObject::GOReferencingObject(GOEventHandlerList &handlerList)
  : r_HandlerList(handlerList) {
  r_HandlerList.RegisterReferencingObject(this);
}

GOReferencingObject::~GOReferencingObject() {
  r_HandlerList.UnRegisterReferencingObject(this);
}

void GOReferencingObject::ResolveReferences() {
  if (!m_HasBeenResolved) {
    OnResolvingReferences();
    m_HasBeenResolved = true;
  }
}