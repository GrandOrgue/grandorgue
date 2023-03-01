/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIHW1DISPLAYMETRICS_H
#define GOGUIHW1DISPLAYMETRICS_H

#include "GOGUIDisplayMetrics.h"

class GOConfigReader;

class GOGUIHW1DisplayMetrics : public GOGUIDisplayMetrics {
public:
  GOGUIHW1DisplayMetrics(GOConfigReader &ini, wxString group);
};

#endif
