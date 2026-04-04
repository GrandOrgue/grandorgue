/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPROGRESSMONITOR_H
#define GOPROGRESSMONITOR_H

#include <wx/string.h>

/**
 * Abstract interface for reporting loading progress.
 * Decouples model-layer loading code from GUI dialogs.
 */
class GOProgressMonitor {
public:
  virtual ~GOProgressMonitor() = default;

  virtual void Setup(
    long max, const wxString &title, const wxString &msg = wxEmptyString)
    = 0;
  virtual void Reset(long max, const wxString &msg = wxEmptyString) = 0;
  virtual bool Update(unsigned value, const wxString &msg) = 0;
};

#endif
