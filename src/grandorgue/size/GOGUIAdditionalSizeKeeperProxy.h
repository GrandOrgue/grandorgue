/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIADDITIONALSIZEKEEPERPROXY_H
#define GOGUIADDITIONALSIZEKEEPERPROXY_H

#include "GOGUIAdditionalSizeKeeper.h"

class GOGUIAdditionalSizeKeeperProxy : public GOGUIAdditionalSizeKeeper {
private:
  GOGUIAdditionalSizeKeeper &r_parent;
  wxString m_KeyPrefix;

  inline wxString MakeKey(const wxString &key) const {
    return m_KeyPrefix + key;
  }

public:
  GOGUIAdditionalSizeKeeperProxy(
    GOGUIAdditionalSizeKeeper &parent, const wxString &prefix)
    : r_parent(parent),
      m_KeyPrefix(prefix.IsEmpty() ? prefix : prefix + wxT(".")) {}

  int GetAddSize(const wxString &key) const override {
    return r_parent.GetAddSize(MakeKey(key));
  }

  void SetAddSize(const wxString &key, int value) override {
    r_parent.SetAddSize(MakeKey(key), value);
  }
};

#endif /* GOGUIADDITIONALSIZEKEEPERPROXY_H */
