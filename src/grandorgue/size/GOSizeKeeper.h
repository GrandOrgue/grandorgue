/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSIZEKEEPER_H
#define GOSIZEKEEPER_H

#include <map>

#include <wx/gdicmn.h>

#include "GOAdditionalSizeKeeper.h"
#include "GOSaveableObject.h"

class wxTopLevelWindow;

class GOConfigReader;
class GOConfigWriter;

/**
 * Saving and restoring a window size info (position and size)
 * and size in a config file.
 * Note: not all window managers allows applications to control window positions
 */
class GOSizeKeeper : public GOSaveableObject, public GOAdditionalSizeKeeper {
private:
  wxRect m_rect = wxRect(wxDefaultPosition, wxDefaultSize);
  int m_DisplayNum = -1;
  bool m_IsMaximized = false;
  // additional sizes for elements
  std::map<wxString, int> m_AdditionalSizes;

public:
  const wxRect &GetWindowRect() const { return m_rect; }

  void SetWindowRect(const wxRect &rect) { m_rect = rect; }

  int GetAdditionalSize(const wxString &key) const override;

  void SetAdditionalSize(const wxString &key, int value) override {
    m_AdditionalSizes[key] = value;
  }

  // set the group and read the size info from config files
  virtual void Load(GOConfigReader &cfg, const wxString &group);

  // save the size info to the config file
  virtual void Save(GOConfigWriter &cfg) override;

  // gets the current size info of the window
  virtual void CaptureSizeInfo(const wxTopLevelWindow &win);

  // make the windows position and size as the stored size info
  void ApplySizeInfo(wxTopLevelWindow &win);
};

#endif /* GOSIZEKEEPER_H */
