/*
 * Copyright 2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */
#ifndef GOTESTORGANMODEL_H
#define GOTESTORGANMODEL_H

#include "GOTest.h"
#include <wx/string.h>

#include "GOOrganController.h"
#include "config/GOConfig.h"
#include "model/GOWindchest.h"

#include <wx/string.h>

class GOTestOrganModel : public GOCommonControllerTest {

private:
  wxString name = "GOTestOrganModel";

public:
  GOTestOrganModel() { name = "GOTestOrganModel"; }
  virtual ~GOTestOrganModel();
  virtual void run();
  wxString GetName() { return name; };
};

#endif
