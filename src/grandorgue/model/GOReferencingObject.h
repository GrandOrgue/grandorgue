/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOREFERENCINGOBJECT_H
#define GOREFERENCINGOBJECT_H

class GOEventHandlerList;

class GOReferencingObject {
private:
  GOEventHandlerList &r_HandlerList;
  bool m_HasBeenResolved = false;

protected:
  virtual void OnResolvingReferences() {}

public:
  GOReferencingObject(GOEventHandlerList &handlerList);
  virtual ~GOReferencingObject();

  void ResolveReferences();
};

#endif /* GOREFERENCINGOBJECT_H */
