/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGENERALBUTTONCONTROL_H
#define GOGENERALBUTTONCONTROL_H

#include "combinations/model/GOGeneralCombination.h"
#include "control/GOPushbuttonControl.h"

class GOConfigReader;
class GOOrganModel;

class GOGeneralButtonControl : public GOPushbuttonControl {
private:
  GOOrganModel &r_OrganModel;
  GOGeneralCombination m_combination;

public:
  GOGeneralButtonControl(GOOrganModel &organModel, bool is_setter);
  void Load(GOConfigReader &cfg, wxString group);
  void Push() override;
  GOGeneralCombination &GetCombination();

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
};

#endif /* GOGENERAL_H */
