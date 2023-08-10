/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTWINDCHEST_H
#define GOTESTWINDCHEST_H

#include "GOTest.h"
#include <wx/string.h>

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"

#include <wx/string.h>

class GOTestWindchest : public GOCommonControllerTest {

public:
  GOTestWindchest();
  virtual ~GOTestWindchest();
  virtual void run();

  wxString name = "Windchest";
};

#endif
