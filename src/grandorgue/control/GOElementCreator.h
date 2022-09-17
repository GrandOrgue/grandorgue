/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOELEMENTCREATOR_H
#define GOELEMENTCREATOR_H

#include <wx/string.h>

#include "GOButtonCallback.h"
#include "ptrvector.h"

class GOConfigReader;
class GOButtonControl;
class GOEnclosure;
class GOLabel;
class GOSetterButtonControl;
class GODefinitionFile;

struct ElementListEntry {
  wxString name;
  int value;
  bool is_public;
  bool is_pushbutton;
};

class GOElementCreator : private GOSetterButtonCallback {
private:
  void SetterButtonChanged(GOSetterButtonControl *button);

protected:
  ptr_vector<GOSetterButtonControl> m_button;

  virtual const struct ElementListEntry *GetButtonList() = 0;
  virtual void ButtonChanged(int id) = 0;
  void CreateButtons(GODefinitionFile *organfile);

public:
  GOElementCreator();
  virtual ~GOElementCreator();

  virtual void Load(GOConfigReader &cfg) = 0;

  virtual GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) = 0;
  virtual GOLabel *GetLabel(const wxString &name, bool is_panel) = 0;
  virtual GOButtonControl *GetButton(const wxString &name, bool is_panel);
};

#endif
