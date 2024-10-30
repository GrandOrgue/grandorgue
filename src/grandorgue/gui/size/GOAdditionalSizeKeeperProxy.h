/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOADDITIONALSIZEKEEPERPROXY_H
#define GOADDITIONALSIZEKEEPERPROXY_H

#include "GOAdditionalSizeKeeper.h"

class GOAdditionalSizeKeeperProxy : public GOAdditionalSizeKeeper {
private:
  GOAdditionalSizeKeeper &r_parent;
  wxString m_KeyPrefix;

  inline wxString MakeKey(const wxString &key) const {
    return m_KeyPrefix + key;
  }

public:
  GOAdditionalSizeKeeperProxy(
    GOAdditionalSizeKeeper &parent, const wxString &prefix)
    : r_parent(parent),
      m_KeyPrefix(prefix.IsEmpty() ? prefix : prefix + wxT(".")) {}

  int GetAdditionalSize(const wxString &key) const override {
    return r_parent.GetAdditionalSize(MakeKey(key));
  }

  void SetAdditionalSize(const wxString &key, int value) override {
    r_parent.SetAdditionalSize(MakeKey(key), value);
  }
};

#endif /* GOADDITIONALSIZEKEEPERPROXY_H */
