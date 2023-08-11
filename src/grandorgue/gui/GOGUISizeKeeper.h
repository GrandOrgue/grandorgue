/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUISIZEKEEPER_H
#define GOGUISIZEKEEPER_H

#include <wx/gdicmn.h>

#include "GOSaveableObject.h"

class wxTopLevelWindow;

class GOConfigReader;
class GOConfigWriter;

/**
 * Saving and restoring a window size info (position and size)
 * and size in a config file.
 * Note: not all window managers allows applications to control window positions
 */
class GOGUISizeKeeper : public GOSaveableObject {
private:
  wxRect m_rect = wxRect(wxDefaultPosition, wxDefaultSize);
  int m_DisplayNum = -1;
  bool m_IsMaximized = false;

public:
  const wxRect &GetWindowRect() const { return m_rect; }

  void SetWindowRect(const wxRect &rect) { m_rect = rect; }

  // set the group and read the size info from config files
  virtual void Load(GOConfigReader &cfg, const wxString &group);

  // save the size info to the config file
  virtual void Save(GOConfigWriter &cfg) override;

  // gets the current size info of the window
  virtual void CaptureSizeInfo(const wxTopLevelWindow &win);

  // make the windows position and size as the stored size info
  void ApplySizeInfo(wxTopLevelWindow &win);
};

#endif /* GOGUISIZEKEEPER_H */
