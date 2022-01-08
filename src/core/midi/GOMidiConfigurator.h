/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDICONFIGURATOR_H
#define GOMIDICONFIGURATOR_H

#include <wx/string.h>

#include <vector>

class GOMidiConfigurator {
 public:
  virtual ~GOMidiConfigurator() {}

  virtual wxString GetMidiType() = 0;
  virtual wxString GetMidiName() = 0;

  virtual void ShowConfigDialog() = 0;

  virtual wxString GetElementStatus() = 0;
  virtual std::vector<wxString> GetElementActions() = 0;
  virtual void TriggerElementActions(unsigned no) = 0;
};

#endif
