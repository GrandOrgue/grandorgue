/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOELEMENTCREATOR_H
#define GOELEMENTCREATOR_H

#include <wx/string.h>

#include "GOButtonCallback.h"
#include "ptrvector.h"

class GOButtonControl;
class GOConfigReader;
class GOOrganModel;
class GOEnclosure;
class GOLabelControl;

class GOElementCreator : private GOButtonCallback {
public:
  struct ButtonDefinitionEntry {
    wxString name;
    int value;
    bool is_public;
    bool is_pushbutton;
    bool is_piston;
  };

protected:
  ptr_vector<GOButtonControl> m_buttons;

  virtual const struct ButtonDefinitionEntry *GetButtonDefinitionList() = 0;
  virtual void ButtonStateChanged(int id, bool newState) = 0;
  void CreateButtons(GOOrganModel &organModel);

public:
  virtual void Load(GOConfigReader &cfg) = 0;

  virtual GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) = 0;
  virtual GOLabelControl *GetLabelControl(const wxString &name, bool is_panel)
    = 0;
  virtual GOButtonControl *GetButtonControl(
    const wxString &name, bool is_panel);

  void ButtonStateChanged(GOButtonControl *button, bool newState) override;
};

#endif
